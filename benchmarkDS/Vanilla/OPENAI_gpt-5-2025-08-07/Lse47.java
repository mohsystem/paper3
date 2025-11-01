import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.URI;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse47 {

    // Returns image bytes if exists in images folder, else null
    public static byte[] getImageBytes(String filename) {
        if (filename == null || filename.contains("..") || filename.contains(File.separator + File.separator)) {
            return null;
        }
        Path imgPath = Paths.get("images").resolve(filename).normalize();
        if (!imgPath.startsWith(Paths.get("images").toAbsolutePath().normalize())) {
            return null; // prevent path traversal
        }
        if (Files.exists(imgPath) && Files.isRegularFile(imgPath)) {
            try {
                return Files.readAllBytes(imgPath);
            } catch (IOException e) {
                return null;
            }
        }
        return null;
    }

    // Simple content-type resolution
    public static String getContentType(String filename) {
        String f = filename.toLowerCase();
        if (f.endsWith(".jpg") || f.endsWith(".jpeg")) return "image/jpeg";
        if (f.endsWith(".png")) return "image/png";
        if (f.endsWith(".gif")) return "image/gif";
        if (f.endsWith(".bmp")) return "image/bmp";
        if (f.endsWith(".webp")) return "image/webp";
        try {
            String probed = Files.probeContentType(Paths.get(filename));
            if (probed != null) return probed;
        } catch (Exception ignored) {}
        return "application/octet-stream";
    }

    // Starts a simple HTTP server with /image/ route
    public static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/image/", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    exchange.sendResponseHeaders(405, -1);
                    return;
                }
                URI uri = exchange.getRequestURI();
                String path = uri.getPath();
                String prefix = "/image/";
                String filename = path.startsWith(prefix) ? path.substring(prefix.length()) : "";
                if (filename.isEmpty()) {
                    exchange.sendResponseHeaders(400, -1);
                    return;
                }
                byte[] data = getImageBytes(filename);
                if (data == null) {
                    String notFound = "File not found";
                    exchange.sendResponseHeaders(404, notFound.length());
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(notFound.getBytes());
                    }
                    return;
                }
                Headers h = exchange.getResponseHeaders();
                h.add("Content-Type", getContentType(filename));
                exchange.sendResponseHeaders(200, data.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(data);
                }
            }
        });
        server.setExecutor(null);
        server.start();
        System.out.println("Java server running on http://localhost:" + port + "/image/{filename}");
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases for the core function
        String[] tests = {
                "sample.jpg",
                "logo.png",
                "missing.png",
                "test.gif",
                "photos/pic.jpg"
        };
        for (String t : tests) {
            byte[] b = getImageBytes(t);
            System.out.println("Test '" + t + "': exists=" + (b != null) + (b != null ? (", bytes=" + b.length) : ""));
        }

        // Start server
        startServer(8080);

        // Keep running
        Thread.currentThread().join();
    }
}