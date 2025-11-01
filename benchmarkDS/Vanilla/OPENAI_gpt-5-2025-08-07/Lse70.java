import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.HashMap;
import java.util.Map;

public class Lse70 {

    // Ensures uploads directory exists
    private static void ensureUploadsDir() throws IOException {
        Path dir = Paths.get("uploads");
        if (!Files.exists(dir)) {
            Files.createDirectories(dir);
        }
    }

    // Sanitize filename to avoid path traversal
    private static String sanitizeFileName(String name) {
        String base = name.replace("\\", "/");
        int idx = base.lastIndexOf('/');
        if (idx >= 0) base = base.substring(idx + 1);
        base = base.replace("..", "");
        if (base.isEmpty()) base = "upload.bin";
        return base;
    }

    // Read all bytes from InputStream
    private static byte[] readAll(InputStream in) throws IOException {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        byte[] buf = new byte[8192];
        int r;
        while ((r = in.read(buf)) != -1) {
            baos.write(buf, 0, r);
        }
        return baos.toByteArray();
    }

    // Parse query string into map
    private static Map<String, String> parseQuery(String query) throws UnsupportedEncodingException {
        Map<String, String> map = new HashMap<>();
        if (query == null || query.isEmpty()) return map;
        String[] pairs = query.split("&");
        for (String p : pairs) {
            int eq = p.indexOf('=');
            if (eq >= 0) {
                String k = URLDecoder.decode(p.substring(0, eq), "UTF-8");
                String v = URLDecoder.decode(p.substring(eq + 1), "UTF-8");
                map.put(k, v);
            } else {
                map.put(URLDecoder.decode(p, "UTF-8"), "");
            }
        }
        return map;
    }

    // The upload_file function: saves data under uploads directory. Returns absolute path string.
    public static String upload_file(String filename, byte[] data) throws IOException {
        ensureUploadsDir();
        String safe = sanitizeFileName(filename);
        Path out = Paths.get("uploads").resolve(safe);
        Files.write(out, data, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
        return out.toAbsolutePath().toString();
    }

    // Start simple HTTP server that accepts POST /upload?filename=NAME with raw body as file data
    public static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/upload", (HttpExchange exchange) -> {
            try {
                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                    exchange.sendResponseHeaders(405, -1);
                    return;
                }
                URI uri = exchange.getRequestURI();
                Map<String, String> q = parseQuery(uri.getQuery());
                String filename = q.get("filename");
                if (filename == null || filename.isEmpty()) {
                    byte[] resp = "Missing filename query parameter".getBytes(StandardCharsets.UTF_8);
                    exchange.sendResponseHeaders(400, resp.length);
                    OutputStream os = exchange.getResponseBody();
                    os.write(resp);
                    os.close();
                    return;
                }
                byte[] body = readAll(exchange.getRequestBody());
                String saved = upload_file(filename, body);
                byte[] resp = ("Saved: " + saved).getBytes(StandardCharsets.UTF_8);
                Headers headers = exchange.getResponseHeaders();
                headers.add("Content-Type", "text/plain; charset=utf-8");
                exchange.sendResponseHeaders(200, resp.length);
                OutputStream os = exchange.getResponseBody();
                os.write(resp);
                os.close();
            } catch (Exception e) {
                String msg = "Error: " + e.getMessage();
                exchange.sendResponseHeaders(500, msg.getBytes(StandardCharsets.UTF_8).length);
                OutputStream os = exchange.getResponseBody();
                os.write(msg.getBytes(StandardCharsets.UTF_8));
                os.close();
            }
        });
        server.setExecutor(null);
        server.start();
    }

    public static void main(String[] args) throws Exception {
        // Start server on port 8080
        startServer(8080);

        // 5 test cases calling upload_file directly
        String p1 = upload_file("hello.txt", "Hello, World!".getBytes(StandardCharsets.UTF_8));
        String p2 = upload_file("data.bin", new byte[]{0, 1, 2, 3, 4, 5});
        String p3 = upload_file("image.jpg", new byte[]{(byte) 0xFF, (byte) 0xD8, 0x00, 0x01});
        String p4 = upload_file("../evil/../../should_not_escape.txt", "safe".getBytes(StandardCharsets.UTF_8));
        String p5 = upload_file("unicode_файл.txt", "Привет".getBytes(StandardCharsets.UTF_8));

        // Optional to print paths (not required, but useful to verify)
        System.out.println(p1);
        System.out.println(p2);
        System.out.println(p3);
        System.out.println(p4);
        System.out.println(p5);
    }
}