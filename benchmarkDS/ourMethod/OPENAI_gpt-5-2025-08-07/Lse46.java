import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import java.util.regex.Pattern;

public class Lse46 {
    private static final Path BASE_DIR = Paths.get("images").toAbsolutePath().normalize();
    private static final int MAX_SIZE = 5 * 1024 * 1024;
    private static final Pattern NAME_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,100}$");

    // Validate filename according to policy
    public static boolean isValidName(String name) {
        if (name == null) return false;
        if (!NAME_PATTERN.matcher(name).matches()) return false;
        if (name.contains("..") || name.contains("/") || name.contains("\\"))
            return false;
        return true;
    }

    // Open and read image securely; returns bytes or throws IOException/IllegalArgumentException
    public static byte[] readImage(String name) throws IOException {
        if (!isValidName(name)) throw new IllegalArgumentException("invalid");
        Path p = BASE_DIR.resolve(name).normalize();
        if (!p.startsWith(BASE_DIR)) throw new IllegalArgumentException("invalid");
        if (!Files.isRegularFile(p, LinkOption.NOFOLLOW_LINKS)) throw new NoSuchFileException("not found");
        long size = Files.size(p);
        if (size < 0 || size > MAX_SIZE) throw new IOException("too large");
        try (SeekableByteChannel ch = Files.newByteChannel(p, StandardOpenOption.READ)) {
            ByteBuffer buf = ByteBuffer.allocate((int) size);
            while (buf.hasRemaining()) {
                int r = ch.read(buf);
                if (r < 0) break;
            }
            return buf.array();
        }
    }

    // Guess simple content type
    private static String guessContentType(String name) {
        String lower = name.toLowerCase(Locale.ROOT);
        if (lower.endsWith(".png")) return "image/png";
        if (lower.endsWith(".jpg") || lower.endsWith(".jpeg")) return "image/jpeg";
        if (lower.endsWith(".gif")) return "image/gif";
        if (lower.endsWith(".bmp")) return "image/bmp";
        if (lower.endsWith(".webp")) return "image/webp";
        return "application/octet-stream";
    }

    // Start a minimal HTTP server mapping /img
    public static HttpServer startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/img", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                String method = exchange.getRequestMethod();
                if (!"GET".equalsIgnoreCase(method)) {
                    exchange.sendResponseHeaders(405, -1);
                    exchange.close();
                    return;
                }
                String rawQuery = exchange.getRequestURI().getRawQuery();
                Map<String, String> params = parseQuery(rawQuery);
                String rawName = params.get("name");
                String name = rawName == null ? null : URLDecoder.decode(rawName, StandardCharsets.UTF_8);
                if (!isValidName(name)) {
                    exchange.sendResponseHeaders(400, -1);
                    exchange.close();
                    return;
                }
                byte[] data;
                try {
                    data = readImage(name);
                } catch (IllegalArgumentException e) {
                    exchange.sendResponseHeaders(400, -1);
                    exchange.close();
                    return;
                } catch (NoSuchFileException e) {
                    exchange.sendResponseHeaders(404, -1);
                    exchange.close();
                    return;
                } catch (IOException e) {
                    exchange.sendResponseHeaders(500, -1);
                    exchange.close();
                    return;
                }
                Headers headers = exchange.getResponseHeaders();
                headers.add("Content-Type", guessContentType(name));
                headers.add("Content-Length", Integer.toString(data.length));
                exchange.sendResponseHeaders(200, data.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(data);
                } finally {
                    exchange.close();
                }
            }
        });
        server.setExecutor(null);
        server.start();
        return server;
    }

    private static Map<String, String> parseQuery(String raw) {
        Map<String, String> map = new HashMap<>();
        if (raw == null || raw.isEmpty()) return map;
        String[] parts = raw.split("&");
        for (String part : parts) {
            int idx = part.indexOf('=');
            if (idx <= 0) continue;
            String k = part.substring(0, idx);
            String v = part.substring(idx + 1);
            map.put(k, v);
        }
        return map;
    }

    // Simple tests
    public static void main(String[] args) throws Exception {
        String[] tests = new String[]{
                "test.png",
                "a.txt",
                "../etc/passwd",
                "nonexistent.xxx",
                "valid-name.jpg"
        };
        for (String t : tests) {
            try {
                byte[] data = readImage(t);
                System.out.println("Test [" + t + "]: OK, size=" + data.length);
            } catch (IllegalArgumentException e) {
                System.out.println("Test [" + t + "]: Rejected");
            } catch (NoSuchFileException e) {
                System.out.println("Test [" + t + "]: NotFound");
            } catch (IOException e) {
                System.out.println("Test [" + t + "]: Error");
            }
        }
        // Uncomment to start server: curl 'http://127.0.0.1:8080/img?name=test.png'
        // HttpServer server = startServer(8080);
        // System.out.println("Server running on http://127.0.0.1:8080");
    }
}