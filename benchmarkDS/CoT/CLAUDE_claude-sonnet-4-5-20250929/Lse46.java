
import com.sun.net.httpserver.*;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.file.*;
import java.util.regex.Pattern;

public class Lse46 {
    private static final Pattern SAFE_FILENAME = Pattern.compile("^[a-zA-Z0-9_-]+\\\\.(jpg|jpeg|png|gif|bmp)$");
    private static final String IMAGES_DIR = "images";
    
    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/img", new ImgHandler());
        server.setExecutor(null);
        System.out.println("Server started on port 8000");
        
        // Create test directory and files
        setupTestEnvironment();
        
        // Test cases (access via browser or curl)
        System.out.println("Test cases:");
        System.out.println("1. Valid: http://localhost:8000/img?name=test1.jpg");
        System.out.println("2. Valid: http://localhost:8000/img?name=test2.png");
        System.out.println("3. Invalid (path traversal): http://localhost:8000/img?name=../etc/passwd");
        System.out.println("4. Invalid (wrong extension): http://localhost:8000/img?name=test.txt");
        System.out.println("5. Non-existent: http://localhost:8000/img?name=nonexistent.jpg");
        
        server.start();
    }
    
    static class ImgHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            String name = extractNameParameter(query);
            
            if (name == null || !isValidFilename(name)) {
                sendResponse(exchange, 400, "Invalid filename");
                return;
            }
            
            Path imagePath = Paths.get(IMAGES_DIR, name).normalize();
            Path imagesDir = Paths.get(IMAGES_DIR).toAbsolutePath().normalize();
            
            if (!imagePath.toAbsolutePath().normalize().startsWith(imagesDir)) {
                sendResponse(exchange, 403, "Access denied");
                return;
            }
            
            if (!Files.exists(imagePath)) {
                sendResponse(exchange, 404, "File not found");
                return;
            }
            
            byte[] fileContent = Files.readAllBytes(imagePath);
            exchange.getResponseHeaders().set("Content-Type", getContentType(name));
            exchange.sendResponseHeaders(200, fileContent.length);
            OutputStream os = exchange.getResponseBody();
            os.write(fileContent);
            os.close();
        }
        
        private String extractNameParameter(String query) {
            if (query == null) return null;
            String[] params = query.split("&");
            for (String param : params) {
                String[] pair = param.split("=");
                if (pair.length == 2 && pair[0].equals("name")) {
                    return pair[1];
                }
            }
            return null;
        }
        
        private boolean isValidFilename(String filename) {
            return SAFE_FILENAME.matcher(filename).matches();
        }
        
        private String getContentType(String filename) {
            if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
            if (filename.endsWith(".png")) return "image/png";
            if (filename.endsWith(".gif")) return "image/gif";
            if (filename.endsWith(".bmp")) return "image/bmp";
            return "application/octet-stream";
        }
        
        private void sendResponse(HttpExchange exchange, int code, String message) throws IOException {
            exchange.sendResponseHeaders(code, message.length());
            OutputStream os = exchange.getResponseBody();
            os.write(message.getBytes());
            os.close();
        }
    }
    
    private static void setupTestEnvironment() throws IOException {
        Files.createDirectories(Paths.get(IMAGES_DIR));
        Files.write(Paths.get(IMAGES_DIR, "test1.jpg"), "Fake JPG content".getBytes());
        Files.write(Paths.get(IMAGES_DIR, "test2.png"), "Fake PNG content".getBytes());
    }
}
