import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.io.FileWriter;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;

public class Lse46 {

    private static final String IMAGE_DIR = "images";

    public static void main(String[] args) throws IOException {
        // Setup: Create dummy directory and files for testing
        setupTestFiles();

        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        // Associate the URL /img with the ImgHandler
        server.createContext("/img", new ImgHandler());
        server.setExecutor(Executors.newCachedThreadPool()); 
        server.start();

        System.out.println("Java server started on port " + port);
        System.out.println("Test cases:");
        System.out.println("1. Valid file: http://localhost:8080/img?name=test1.txt");
        System.out.println("2. Another valid file: http://localhost:8080/img?name=image.png");
        System.out.println("3. Non-existent file: http://localhost:8080/img?name=notfound.txt");
        System.out.println("4. Missing parameter: http://localhost:8080/img");
        System.out.println("5. Directory traversal attempt: http://localhost:8080/img?name=../somefile.txt");
    }

    static class ImgHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            Map<String, String> params = queryToMap(query);
            String fileName = params.get("name");

            if (fileName == null || fileName.isEmpty()) {
                sendResponse(exchange, 400, "Bad Request: 'name' parameter is missing.");
                return;
            }

            // Security: Prevent directory traversal attacks
            if (fileName.contains("..")) {
                sendResponse(exchange, 400, "Bad Request: Invalid file name.");
                return;
            }

            Path filePath = Paths.get(IMAGE_DIR, fileName).toAbsolutePath().normalize();
            File file = filePath.toFile();

            // Security: Ensure the resolved path is still within the intended directory
            if (!filePath.startsWith(Paths.get(IMAGE_DIR).toAbsolutePath())) {
                 sendResponse(exchange, 400, "Bad Request: Access denied.");
                 return;
            }

            if (file.exists() && !file.isDirectory()) {
                byte[] fileBytes = Files.readAllBytes(filePath);
                exchange.getResponseHeaders().set("Content-Type", "application/octet-stream");
                exchange.sendResponseHeaders(200, fileBytes.length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(fileBytes);
                }
            } else {
                sendResponse(exchange, 404, "Not Found: The requested file does not exist.");
            }
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
        
        private Map<String, String> queryToMap(String query) {
            Map<String, String> result = new HashMap<>();
            if (query == null) {
                return result;
            }
            for (String param : query.split("&")) {
                String[] entry = param.split("=");
                if (entry.length > 1) {
                    result.put(entry[0], entry[1]);
                } else {
                    result.put(entry[0], "");
                }
            }
            return result;
        }
    }

    private static void setupTestFiles() {
        File dir = new File(IMAGE_DIR);
        if (!dir.exists()) {
            dir.mkdirs();
        }
        try {
            new FileWriter(IMAGE_DIR + "/test1.txt").append("This is test file 1.").close();
            new FileWriter(IMAGE_DIR + "/image.png").append("This is a dummy png file content.").close();
        } catch (IOException e) {
            System.err.println("Could not create test files.");
            e.printStackTrace();
        }
    }
}