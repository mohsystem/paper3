import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;

public class Lse66 {

    public static String savePdf(byte[] data, String filename) throws IOException {
        if (filename == null) throw new IllegalArgumentException("filename is null");
        if (data == null) throw new IllegalArgumentException("data is null");
        if (data.length < 4 || !isPdfBytes(data)) {
            throw new IllegalArgumentException("Not a valid PDF (bad signature)");
        }
        String sanitized = sanitizeFilename(filename);
        if (!sanitized.toLowerCase(Locale.ROOT).endsWith(".pdf")) {
            throw new IllegalArgumentException("Filename must end with .pdf");
        }

        Path uploadsDir = Paths.get("uploads");
        Files.createDirectories(uploadsDir);

        Path target = uploadsDir.resolve(sanitized);
        if (Files.exists(target)) {
            String base = sanitized.substring(0, sanitized.length() - 4);
            String ext = ".pdf";
            int i = 1;
            while (Files.exists(target)) {
                target = uploadsDir.resolve(base + "_" + i + ext);
                i++;
            }
        }

        Files.write(target, data, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);
        return target.toString();
    }

    private static boolean isPdfBytes(byte[] data) {
        if (data.length < 4) return false;
        return data[0] == '%' && data[1] == 'P' && data[2] == 'D' && data[3] == 'F';
    }

    private static String sanitizeFilename(String name) {
        // keep only last path segment
        name = name.replace('\\', '/');
        int idx = name.lastIndexOf('/');
        if (idx >= 0) name = name.substring(idx + 1);
        // remove illegal characters
        StringBuilder sb = new StringBuilder();
        for (char c : name.toCharArray()) {
            if (Character.isLetterOrDigit(c) || c == '.' || c == '_' || c == '-') {
                sb.append(c);
            } else {
                sb.append('_');
            }
        }
        String cleaned = sb.toString();
        if (!cleaned.toLowerCase(Locale.ROOT).endsWith(".pdf")) {
            // ensure we don't end up without extension if original had none
            cleaned = cleaned + ".pdf";
        }
        return cleaned;
    }

    private static Map<String, String> queryToMap(String query) {
        Map<String, String> result = new HashMap<>();
        if (query == null || query.isEmpty()) return result;
        String[] pairs = query.split("&");
        for (String pair : pairs) {
            int idx = pair.indexOf('=');
            if (idx >= 0) {
                String key = urlDecode(pair.substring(0, idx));
                String val = urlDecode(pair.substring(idx + 1));
                result.put(key, val);
            } else {
                result.put(urlDecode(pair), "");
            }
        }
        return result;
    }

    private static String urlDecode(String s) {
        try {
            return URLDecoder.decode(s, "UTF-8");
        } catch (Exception e) {
            return s;
        }
    }

    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                send(exchange, 405, "Method Not Allowed");
                return;
            }
            Headers headers = exchange.getRequestHeaders();
            String contentType = String.join(",", headers.getOrDefault("Content-Type", List.of("")));
            if (contentType == null || !contentType.toLowerCase(Locale.ROOT).contains("application/pdf")) {
                send(exchange, 415, "Content-Type must be application/pdf");
                return;
            }
            Map<String, String> qp = queryToMap(exchange.getRequestURI().getRawQuery());
            String filename = qp.get("filename");
            if (filename == null || filename.isBlank()) {
                send(exchange, 400, "Missing filename query parameter");
                return;
            }
            byte[] body = readAll(exchange.getRequestBody());
            try {
                String saved = savePdf(body, filename);
                send(exchange, 200, "Saved: " + saved);
            } catch (IllegalArgumentException iae) {
                send(exchange, 400, iae.getMessage());
            } catch (Exception e) {
                send(exchange, 500, "Server error: " + e.getMessage());
            }
        }

        private byte[] readAll(InputStream in) throws IOException {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            byte[] buf = new byte[8192];
            int r;
            while ((r = in.read(buf)) != -1) {
                bos.write(buf, 0, r);
            }
            return bos.toByteArray();
        }

        private void send(HttpExchange ex, int code, String msg) throws IOException {
            byte[] data = msg.getBytes(StandardCharsets.UTF_8);
            ex.getResponseHeaders().add("Content-Type", "text/plain; charset=UTF-8");
            ex.sendResponseHeaders(code, data.length);
            try (OutputStream os = ex.getResponseBody()) {
                os.write(data);
            }
        }
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for savePdf
        byte[] valid = "%PDF-1.7\n%..\n".getBytes(StandardCharsets.US_ASCII);
        byte[] invalidSig = "XXXX".getBytes(StandardCharsets.US_ASCII);
        try {
            System.out.println("Test1: " + savePdf(valid, "report.pdf"));
        } catch (Exception e) {
            System.out.println("Test1 failed: " + e.getMessage());
        }
        try {
            System.out.println("Test2: " + savePdf(invalidSig, "bad.pdf"));
        } catch (Exception e) {
            System.out.println("Test2 failed: " + e.getMessage());
        }
        try {
            System.out.println("Test3: " + savePdf(valid, "notes.txt"));
        } catch (Exception e) {
            System.out.println("Test3 failed: " + e.getMessage());
        }
        try {
            System.out.println("Test4: " + savePdf(valid, "../../escape.pdf"));
        } catch (Exception e) {
            System.out.println("Test4 failed: " + e.getMessage());
        }
        try {
            String p1 = savePdf(valid, "duplicate.pdf");
            String p2 = savePdf(valid, "duplicate.pdf");
            System.out.println("Test5: " + p1 + " and " + p2);
        } catch (Exception e) {
            System.out.println("Test5 failed: " + e.getMessage());
        }

        // Start HTTP server
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(null);
        System.out.println("Java server on http://localhost:8080/upload (POST raw PDF, ?filename=...)");
        server.start();
    }
}