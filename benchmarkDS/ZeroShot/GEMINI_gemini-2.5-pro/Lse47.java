import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.concurrent.Executors;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse47 {

    // Simulates a decorator by mapping a URL path to this handler
    static class ImageHandler implements HttpHandler {
        private static final String IMAGE_DIR = "images";

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                String requestedPath = exchange.getRequestURI().getPath();
                // Expecting path like /images/filename.ext
                if (!requestedPath.startsWith("/images/")) {
                    sendErrorResponse(exchange, 404, "Not Found");
                    return;
                }
                String filename = requestedPath.substring("/images/".length());

                // --- Security Check 1: Basic filename validation ---
                // Disallow directory traversal characters.
                if (filename.contains("..") || filename.contains("/") || filename.contains("\\")) {
                    sendErrorResponse(exchange, 400, "Bad Request: Invalid filename.");
                    return;
                }
                
                Path baseDirPath = Paths.get(IMAGE_DIR).toAbsolutePath();
                Path requestedFilePath = baseDirPath.resolve(filename).normalize();

                // --- Security Check 2: Canonical Path Validation ---
                // Ensure the resolved path is still inside the intended directory.
                if (!requestedFilePath.startsWith(baseDirPath)) {
                    sendErrorResponse(exchange, 403, "Forbidden: Access denied.");
                    return;
                }

                File file = requestedFilePath.toFile();
                if (file.exists() && !file.isDirectory()) {
                    // Determine content type
                    String contentType = Files.probeContentType(requestedFilePath);
                    if (contentType == null) {
                        contentType = "application/octet-stream"; // fallback
                    }

                    // Send headers and file content
                    exchange.getResponseHeaders().set("Content-Type", contentType);
                    exchange.sendResponseHeaders(200, file.length());
                    try (OutputStream os = exchange.getResponseBody();
                         FileInputStream fis = new FileInputStream(file)) {
                        byte[] buffer = new byte[4096];
                        int bytesRead;
                        while ((bytesRead = fis.read(buffer)) != -1) {
                            os.write(buffer, 0, bytesRead);
                        }
                    }
                } else {
                    sendErrorResponse(exchange, 404, "Not Found: File does not exist.");
                }
            } catch (Exception e) {
                e.printStackTrace();
                sendErrorResponse(exchange, 500, "Internal Server Error");
            }
        }

        private void sendErrorResponse(HttpExchange exchange, int statusCode, String message) throws IOException {
            exchange.sendResponseHeaders(statusCode, message.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(message.getBytes());
            }
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        // Setup test environment
        setupTestFiles();

        // This is the equivalent of a decorator in a web framework.
        // It tells the server that any request to "/images/*" should be handled by ImageHandler.
        server.createContext("/images/", new ImageHandler());
        server.setExecutor(Executors.newCachedThreadPool()); 
        server.start();

        System.out.println("Server started on port " + port);
        System.out.println("--- Test Cases ---");
        System.out.println("1) Access a valid file: curl http://localhost:8080/images/test1.png --output test1_out.png");
        System.out.println("2) Access another valid file: curl http://localhost:8080/images/safe.txt");
        System.out.println("3) Access a non-existent file: curl -i http://localhost:8080/images/nonexistent.jpg");
        System.out.println("4) Attempt path traversal (should fail): curl -i http://localhost:8080/images/../Lse47.java");
        System.out.println("5) Attempt another path traversal: curl -i http://localhost:8080/images/..%2fLse47.java");
    }

    private static void setupTestFiles() throws IOException {
        Path imageDir = Paths.get("images");
        if (!Files.exists(imageDir)) {
            Files.createDirectory(imageDir);
        }
        // Create dummy files for testing
        Files.write(imageDir.resolve("test1.png"), new byte[]{ (byte)0x89, 'P', 'N', 'G' }); // Minimal PNG header
        Files.write(imageDir.resolve("safe.txt"), "This is a safe file.".getBytes());
    }
}