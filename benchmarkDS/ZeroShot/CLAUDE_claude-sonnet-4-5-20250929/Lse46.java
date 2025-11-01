
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse46 {
    
    public static void img(HttpExchange exchange, String name) throws IOException {
        // Sanitize the filename to prevent directory traversal attacks
        String sanitizedName = sanitizeFilename(name);
        
        // Construct the file path securely
        Path basePath = Paths.get("images").toAbsolutePath().normalize();
        Path filePath = basePath.resolve(sanitizedName).normalize();
        
        // Verify the file is within the images directory
        if (!filePath.startsWith(basePath)) {
            sendResponse(exchange, 403, "Forbidden");
            return;
        }
        
        File file = filePath.toFile();
        
        if (!file.exists() || !file.isFile()) {
            sendResponse(exchange, 404, "File not found");
            return;
        }
        
        // Read file and send response
        byte[] fileContent = Files.readAllBytes(filePath);
        exchange.getResponseHeaders().set("Content-Type", getContentType(sanitizedName));
        exchange.sendResponseHeaders(200, fileContent.length);
        OutputStream os = exchange.getResponseBody();
        os.write(fileContent);
        os.close();
    }
    
    private static String sanitizeFilename(String filename) {
        // Remove any path traversal attempts and keep only the filename
        return filename.replaceAll("[^a-zA-Z0-9._-]", "_");
    }
    
    private static String getContentType(String filename) {
        if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
            return "image/jpeg";
        } else if (filename.endsWith(".png")) {
            return "image/png";
        } else if (filename.endsWith(".gif")) {
            return "image/gif";
        }
        return "application/octet-stream";
    }
    
    private static void sendResponse(HttpExchange exchange, int statusCode, String message) throws IOException {
        exchange.sendResponseHeaders(statusCode, message.length());
        OutputStream os = exchange.getResponseBody();
        os.write(message.getBytes());
        os.close();
    }
    
    public static void main(String[] args) throws IOException {
        // Create images directory if it doesn't exist
        new File("images").mkdirs();
        
        // Create test images
        String[] testFiles = {"test1.jpg", "test2.png", "test3.gif", "test4.jpeg", "test5.jpg"};
        for (String filename : testFiles) {
            File testFile = new File("images/" + filename);
            if (!testFile.exists()) {
                try (FileOutputStream fos = new FileOutputStream(testFile)) {
                    fos.write(("Test content for " + filename).getBytes());
                }
            }
        }
        
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        
        server.createContext("/img", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                String query = exchange.getRequestURI().getQuery();
                String name = "";
                if (query != null && query.startsWith("name=")) {
                    name = query.substring(5);
                }
                img(exchange, name);
            }
        });
        
        server.setExecutor(null);
        server.start();
        
        System.out.println("Server started on port 8000");
        System.out.println("Test URLs:");
        for (String filename : testFiles) {
            System.out.println("http://localhost:8000/img?name=" + filename);
        }
    }
}
