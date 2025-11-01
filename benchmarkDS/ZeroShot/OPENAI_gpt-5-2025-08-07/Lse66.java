import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.EnumSet;
import java.util.Locale;
import java.util.Set;
import java.util.UUID;

public class Lse66 {

    private static final long MAX_SIZE = 1_048_576; // 1 MB
    private static final Path UPLOADS_DIR = Paths.get("uploads");

    public static Path savePdf(byte[] data, String originalFilename) throws IOException {
        if (data == null || data.length == 0) {
            throw new IOException("Empty file");
        }
        if (data.length > MAX_SIZE) {
            throw new IOException("File too large");
        }
        if (!isPdfHeader(data)) {
            throw new IOException("Invalid PDF header");
        }

        ensureUploadsDir();

        String finalName = generateSafeFilename(originalFilename);
        Path temp = Files.createTempFile(UPLOADS_DIR, "upload_", ".tmp");
        setFilePermissionsIfPossible(temp, 0600);

        try (OutputStream os = Files.newOutputStream(temp, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING)) {
            os.write(data);
            os.flush();
        }

        Path finalPath = UPLOADS_DIR.resolve(finalName);
        Files.move(temp, finalPath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
        setFilePermissionsIfPossible(finalPath, 0600);
        return finalPath.toAbsolutePath().normalize();
    }

    private static boolean isPdfHeader(byte[] data) {
        if (data.length < 5) return false;
        return data[0] == '%' && data[1] == 'P' && data[2] == 'D' && data[3] == 'F' && data[4] == '-';
    }

    private static void ensureUploadsDir() throws IOException {
        if (!Files.exists(UPLOADS_DIR)) {
            Files.createDirectories(UPLOADS_DIR);
        }
        setDirPermissionsIfPossible(UPLOADS_DIR, 0700);
    }

    private static String generateSafeFilename(String originalFilename) {
        String base = sanitizeFilename(originalFilename);
        String uuid = UUID.randomUUID().toString().replace("-", "");
        String timestamp = String.valueOf(Instant.now().toEpochMilli());
        String name = (base.isEmpty() ? "file" : base) + "_" + timestamp + "_" + uuid + ".pdf";
        return name.length() > 255 ? name.substring(0, 255) : name;
    }

    private static String sanitizeFilename(String name) {
        if (name == null) return "";
        // Keep only base name, remove any path
        int idx1 = name.lastIndexOf('/');
        int idx2 = name.lastIndexOf('\\');
        int cut = Math.max(idx1, idx2);
        if (cut >= 0 && cut + 1 < name.length()) {
            name = name.substring(cut + 1);
        }
        // Remove extension and non-allowed chars
        name = name.replaceAll("\\.+$", ""); // trim trailing dots
        StringBuilder sb = new StringBuilder();
        for (char c : name.toCharArray()) {
            if ((c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '-' || c == '_' || c == '.') {
                sb.append(c);
            } else {
                sb.append('_');
            }
        }
        String cleaned = sb.toString();
        // strip extension if present and not .pdf
        String lower = cleaned.toLowerCase(Locale.ROOT);
        if (lower.endsWith(".pdf")) {
            cleaned = cleaned.substring(0, cleaned.length() - 4);
        }
        if (cleaned.isEmpty()) cleaned = "file";
        if (cleaned.length() > 100) cleaned = cleaned.substring(0, 100);
        return cleaned;
    }

    private static void setDirPermissionsIfPossible(Path dir, int mode) {
        try {
            Set<PosixFilePermission> perms = EnumSet.noneOf(PosixFilePermission.class);
            if ((mode & 0400) != 0) perms.add(PosixFilePermission.OWNER_READ);
            if ((mode & 0200) != 0) perms.add(PosixFilePermission.OWNER_WRITE);
            if ((mode & 0100) != 0) perms.add(PosixFilePermission.OWNER_EXECUTE);
            if ((mode & 0040) != 0) perms.add(PosixFilePermission.GROUP_READ);
            if ((mode & 0020) != 0) perms.add(PosixFilePermission.GROUP_WRITE);
            if ((mode & 0010) != 0) perms.add(PosixFilePermission.GROUP_EXECUTE);
            if ((mode & 0004) != 0) perms.add(PosixFilePermission.OTHERS_READ);
            if ((mode & 0002) != 0) perms.add(PosixFilePermission.OTHERS_WRITE);
            if ((mode & 0001) != 0) perms.add(PosixFilePermission.OTHERS_EXECUTE);
            Files.setPosixFilePermissions(dir, perms);
        } catch (UnsupportedOperationException | IOException ignored) {
        }
    }

    private static void setFilePermissionsIfPossible(Path file, int mode) {
        try {
            Set<PosixFilePermission> perms = EnumSet.noneOf(PosixFilePermission.class);
            if ((mode & 0400) != 0) perms.add(PosixFilePermission.OWNER_READ);
            if ((mode & 0200) != 0) perms.add(PosixFilePermission.OWNER_WRITE);
            if ((mode & 0100) != 0) perms.add(PosixFilePermission.OWNER_EXECUTE);
            if ((mode & 0040) != 0) perms.add(PosixFilePermission.GROUP_READ);
            if ((mode & 0020) != 0) perms.add(PosixFilePermission.GROUP_WRITE);
            if ((mode & 0010) != 0) perms.add(PosixFilePermission.GROUP_EXECUTE);
            if ((mode & 0004) != 0) perms.add(PosixFilePermission.OTHERS_READ);
            if ((mode & 0002) != 0) perms.add(PosixFilePermission.OTHERS_WRITE);
            if ((mode & 0001) != 0) perms.add(PosixFilePermission.OTHERS_EXECUTE);
            Files.setPosixFilePermissions(file, perms);
        } catch (UnsupportedOperationException | IOException ignored) {
        }
    }

    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendResponse(exchange, 405, "Method Not Allowed");
                    return;
                }
                String path = exchange.getRequestURI().getPath();
                if (!"/upload".equals(path)) {
                    sendResponse(exchange, 404, "Not Found");
                    return;
                }

                Headers headers = exchange.getRequestHeaders();
                String contentType = headers.getFirst("Content-Type");
                if (contentType == null || !contentType.toLowerCase(Locale.ROOT).startsWith("application/pdf")) {
                    sendResponse(exchange, 415, "Unsupported Media Type - must be application/pdf");
                    return;
                }

                String originalFilename = headers.getFirst("X-Filename");
                if (originalFilename == null) originalFilename = "upload.pdf";

                ByteArrayOutputStream baos = new ByteArrayOutputStream();
                try (InputStream is = exchange.getRequestBody()) {
                    byte[] buf = new byte[8192];
                    long total = 0;
                    int r;
                    while ((r = is.read(buf)) != -1) {
                        total += r;
                        if (total > MAX_SIZE) {
                            sendResponse(exchange, 413, "Payload Too Large");
                            return;
                        }
                        baos.write(buf, 0, r);
                    }
                }
                byte[] body = baos.toByteArray();

                Path saved = savePdf(body, originalFilename);
                sendResponse(exchange, 201, "Saved: " + saved.toString());
            } catch (IOException e) {
                sendResponse(exchange, 400, "Bad Request: " + e.getMessage());
            } catch (Exception e) {
                sendResponse(exchange, 500, "Internal Server Error");
            }
        }

        private void sendResponse(HttpExchange exchange, int status, String message) throws IOException {
            byte[] resp = message.getBytes("UTF-8");
            Headers respHeaders = exchange.getResponseHeaders();
            respHeaders.set("Content-Type", "text/plain; charset=utf-8");
            respHeaders.set("X-Content-Type-Options", "nosniff");
            exchange.sendResponseHeaders(status, resp.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(resp);
            }
        }
    }

    private static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(null);
        Thread t = new Thread(server::start, "http-server");
        t.setDaemon(true);
        t.start();
    }

    private static String postPdf(byte[] data, String contentType, String filename) throws IOException {
        URL url = new URL("http://127.0.0.1:8080/upload");
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("POST");
        conn.setDoOutput(true);
        conn.setRequestProperty("Content-Type", contentType);
        if (filename != null) {
            conn.setRequestProperty("X-Filename", filename);
        }
        conn.setFixedLengthStreamingMode(data.length);
        conn.connect();
        try (OutputStream os = conn.getOutputStream()) {
            os.write(data);
            os.flush();
        }
        int code = conn.getResponseCode();
        InputStream is = (code >= 200 && code < 400) ? conn.getInputStream() : conn.getErrorStream();
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        if (is != null) {
            byte[] buf = new byte[4096];
            int r;
            while ((r = is.read(buf)) != -1) {
                baos.write(buf, 0, r);
            }
            is.close();
        }
        conn.disconnect();
        return code + " " + baos.toString("UTF-8");
    }

    public static void main(String[] args) throws Exception {
        startServer(8080);
        Thread.sleep(500);

        // Test 1: Valid PDF
        byte[] validPdf = ("%PDF-1.7\n%Test PDF\n1 0 obj\n<< /Type /Catalog >>\nendobj\ntrailer\n<<>>\n%%EOF\n").getBytes("UTF-8");
        System.out.println("Test1: " + postPdf(validPdf, "application/pdf", "test.pdf"));

        // Test 2: Wrong content type
        System.out.println("Test2: " + postPdf(validPdf, "text/plain", "test.pdf"));

        // Test 3: Invalid PDF header
        byte[] invalidHeader = "HELLO WORLD".getBytes("UTF-8");
        System.out.println("Test3: " + postPdf(invalidHeader, "application/pdf", "doc.pdf"));

        // Test 4: Path traversal in filename
        System.out.println("Test4: " + postPdf(validPdf, "application/pdf", "../evil.pdf"));

        // Test 5: Too large payload
        int bigSize = (int) (MAX_SIZE + 10);
        byte[] big = new byte[bigSize];
        System.arraycopy(validPdf, 0, big, 0, Math.min(validPdf.length, big.length));
        SecureRandom sr = new SecureRandom();
        sr.nextBytes(big);
        // Ensure header remains PDF
        byte[] header = "%PDF-".getBytes("UTF-8");
        System.arraycopy(header, 0, big, 0, header.length);
        System.out.println("Test5: " + postPdf(big, "application/pdf", "big.pdf"));
    }
}