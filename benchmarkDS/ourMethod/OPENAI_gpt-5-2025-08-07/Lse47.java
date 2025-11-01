import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URL;
import java.net.URLDecoder;
import java.nio.ByteBuffer;
import java.nio.channels.SeekableByteChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.EnumSet;

@Retention(RetentionPolicy.RUNTIME)
@interface Route {
    String value();
}

public class Lse47 {

    private static final long MAX_SIZE = 1_048_576L; // 1 MiB max
    private static final Path IMAGES_DIR = Paths.get(System.getProperty("user.dir"), "images");

    @Route("/images")
    public static void handleImages(HttpExchange exchange) throws IOException {
        try {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                sendPlain(exchange, 405, "Method Not Allowed");
                return;
            }

            String contextPath = "/images";
            URI uri = exchange.getRequestURI();
            String rawPath = uri.getRawPath();
            if (rawPath == null) {
                sendPlain(exchange, 400, "Bad Request");
                return;
            }
            if (!rawPath.startsWith(contextPath)) {
                sendPlain(exchange, 404, "Not Found");
                return;
            }
            String sub = rawPath.substring(contextPath.length());
            if (sub.startsWith("/")) sub = sub.substring(1);
            String decoded = URLDecoder.decode(sub, StandardCharsets.UTF_8.name());

            if (decoded.isEmpty()) {
                sendPlain(exchange, 400, "Missing filename");
                return;
            }
            // Basic validation
            if (decoded.contains("\0") || decoded.contains("\\") || decoded.startsWith("/") || decoded.length() > 4096) {
                sendPlain(exchange, 400, "Invalid path");
                return;
            }

            Path normalizedRel = Paths.get(decoded).normalize();
            if (normalizedRel.isAbsolute() || normalizedRel.toString().startsWith("..")) {
                sendPlain(exchange, 400, "Invalid path");
                return;
            }
            Path target = IMAGES_DIR.resolve(normalizedRel).normalize();
            if (!target.startsWith(IMAGES_DIR)) {
                sendPlain(exchange, 400, "Invalid path");
                return;
            }

            // Open first, then validate and stream. Try to avoid following symlinks.
            SeekableByteChannel ch = null;
            try {
                ch = Files.newByteChannel(target, EnumSet.of(StandardOpenOption.READ, LinkOption.NOFOLLOW_LINKS));
            } catch (IOException e) {
                sendPlain(exchange, 404, "Not Found");
                return;
            }

            try (SeekableByteChannel channel = ch) {
                long size = channel.size();
                if (size < 0 || size > MAX_SIZE) {
                    sendPlain(exchange, 413, "Payload Too Large");
                    return;
                }

                String contentType = Files.probeContentType(target);
                if (contentType == null) contentType = "application/octet-stream";

                Headers hdrs = exchange.getResponseHeaders();
                hdrs.set("Content-Type", contentType);
                hdrs.set("Content-Length", Long.toString(size));
                exchange.sendResponseHeaders(200, size);

                try (OutputStream os = new BufferedOutputStream(exchange.getResponseBody())) {
                    ByteBuffer buffer = ByteBuffer.allocate(8192);
                    while (channel.read(buffer) > 0) {
                        buffer.flip();
                        byte[] arr = new byte[buffer.remaining()];
                        buffer.get(arr);
                        os.write(arr);
                        buffer.clear();
                    }
                }
            }
        } catch (Exception e) {
            // Generic error to client
            sendPlain(exchange, 500, "Internal Server Error");
        }
    }

    private static void sendPlain(HttpExchange exchange, int status, String msg) throws IOException {
        byte[] data = msg.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
        exchange.sendResponseHeaders(status, data.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(data);
        }
    }

    private static void registerRoutes(HttpServer server) throws Exception {
        for (var m : Lse47.class.getDeclaredMethods()) {
            Route r = m.getAnnotation(Route.class);
            if (r != null) {
                String path = r.value();
                if (!path.startsWith("/")) path = "/" + path;
                final var method = m;
                server.createContext(path, new HttpHandler() {
                    @Override
                    public void handle(HttpExchange exchange) throws IOException {
                        try {
                            method.invoke(null, exchange);
                        } catch (Exception e) {
                            byte[] msg = "Internal Server Error".getBytes(StandardCharsets.UTF_8);
                            exchange.sendResponseHeaders(500, msg.length);
                            try (OutputStream os = exchange.getResponseBody()) {
                                os.write(msg);
                            }
                        }
                    }
                });
            }
        }
    }

    private static void createFile(Path p, byte[] data) throws IOException {
        Files.createDirectories(p.getParent());
        Files.write(p, data, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE);
    }

    public static void main(String[] args) throws Exception {
        // Prepare images directory and test files
        Files.createDirectories(IMAGES_DIR);
        createFile(IMAGES_DIR.resolve("hello.txt"), "Hello World!".getBytes(StandardCharsets.UTF_8));
        // Large file slightly above 1 MiB
        byte[] big = new byte[1_048_576 + 1];
        for (int i = 0; i < big.length; i++) big[i] = (byte) (i & 0xFF);
        createFile(IMAGES_DIR.resolve("big.bin"), big);
        // Outside file for symlink test
        Path outside = Paths.get(System.getProperty("user.dir"), "outside.txt");
        createFile(outside, "outside".getBytes(StandardCharsets.UTF_8));
        // Try to create a symlink inside images to outside file
        Path symlink = IMAGES_DIR.resolve("link_to_outside.txt");
        try {
            if (Files.exists(symlink)) Files.delete(symlink);
            Files.createSymbolicLink(symlink, Paths.get("..", "outside.txt"));
        } catch (Exception ignored) {
            // Symlinks may not be supported; ignore
        }

        // Start server
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", 8080), 0);
        registerRoutes(server);
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on http://127.0.0.1:8080");

        // Allow server to start
        Thread.sleep(300);

        // 5 test cases
        String[] tests = new String[]{
                "http://127.0.0.1:8080/images/hello.txt",       // 200
                "http://127.0.0.1:8080/images/missing.txt",      // 404
                "http://127.0.0.1:8080/images/../outside.txt",   // 400
                "http://127.0.0.1:8080/images/link_to_outside.txt", // 404 or 400 depending on platform
                "http://127.0.0.1:8080/images/big.bin"           // 413
        };
        for (String t : tests) {
            URL u = new URL(t);
            HttpURLConnection c = (HttpURLConnection) u.openConnection();
            c.setRequestMethod("GET");
            c.connect();
            int code = c.getResponseCode();
            System.out.println(t + " -> " + code);
            try {
                c.getInputStream().close();
            } catch (IOException ignored) {}
            c.disconnect();
        }

        server.stop(0);
        System.out.println("Server stopped.");
    }
}