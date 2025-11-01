import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.SecureRandom;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.*;

public class Lse71 {
    private static final long MAX_SIZE_BYTES = 5L * 1024 * 1024; // 5MB
    private static final Set<String> ALLOWED_EXT = new HashSet<>(Arrays.asList(".txt", ".png", ".jpg", ".jpeg", ".pdf", ".gif"));
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();

    public static void main(String[] args) throws Exception {
        Path uploadDir = Paths.get("uploads_java");
        Files.createDirectories(uploadDir);
        try {
            // Attempt to restrict permissions on POSIX systems
            Files.setPosixFilePermissions(uploadDir, PosixFilePermissions.fromString("rwx------"));
        } catch (UnsupportedOperationException ignored) {}

        // Start HTTP server
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/upload", new UploadHandler(uploadDir));
        server.setExecutor(null);
        server.start();
        System.out.println("Java server running on http://127.0.0.1:" + port);

        // 5 test cases invoking the secure uploader directly
        System.out.println("Running Java tests:");
        // 1) Valid small text file
        try (InputStream in = new ByteArrayInputStream("Hello, world!".getBytes(StandardCharsets.UTF_8))) {
            Path saved = saveUploadedFile(in, "greeting.txt", "text/plain", MAX_SIZE_BYTES, uploadDir);
            System.out.println("Test 1 OK: " + saved.toAbsolutePath());
        } catch (Exception e) {
            System.out.println("Test 1 FAILED: " + e.getMessage());
        }
        // 2) Suspicious filename traversal
        try (InputStream in = new ByteArrayInputStream("safe content".getBytes(StandardCharsets.UTF_8))) {
            Path saved = saveUploadedFile(in, "../../evil.txt", "text/plain", MAX_SIZE_BYTES, uploadDir);
            System.out.println("Test 2 OK (sanitized): " + saved.toAbsolutePath());
        } catch (Exception e) {
            System.out.println("Test 2 FAILED: " + e.getMessage());
        }
        // 3) Invalid extension
        try (InputStream in = new ByteArrayInputStream("binary".getBytes(StandardCharsets.UTF_8))) {
            Path saved = saveUploadedFile(in, "malware.exe", "application/octet-stream", MAX_SIZE_BYTES, uploadDir);
            System.out.println("Test 3 FAILED (should not save .exe): " + saved.toAbsolutePath());
        } catch (Exception e) {
            System.out.println("Test 3 OK (rejected): " + e.getMessage());
        }
        // 4) Oversized file
        try {
            byte[] big = new byte[(int) (MAX_SIZE_BYTES + 1024)];
            SECURE_RANDOM.nextBytes(big);
            try (InputStream in = new ByteArrayInputStream(big)) {
                Path saved = saveUploadedFile(in, "big.txt", "text/plain", MAX_SIZE_BYTES, uploadDir);
                System.out.println("Test 4 FAILED (should reject oversize): " + saved.toAbsolutePath());
            }
        } catch (Exception e) {
            System.out.println("Test 4 OK (oversize rejected): " + e.getMessage());
        }
        // 5) Allowed image extension with odd characters
        try (InputStream in = new ByteArrayInputStream(new byte[]{(byte)0x89, 0x50, 0x4E, 0x47})) {
            Path saved = saveUploadedFile(in, "my image (1).jpg", "image/jpeg", MAX_SIZE_BYTES, uploadDir);
            System.out.println("Test 5 OK: " + saved.toAbsolutePath());
        } catch (Exception e) {
            System.out.println("Test 5 FAILED: " + e.getMessage());
        }
    }

    public static Path saveUploadedFile(InputStream in, String originalFilename, String contentType, long maxSizeBytes, Path uploadDir) throws IOException {
        if (in == null) throw new IOException("No input stream");
        if (originalFilename == null || originalFilename.isEmpty()) throw new IOException("Missing filename");
        String sanitized = sanitizeFilename(originalFilename);
        String ext = getExtension(sanitized).toLowerCase(Locale.ROOT);
        if (!ALLOWED_EXT.contains(ext)) throw new IOException("Disallowed file extension");
        Files.createDirectories(uploadDir);
        String unique = randomHex(12) + ext;
        Path target = uploadDir.resolve(unique).normalize();

        if (!target.getParent().equals(uploadDir.toAbsolutePath().normalize())) {
            throw new IOException("Invalid upload path");
        }

        try (OutputStream out = Files.newOutputStream(target, StandardOpenOption.CREATE_NEW)) {
            long written = copyWithLimit(in, out, maxSizeBytes);
            if (written <= 0) {
                try { Files.deleteIfExists(target); } catch (Exception ignored) {}
                throw new IOException("Empty upload");
            }
        } catch (IOException e) {
            try { Files.deleteIfExists(target); } catch (Exception ignored) {}
            throw e;
        }

        try {
            Files.setPosixFilePermissions(target, PosixFilePermissions.fromString("rw-------"));
        } catch (UnsupportedOperationException ignored) {}

        return target.toAbsolutePath();
    }

    private static long copyWithLimit(InputStream in, OutputStream out, long limit) throws IOException {
        byte[] buf = new byte[8192];
        long total = 0;
        int r;
        while ((r = in.read(buf)) != -1) {
            total += r;
            if (total > limit) {
                throw new IOException("File too large");
            }
            out.write(buf, 0, r);
        }
        return total;
    }

    private static String sanitizeFilename(String name) {
        String base = Paths.get(name).getFileName().toString();
        StringBuilder sb = new StringBuilder();
        for (char c : base.toCharArray()) {
            if (Character.isLetterOrDigit(c) || c == '.' || c == '_' || c == '-') {
                sb.append(c);
            } else {
                sb.append('_');
            }
        }
        String sanitized = sb.toString();
        if (sanitized.length() > 100) sanitized = sanitized.substring(0, 100);
        if (!sanitized.contains(".")) sanitized += ".bin";
        return sanitized;
    }

    private static String getExtension(String name) {
        int idx = name.lastIndexOf('.');
        return idx >= 0 ? name.substring(idx) : "";
    }

    private static String randomHex(int bytes) {
        byte[] b = new byte[bytes];
        SECURE_RANDOM.nextBytes(b);
        StringBuilder sb = new StringBuilder(bytes * 2);
        for (byte x : b) sb.append(String.format("%02x", x));
        return sb.toString();
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String html = "<!doctype html><html><head><meta charset='utf-8'><title>Java Upload</title></head><body>" +
                    "<h1>Upload a file</h1>" +
                    "<input type='file' id='f'><button id='u'>Upload</button>" +
                    "<pre id='o'></pre>" +
                    "<script>document.getElementById('u').onclick=async()=>{" +
                    "let f=document.getElementById('f').files[0]; if(!f){alert('Choose a file');return;}" +
                    "let r=await fetch('/upload',{method:'POST',headers:{'X-Filename':f.name,'Content-Type':'application/octet-stream'},body:await f.arrayBuffer()});" +
                    "document.getElementById('o').textContent=await r.text();};</script>" +
                    "</body></html>";
            Headers h = exchange.getResponseHeaders();
            h.set("Content-Type", "text/html; charset=utf-8");
            h.set("X-Content-Type-Options", "nosniff");
            h.set("Content-Security-Policy", "default-src 'self'");
            byte[] bytes = html.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(200, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    static class UploadHandler implements HttpHandler {
        private final Path uploadDir;

        UploadHandler(Path uploadDir) {
            this.uploadDir = uploadDir;
        }

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                send(exchange, 405, "Method Not Allowed");
                return;
            }
            Headers headers = exchange.getRequestHeaders();
            String filename = headers.getFirst("X-Filename");
            String contentType = headers.getFirst("Content-Type");
            if (contentType == null) contentType = "application/octet-stream";
            long contentLen = -1;
            try {
                String cl = headers.getFirst("Content-Length");
                if (cl != null) contentLen = Long.parseLong(cl);
            } catch (NumberFormatException ignored) {}

            if (filename == null || filename.isEmpty()) {
                send(exchange, 400, "Missing X-Filename header");
                return;
            }
            if (contentLen < 0 || contentLen > MAX_SIZE_BYTES) {
                send(exchange, 413, "Payload too large or missing Content-Length");
                return;
            }

            try (InputStream in = exchange.getRequestBody()) {
                Path saved = saveUploadedFile(in, filename, contentType, MAX_SIZE_BYTES, uploadDir);
                send(exchange, 200, "Saved: " + saved.getFileName().toString());
            } catch (IOException e) {
                send(exchange, 400, "Upload failed: " + e.getMessage());
            }
        }

        private void send(HttpExchange exchange, int code, String msg) throws IOException {
            Headers h = exchange.getResponseHeaders();
            h.set("Content-Type", "text/plain; charset=utf-8");
            h.set("X-Content-Type-Options", "nosniff");
            h.set("Date", DateTimeFormatter.RFC_1123_DATE_TIME.format(ZonedDateTime.now()));
            byte[] bytes = msg.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(code, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }
}