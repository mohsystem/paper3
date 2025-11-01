import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.lang.annotation.*;
import java.lang.reflect.Method;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.file.*;
import java.util.*;
import java.util.concurrent.Executors;

public class Lse47 {

    @Retention(RetentionPolicy.RUNTIME)
    @Target(ElementType.METHOD)
    public @interface Route {
        String value();
    }

    public static class HttpResult {
        public final int status;
        public final String contentType;
        public final byte[] body;

        public HttpResult(int status, String contentType, byte[] body) {
            this.status = status;
            this.contentType = contentType;
            this.body = body;
        }
    }

    private static final Map<String, Method> routeMap = new LinkedHashMap<>();
    private static final Set<String> ALLOWED_EXT = new HashSet<>(Arrays.asList(".png", ".jpg", ".jpeg", ".gif", ".webp"));

    private static void initRoutes() {
        for (Method m : Lse47.class.getDeclaredMethods()) {
            Route r = m.getAnnotation(Route.class);
            if (r != null) {
                routeMap.put(r.value(), m);
            }
        }
    }

    @Route("/images")
    public static HttpResult serveImage(String filename) {
        try {
            if (filename == null || filename.isEmpty()) {
                return new HttpResult(400, "text/plain; charset=utf-8", "Bad Request".getBytes());
            }
            // Normalize and security checks
            Path imagesDir = Paths.get("images").toAbsolutePath().normalize();
            String lower = filename.toLowerCase(Locale.ROOT);
            String ext = "";
            int idx = lower.lastIndexOf('.');
            if (idx >= 0) ext = lower.substring(idx);
            if (!ALLOWED_EXT.contains(ext)) {
                return new HttpResult(403, "text/plain; charset=utf-8", "Forbidden".getBytes());
            }

            Path requested = imagesDir.resolve(filename).normalize();
            if (!requested.startsWith(imagesDir)) {
                return new HttpResult(400, "text/plain; charset=utf-8", "Bad Request".getBytes());
            }
            if (!Files.exists(requested) || !Files.isRegularFile(requested)) {
                return new HttpResult(404, "text/plain; charset=utf-8", "Not Found".getBytes());
            }

            byte[] data = Files.readAllBytes(requested);
            String ct = Files.probeContentType(requested);
            if (ct == null) {
                switch (ext) {
                    case ".png": ct = "image/png"; break;
                    case ".jpg":
                    case ".jpeg": ct = "image/jpeg"; break;
                    case ".gif": ct = "image/gif"; break;
                    case ".webp": ct = "image/webp"; break;
                    default: ct = "application/octet-stream";
                }
            }
            return new HttpResult(200, ct, data);
        } catch (Exception e) {
            return new HttpResult(500, "text/plain; charset=utf-8", "Internal Server Error".getBytes());
        }
    }

    private static void sendResponse(HttpExchange ex, HttpResult res) throws IOException {
        Headers h = ex.getResponseHeaders();
        h.set("X-Content-Type-Options", "nosniff");
        h.set("Cache-Control", "no-store");
        if (res.contentType != null) {
            h.set("Content-Type", res.contentType);
        }
        byte[] body = res.body != null ? res.body : new byte[0];
        ex.sendResponseHeaders(res.status, body.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(body);
        }
    }

    private static void startServer(int port) throws IOException {
        initRoutes();
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.setExecutor(Executors.newFixedThreadPool(4));

        for (String route : routeMap.keySet()) {
            String ctx = route.endsWith("/") ? route : route + "/";
            server.createContext(ctx, new HttpHandler() {
                @Override
                public void handle(HttpExchange exchange) throws IOException {
                    try {
                        if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                            sendResponse(exchange, new HttpResult(405, "text/plain; charset=utf-8", "Method Not Allowed".getBytes()));
                            return;
                        }
                        String reqPath = exchange.getRequestURI().getPath();
                        String base = ctx;
                        if (!base.endsWith("/")) base += "/";
                        String filename = reqPath.startsWith(base) ? reqPath.substring(base.length()) : "";
                        Method m = routeMap.get(route);
                        HttpResult res = (HttpResult) m.invoke(null, filename);
                        sendResponse(exchange, res);
                    } catch (Exception e) {
                        sendResponse(exchange, new HttpResult(500, "text/plain; charset=utf-8", "Internal Server Error".getBytes()));
                    }
                }
            });
        }
        server.start();
    }

    private static void ensureTestImages() throws IOException {
        Path imagesDir = Paths.get("images");
        if (!Files.exists(imagesDir)) Files.createDirectories(imagesDir);
        // Write small dummy files
        Path png = imagesDir.resolve("sample.png");
        Path jpg = imagesDir.resolve("photo.jpg");
        if (!Files.exists(png)) {
            byte[] content = new byte[]{(byte)0x89, 0x50, 0x4E, 0x47, 0x00, 0x00, 0x00, 0x00}; // dummy bytes
            Files.write(png, content);
        }
        if (!Files.exists(jpg)) {
            byte[] content = "JPEGDATA".getBytes();
            Files.write(jpg, content);
        }
    }

    private static int httpGetStatus(String urlStr) throws IOException {
        HttpURLConnection conn = (HttpURLConnection) new URL(urlStr).openConnection();
        conn.setRequestMethod("GET");
        conn.setInstanceFollowRedirects(false);
        conn.connect();
        int code = conn.getResponseCode();
        try (InputStream is = code >= 400 ? conn.getErrorStream() : conn.getInputStream()) {
            if (is != null) {
                byte[] buf = new byte[1024];
                while (is.read(buf) != -1) { /* drain */ }
            }
        }
        conn.disconnect();
        return code;
    }

    public static void main(String[] args) throws Exception {
        ensureTestImages();
        int port = 8085;
        startServer(port);
        Thread.sleep(500);

        // 5 test cases
        String base = "http://localhost:" + port;
        System.out.println(httpGetStatus(base + "/images/sample.png"));       // Expect 200
        System.out.println(httpGetStatus(base + "/images/none.png"));         // Expect 404
        System.out.println(httpGetStatus(base + "/images/../secret.txt"));    // Expect 400
        System.out.println(httpGetStatus(base + "/images/bad.txt"));          // Expect 403
        System.out.println(httpGetStatus(base + "/images/photo.jpg"));        // Expect 200
    }
}