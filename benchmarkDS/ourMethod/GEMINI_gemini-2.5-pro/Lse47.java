import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.NoSuchFileException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.concurrent.Executors;

public class Lse47 {
    private static final String IMAGES_DIR = "images";
    private static final Path IMAGE_DIR_PATH = Paths.get(IMAGES_DIR).toAbsolutePath();

    public static void main(String[] args) throws IOException {
        // Setup: Create a directory and some dummy files for testing.
        setupTestEnvironment();
        
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        // A decorator equivalent: mapping a URL context to a handler function.
        server.createContext("/images/", new ImageHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();

        System.out.println("Server started on port " + port);
        System.out.println("Test cases:");
        System.out.println("1. Valid file: http://localhost:8080/images/test1.txt");
        System.out.println("2. Valid file with extension: http://localhost:8080/images/test2.jpg");
        System.out.println("3. Non-existent file: http://localhost:8080/images/nonexistent.png");
        System.out.println("4. Path traversal attempt 1: http://localhost:8080/images/../Lse47.java");
        System.out.println("5. Path traversal attempt 2: http://localhost:8080/images/%2e%2e/Lse47.java");
    }

    private static void setupTestEnvironment() throws IOException {
        Files.createDirectories(IMAGE_DIR_PATH);
        Path testFile1 = IMAGE_DIR_PATH.resolve("test1.txt");
        Path testFile2 = IMAGE_DIR_PATH.resolve("test2.jpg");
        if (!Files.exists(testFile1)) {
            Files.write(testFile1, "This is a test file.".getBytes());
        }
        if (!Files.exists(testFile2)) {
            Files.write(testFile2, "This is a fake jpeg.".getBytes());
        }
    }

    static class ImageHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                String requestPath = exchange.getRequestURI().getPath();
                // Extract filename part from the URL
                String requestedFilename = requestPath.substring(requestPath.lastIndexOf('/') + 1);

                // Sanitize filename to prevent directory traversal.
                // Paths.get(filename).getFileName() extracts only the file part, discarding any directory info.
                Path sanitizedFilenamePath = Paths.get(requestedFilename).getFileName();
                if (sanitizedFilenamePath == null) {
                    sendErrorResponse(exchange, 400, "Bad Request: Invalid filename.");
                    return;
                }

                // Securely construct the full path
                Path filePath = IMAGE_DIR_PATH.resolve(sanitizedFilenamePath).normalize();

                // Final security check: ensure the resolved path is still within the base directory.
                if (!filePath.startsWith(IMAGE_DIR_PATH)) {
                    sendErrorResponse(exchange, 403, "Forbidden: Access denied.");
                    return;
                }

                serveFile(exchange, filePath);

            } catch (Exception e) {
                // Generic error for unexpected issues
                sendErrorResponse(exchange, 500, "Internal Server Error");
            }
        }

        private void serveFile(HttpExchange exchange, Path filePath) throws IOException {
            // Securely open and serve the file (TOCTOU-safe)
            // By trying to open a stream directly, we check existence and open in one operation.
            // NOFOLLOW_LINKS option prevents following symbolic links.
            try (InputStream is = Files.newInputStream(filePath, StandardOpenOption.READ, LinkOption.NOFOLLOW_LINKS)) {
                
                String mimeType = Files.probeContentType(filePath);
                if (mimeType == null) {
                    mimeType = "application/octet-stream"; // Default MIME type
                }
                
                exchange.getResponseHeaders().set("Content-Type", mimeType);
                exchange.sendResponseHeaders(200, Files.size(filePath));

                try (OutputStream os = exchange.getResponseBody()) {
                    byte[] buffer = new byte[4096];
                    int bytesRead;
                    while ((bytesRead = is.read(buffer)) != -1) {
                        os.write(buffer, 0, bytesRead);
                    }
                }
            } catch (NoSuchFileException e) {
                sendErrorResponse(exchange, 404, "Not Found: The requested file does not exist.");
            } catch (SecurityException e) {
                sendErrorResponse(exchange, 403, "Forbidden: Access denied.");
            } catch (IOException e) {
                sendErrorResponse(exchange, 500, "Internal Server Error: Could not read file.");
            }
        }

        private void sendErrorResponse(HttpExchange exchange, int statusCode, String message) throws IOException {
            byte[] response = message.getBytes("UTF-8");
            exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, response.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response);
            }
        }
    }
}