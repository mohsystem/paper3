import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.PosixFilePermission;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.Executors;

public class Lse65 {

    // Configuration
    private static final int DEFAULT_PORT = 8080;
    private static final int MAX_UPLOAD_SIZE = 10 * 1024 * 1024; // 10MB
    private static final Path UPLOADS_DIR = Paths.get("uploads");

    // Start the server on a given port
    public static void startServer(int port) throws IOException {
        ensureUploadsDirSecure();

        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
        System.out.println("Java server started at http://127.0.0.1:" + port + " (uploads -> " + UPLOADS_DIR.toAbsolutePath() + ")");
    }

    // Ensure uploads directory exists with secure permissions
    private static void ensureUploadsDirSecure() throws IOException {
        if (Files.notExists(UPLOADS_DIR)) {
            Files.createDirectories(UPLOADS_DIR);
        }
        try {
            Set<PosixFilePermission> dirPerms = new HashSet<>();
            dirPerms.add(PosixFilePermission.OWNER_READ);
            dirPerms.add(PosixFilePermission.OWNER_WRITE);
            dirPerms.add(PosixFilePermission.OWNER_EXECUTE);
            Files.setPosixFilePermissions(UPLOADS_DIR, dirPerms);
        } catch (UnsupportedOperationException ignored) {
            // Non-POSIX (e.g., Windows) - best effort
        }
    }

    // Sanitize filename and enforce .pdf extension
    public static String sanitizeFilename(String original) {
        if (original == null) return "upload.pdf";
        // Remove path components and control chars
        String base = original.replace("\\", "/");
        int idx = base.lastIndexOf('/');
        if (idx >= 0) base = base.substring(idx + 1);
        base = base.replaceAll("[\\p{Cntrl}]", "");
        // Allow only safe characters
        base = base.replaceAll("[^A-Za-z0-9._-]", "_");
        // Enforce .pdf extension
        if (!base.toLowerCase(Locale.ROOT).endsWith(".pdf")) {
            base = base + ".pdf";
        }
        // Trim length
        if (base.length() > 100) {
            String ext = ".pdf";
            base = base.substring(0, 100 - ext.length()) + ext;
        }
        if (base.equals(".pdf")) base = "upload.pdf";
        return base;
    }

    // Validate PDF by magic header and minimal structure
    public static boolean isLikelyPdf(byte[] data) {
        if (data == null || data.length < 5) return false;
        // %PDF- header
        if (!(data[0] == '%' && data[1] == 'P' && data[2] == 'D' && data[3] == 'F' && data[4] == '-')) {
            return false;
        }
        return true;
    }

    // Save bytes to uploads directory securely; returns final path
    public static Path savePdfSecurely(String originalFilename, byte[] bytes) throws IOException {
        String safe = sanitizeFilename(originalFilename);
        Path target = UPLOADS_DIR.resolve(safe).normalize();
        if (!target.startsWith(UPLOADS_DIR)) {
            throw new SecurityException("Invalid path traversal attempt");
        }
        // Ensure unique filename
        if (Files.exists(target)) {
            String name = safe;
            String base = name;
            String ext = "";
            int dot = name.lastIndexOf('.');
            if (dot > 0) {
                base = name.substring(0, dot);
                ext = name.substring(dot);
            }
            int counter = 1;
            do {
                String candidate = base + "-" + counter + ext;
                target = UPLOADS_DIR.resolve(candidate);
                counter++;
            } while (Files.exists(target));
        }
        Files.write(target, bytes, StandardOpenOption.CREATE_NEW);
        try {
            Set<PosixFilePermission> perms = new HashSet<>();
            perms.add(PosixFilePermission.OWNER_READ);
            perms.add(PosixFilePermission.OWNER_WRITE);
            Files.setPosixFilePermissions(target, perms);
        } catch (UnsupportedOperationException ignored) {
        }
        return target;
    }

    // Parse boundary from Content-Type header
    private static String parseBoundary(String contentType) {
        if (contentType == null) return null;
        String[] parts = contentType.split(";");
        for (String p : parts) {
            String s = p.trim();
            if (s.toLowerCase(Locale.ROOT).startsWith("boundary=")) {
                String b = s.substring(9);
                if (b.startsWith("\"") && b.endsWith("\"") && b.length() >= 2) {
                    b = b.substring(1, b.length() - 1);
                }
                return b;
            }
        }
        return null;
    }

    // Read request body with size limit
    private static byte[] readRequestBodyLimited(InputStream is, int maxBytes) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[8192];
        int total = 0;
        int r;
        while ((r = is.read(buf)) != -1) {
            total += r;
            if (total > maxBytes) {
                throw new IOException("Request entity too large");
            }
            baos.write(buf, 0, r);
        }
        return baos.toByteArray();
    }

    // Find byte sequence
    private static int indexOf(byte[] data, byte[] pattern, int from) {
        outer:
        for (int i = from; i <= data.length - pattern.length; i++) {
            for (int j = 0; j < pattern.length; j++) {
                if (data[i + j] != pattern[j]) continue outer;
            }
            return i;
        }
        return -1;
    }

    // Extract the single file part named "file" from multipart body
    private static class FilePart {
        String filename;
        String contentType;
        byte[] content;
    }

    private static FilePart parseMultipartSingleFile(byte[] body, String boundary) throws IOException {
        if (boundary == null || boundary.isEmpty()) throw new IOException("Missing boundary");
        byte[] bStart = ("--" + boundary).getBytes(StandardCharsets.ISO_8859_1);
        byte[] bEnd = ("--" + boundary + "--").getBytes(StandardCharsets.ISO_8859_1);
        int pos = indexOf(body, bStart, 0);
        if (pos == -1) throw new IOException("Boundary not found");
        // Move after boundary line
        int lineEnd = indexOf(body, "\r\n".getBytes(StandardCharsets.ISO_8859_1), pos);
        if (lineEnd == -1) throw new IOException("Malformed multipart");
        int cursor = lineEnd + 2;

        // Headers
        int headersEnd = indexOf(body, "\r\n\r\n".getBytes(StandardCharsets.ISO_8859_1), cursor);
        if (headersEnd == -1) throw new IOException("Malformed headers");
        String headers = new String(body, cursor, headersEnd - cursor, StandardCharsets.ISO_8859_1);

        String disp = null;
        String ctype = null;
        for (String h : headers.split("\r\n")) {
            int colon = h.indexOf(':');
            if (colon > 0) {
                String name = h.substring(0, colon).trim().toLowerCase(Locale.ROOT);
                String val = h.substring(colon + 1).trim();
                if (name.equals("content-disposition")) disp = val;
                if (name.equals("content-type")) ctype = val;
            }
        }
        if (disp == null || !disp.toLowerCase(Locale.ROOT).contains("form-data")) {
            throw new IOException("Missing content-disposition");
        }
        if (!disp.toLowerCase(Locale.ROOT).contains("name=\"file\"")) {
            throw new IOException("Expected field 'file'");
        }
        String filename = null;
        // parse filename="..."
        int fnIdx = disp.toLowerCase(Locale.ROOT).indexOf("filename=");
        if (fnIdx >= 0) {
            String rest = disp.substring(fnIdx + 9).trim();
            if (rest.startsWith("\"")) {
                int end = rest.indexOf('"', 1);
                if (end > 1) filename = rest.substring(1, end);
            } else {
                int semi = rest.indexOf(';');
                filename = (semi >= 0 ? rest.substring(0, semi) : rest).trim();
            }
        }
        if (filename == null || filename.isEmpty()) {
            throw new IOException("Missing filename");
        }
        int contentStart = headersEnd + 4;

        // Find next boundary
        int nextBoundary = indexOf(body, ("\r\n--" + boundary).getBytes(StandardCharsets.ISO_8859_1), contentStart);
        if (nextBoundary == -1) {
            // maybe end boundary without leading CRLF (rare), try end marker
            nextBoundary = indexOf(body, bEnd, contentStart);
            if (nextBoundary == -1) throw new IOException("Ending boundary not found");
        }
        int contentEnd = nextBoundary;
        // Trim trailing CRLF
        while (contentEnd > contentStart && (body[contentEnd - 1] == '\n' || body[contentEnd - 1] == '\r')) {
            contentEnd--;
        }
        byte[] fileBytes = Arrays.copyOfRange(body, contentStart, contentEnd);

        FilePart part = new FilePart();
        part.filename = filename;
        part.contentType = ctype;
        part.content = fileBytes;
        return part;
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!exchange.getRequestMethod().equalsIgnoreCase("GET")) {
                exchange.sendResponseHeaders(405, -1);
                return;
            }
            String html = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Upload PDF</title></head>" +
                    "<body><h1>Upload PDF</h1>" +
                    "<form method=\"POST\" action=\"/upload\" enctype=\"multipart/form-data\">" +
                    "<input type=\"file\" name=\"file\" accept=\"application/pdf\" required>" +
                    "<button type=\"submit\">Upload</button>" +
                    "</form></body></html>";
            byte[] bytes = html.getBytes(StandardCharsets.UTF_8);
            Headers h = exchange.getResponseHeaders();
            h.set("Content-Type", "text/html; charset=utf-8");
            exchange.sendResponseHeaders(200, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!exchange.getRequestMethod().equalsIgnoreCase("POST")) {
                exchange.sendResponseHeaders(405, -1);
                return;
            }
            String ct = exchange.getRequestHeaders().getFirst("Content-Type");
            String boundary = parseBoundary(ct);
            byte[] body;
            try {
                body = readRequestBodyLimited(exchange.getRequestBody(), MAX_UPLOAD_SIZE);
            } catch (IOException ex) {
                sendPlain(exchange, 413, "Upload too large");
                return;
            }

            FilePart part;
            try {
                part = parseMultipartSingleFile(body, boundary);
            } catch (Exception ex) {
                sendPlain(exchange, 400, "Malformed multipart/form-data");
                return;
            }

            // Basic content-type check
            if (part.contentType == null || !part.contentType.toLowerCase(Locale.ROOT).startsWith("application/pdf")) {
                sendPlain(exchange, 415, "Only application/pdf allowed");
                return;
            }
            if (!isLikelyPdf(part.content)) {
                sendPlain(exchange, 415, "Invalid PDF content");
                return;
            }
            if (part.content.length > MAX_UPLOAD_SIZE) {
                sendPlain(exchange, 413, "Upload too large");
                return;
            }

            Path saved;
            try {
                saved = savePdfSecurely(part.filename, part.content);
            } catch (SecurityException se) {
                sendPlain(exchange, 400, "Invalid filename");
                return;
            } catch (IOException ioe) {
                sendPlain(exchange, 500, "Failed to save file");
                return;
            }

            String msg = "Uploaded to: " + saved.toAbsolutePath();
            sendPlain(exchange, 200, msg);
        }

        private void sendPlain(HttpExchange ex, int code, String msg) throws IOException {
            byte[] b = msg.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().set("Content-Type", "text/plain; charset=utf-8");
            ex.sendResponseHeaders(code, b.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(b);
            }
        }
    }

    // Simple tests
    public static void main(String[] args) throws Exception {
        // Test 1: sanitizeFilename path traversal
        String t1 = sanitizeFilename("../../etc/passwd");
        System.out.println("Test1: " + t1);

        // Test 2: isLikelyPdf true
        byte[] pdf1 = "%PDF-1.7\n%âãÏÓ\n".getBytes(StandardCharsets.ISO_8859_1);
        System.out.println("Test2: " + isLikelyPdf(pdf1));

        // Test 3: isLikelyPdf false
        byte[] notPdf = "Hello world".getBytes(StandardCharsets.UTF_8);
        System.out.println("Test3: " + isLikelyPdf(notPdf));

        // Test 4: sanitize non-pdf extension enforced
        String t4 = sanitizeFilename("report.txt");
        System.out.println("Test4: " + t4);

        // Test 5: savePdfSecurely unique naming
        ensureUploadsDirSecure();
        Path p1 = savePdfSecurely("sample.pdf", pdf1);
        Path p2 = savePdfSecurely("sample.pdf", pdf1);
        System.out.println("Test5: " + p1.getFileName() + " | " + p2.getFileName());

        // Start server
        int port = DEFAULT_PORT;
        if (args.length > 0) {
            try {
                port = Integer.parseInt(args[0]);
            } catch (NumberFormatException ignored) {}
        }
        startServer(port);
    }
}