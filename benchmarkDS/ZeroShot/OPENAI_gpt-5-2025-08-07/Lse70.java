import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Locale;
import java.util.UUID;
import java.util.concurrent.Executors;

public class Lse70 {

    // Max upload size: 10 MB
    private static final long MAX_UPLOAD_BYTES = 10L * 1024L * 1024L;

    public static void main(String[] args) throws Exception {
        // Start HTTP server
        startHttpServer(8080);

        // 5 test cases invoking upload_file directly (not over HTTP)
        System.out.println("Running tests for upload_file...");

        // Test 1: Normal filename
        try (ByteArrayInputStream in = new ByteArrayInputStream("Hello world".getBytes(StandardCharsets.UTF_8))) {
            Path p = upload_file("hello.txt", in, 1024);
            System.out.println("Test 1 saved: " + p.toAbsolutePath());
        }

        // Test 2: Suspicious filename with traversal
        try (ByteArrayInputStream in = new ByteArrayInputStream("evil content".getBytes(StandardCharsets.UTF_8))) {
            Path p = upload_file("../evil.sh", in, 1024);
            System.out.println("Test 2 saved: " + p.toAbsolutePath());
        }

        // Test 3: Filename with invalid chars
        try (ByteArrayInputStream in = new ByteArrayInputStream("data".getBytes(StandardCharsets.UTF_8))) {
            Path p = upload_file("inv@lid:/name?.bin", in, 1024);
            System.out.println("Test 3 saved: " + p.toAbsolutePath());
        }

        // Test 4: Hidden dot file
        try (ByteArrayInputStream in = new ByteArrayInputStream("bashrc-like".getBytes(StandardCharsets.UTF_8))) {
            Path p = upload_file(".bashrc", in, 1024);
            System.out.println("Test 4 saved: " + p.toAbsolutePath());
        }

        // Test 5: Exceeding size
        try {
            InputStream huge = new InputStream() {
                private long remaining = 2L * 1024L * 1024L; // 2MB
                @Override public int read() {
                    if (remaining <= 0) return -1;
                    remaining--;
                    return 0;
                }
            };
            // Limit is 1MB so this should fail
            upload_file("toolarge.dat", huge, 1024 * 1024);
            System.out.println("Test 5 unexpectedly succeeded");
        } catch (IOException ex) {
            System.out.println("Test 5 correctly failed: " + ex.getMessage());
        }

        System.out.println("HTTP server listening on http://127.0.0.1:8080/upload");
    }

    public static Path upload_file(String originalName, InputStream in, long maxBytes) throws IOException {
        if (in == null) throw new IOException("No input stream");
        String safeName = sanitizeFilename(originalName);
        if (safeName.isEmpty()) throw new IOException("Invalid filename");
        Path uploadsDir = Paths.get("uploads").toAbsolutePath().normalize();
        Files.createDirectories(uploadsDir);

        String base = safeName;
        String ext = "";
        int idx = safeName.lastIndexOf('.');
        if (idx > 0 && idx < safeName.length() - 1) {
            base = safeName.substring(0, idx);
            ext = safeName.substring(idx);
        }
        String unique = base + "-" + UUID.randomUUID() + ext;
        Path target = uploadsDir.resolve(unique).normalize();

        if (!target.startsWith(uploadsDir)) throw new IOException("Invalid path resolution");

        try (OutputStream out = Files.newOutputStream(target, StandardOpenOption.CREATE_NEW)) {
            long written = copyLimited(in, out, maxBytes);
            if (written == 0) throw new IOException("Empty file not allowed");
        } catch (IOException e) {
            // Clean up partial file on error
            try { Files.deleteIfExists(target); } catch (Exception ignore) {}
            throw e;
        }
        return target;
    }

    private static String sanitizeFilename(String name) {
        if (name == null) return "";
        StringBuilder sb = new StringBuilder();
        for (char c : name.toCharArray()) {
            if (Character.isLetterOrDigit(c) || c == '.' || c == '_' || c == '-') {
                sb.append(c);
            } else {
                sb.append('_');
            }
        }
        String s = sb.toString();
        // Remove leading dots
        while (s.startsWith(".")) s = s.substring(1);
        if (s.length() > 100) s = s.substring(0, 100);
        if (s.isEmpty()) s = "file";
        return s;
    }

    private static long copyLimited(InputStream in, OutputStream out, long maxBytes) throws IOException {
        byte[] buf = new byte[8192];
        long total = 0;
        int r;
        while ((r = in.read(buf)) != -1) {
            total += r;
            if (total > maxBytes) throw new IOException("Uploaded file exceeds size limit");
            out.write(buf, 0, r);
        }
        return total;
    }

    private static void startHttpServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", port), 0);
        server.createContext("/upload", Lse70::handleUpload);
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();
    }

    private static void handleUpload(HttpExchange exchange) throws IOException {
        String method = exchange.getRequestMethod();
        setSecurityHeaders(exchange.getResponseHeaders());

        if (!"POST".equalsIgnoreCase(method)) {
            respond(exchange, 405, "Method Not Allowed");
            return;
        }
        Headers headers = exchange.getRequestHeaders();
        String filename = headers.getFirst("X-Filename");
        if (filename == null || filename.isEmpty()) {
            respond(exchange, 400, "Missing X-Filename header");
            return;
        }
        long contentLength = -1;
        try {
            String cl = headers.getFirst("Content-Length");
            if (cl != null) contentLength = Long.parseLong(cl);
        } catch (NumberFormatException ignore) {}

        if (contentLength < 0) {
            respond(exchange, 411, "Content-Length required");
            return;
        }
        if (contentLength == 0) {
            respond(exchange, 400, "Empty body");
            return;
        }
        if (contentLength > MAX_UPLOAD_BYTES) {
            respond(exchange, 413, "Payload too large");
            return;
        }

        try (InputStream in = exchange.getRequestBody()) {
            Path saved = upload_file(filename, in, MAX_UPLOAD_BYTES);
            respond(exchange, 200, "Saved: " + saved.toAbsolutePath());
        } catch (IOException ex) {
            respond(exchange, 400, "Upload failed: " + ex.getMessage());
        }
    }

    private static void setSecurityHeaders(Headers h) {
        h.set("X-Content-Type-Options", "nosniff");
        h.set("X-Frame-Options", "DENY");
        h.set("Content-Security-Policy", "default-src 'none'");
        h.set("Cache-Control", "no-store");
    }

    private static void respond(HttpExchange ex, int code, String body) throws IOException {
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        ex.sendResponseHeaders(code, bytes.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(bytes);
        }
    }
}