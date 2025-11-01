import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.File;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.nio.charset.StandardCharsets;

public class Lse46 {

    private static final String IMAGE_DIR = "images";

    public static void main(String[] args) throws IOException {
        // Setup: Create a directory and a dummy file for testing
        Path imageDirPath = Paths.get(IMAGE_DIR);
        if (!Files.exists(imageDirPath)) {
            Files.createDirectories(imageDirPath);
        }
        Path testFile = imageDirPath.resolve("test.txt");
        if (!Files.exists(testFile)) {
            Files.write(testFile, "This is a test file.".getBytes(StandardCharsets.UTF_8));
        }

        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        // The createContext method associates the URL path /img with our ImageHandler
        server.createContext("/img", new ImageHandler());
        server.setExecutor(null); // creates a default executor
        System.out.println("Server started on port 8080");
        System.out.println("Run the following curl commands to test:");
        System.out.println("1. Test Case 1 (Valid): curl http://localhost:8080/img?name=test.txt");
        System.out.println("2. Test Case 2 (Not Found): curl http://localhost:8080/img?name=notfound.txt");
        System.out.println("3. Test Case 3 (Path Traversal): curl http://localhost:8080/img?name=../../Lse46.java");
        System.out.println("4. Test Case 4 (Missing Param): curl http://localhost:8080/img");
        System.out.println("5. Test Case 5 (Empty Param): curl http://localhost:8080/img?name=");
        server.start();
    }

    static class ImageHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendError(exchange, 405, "Method Not Allowed");
                return;
            }

            String query = exchange.getRequestURI().getQuery();
            Map<String, String> params = parseQuery(query);
            String fileName = params.get("name");

            if (fileName == null || fileName.trim().isEmpty()) {
                sendError(exchange, 400, "Bad Request: 'name' parameter is missing or empty.");
                return;
            }

            // Security: Validate filename to prevent path traversal.
            // It should not contain any path separators.
            if (fileName.contains("/") || fileName.contains("\\")) {
                sendError(exchange, 400, "Bad Request: Invalid filename.");
                return;
            }

            try {
                Path imageDir = Paths.get(IMAGE_DIR).toRealPath();
                Path requestedFile = imageDir.resolve(fileName).normalize();

                // Security: Ensure the resolved path is still within the intended directory.
                if (!requestedFile.startsWith(imageDir)) {
                    sendError(exchange, 403, "Forbidden");
                    return;
                }
                
                // Security: Open is implicit here, we check properties first.
                // Files.isRegularFile and Files.isReadable check the file safely.
                if (Files.isRegularFile(requestedFile) && Files.isReadable(requestedFile)) {
                    exchange.sendResponseHeaders(200, Files.size(requestedFile));
                    try (OutputStream os = exchange.getResponseBody()) {
                        Files.copy(requestedFile, os);
                    }
                } else {
                    sendError(exchange, 404, "Not Found");
                }
            } catch (IOException e) {
                // Catches errors like if toRealPath() fails because base dir doesn't exist
                sendError(exchange, 500, "Internal Server Error");
            }
        }

        private void sendError(HttpExchange exchange, int code, String response) throws IOException {
            exchange.sendResponseHeaders(code, response.length());
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }

        private Map<String, String> parseQuery(String query) {
            Map<String, String> params = new HashMap<>();
            if (query != null) {
                String[] pairs = query.split("&");
                for (String pair : pairs) {
                    int idx = pair.indexOf("=");
                    if (idx > 0) {
                        params.put(pair.substring(0, idx), pair.substring(idx + 1));
                    }
                }
            }
            return params;
        }
    }
}