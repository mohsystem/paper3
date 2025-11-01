
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.*;
import java.net.InetSocketAddress;
import java.net.URI;
import java.nio.file.*;
import java.util.*;

public class Lse48 {
    private static final String IMAGES_DIR = "images";
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(
        Arrays.asList(".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp")
    );

    static class ImgHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            byte[] response = img(exchange.getRequestURI());
            if (response != null) {
                exchange.sendResponseHeaders(200, response.length);
                OutputStream os = exchange.getResponseBody();
                os.write(response);
                os.close();
            } else {
                String errorMsg = "File not found or access denied";
                exchange.sendResponseHeaders(404, errorMsg.length());
                OutputStream os = exchange.getResponseBody();
                os.write(errorMsg.getBytes());
                os.close();
            }
        }
    }

    public static byte[] img(URI uri) {
        try {
            String query = uri.getQuery();
            if (query == null) return null;
            
            String filename = null;
            for (String param : query.split("&")) {
                String[] pair = param.split("=");
                if (pair.length == 2 && pair[0].equals("file")) {
                    filename = pair[1];
                    break;
                }
            }
            
            if (filename == null || filename.isEmpty()) return null;
            
            // Security: Validate filename - no path traversal
            if (filename.contains("..") || filename.contains("/") || 
                filename.contains("\\\\") || filename.startsWith(".")) {\n                return null;\n            }\n            \n            // Security: Validate file extension\n            String lowerFilename = filename.toLowerCase();\n            boolean validExtension = ALLOWED_EXTENSIONS.stream()\n                .anyMatch(ext -> lowerFilename.endsWith(ext));\n            if (!validExtension) return null;\n            \n            // Security: Use canonical path to prevent traversal\n            File imagesDir = new File(IMAGES_DIR);\n            if (!imagesDir.exists()) imagesDir.mkdirs();\n            \n            Path filePath = Paths.get(IMAGES_DIR, filename).toAbsolutePath().normalize();\n            Path baseDir = Paths.get(IMAGES_DIR).toAbsolutePath().normalize();\n            \n            // Security: Ensure file is within allowed directory\n            if (!filePath.startsWith(baseDir)) return null;\n            \n            File file = filePath.toFile();\n            if (!file.exists() || !file.isFile()) return null;\n            \n            // Read and return file\n            return Files.readAllBytes(filePath);\n            \n        } catch (Exception e) {\n            return null;\n        }\n    }\n\n    public static void main(String[] args) throws IOException {\n        // Create test images directory and sample files\n        new File(IMAGES_DIR).mkdirs();\n        \n        // Test cases\n        System.out.println("Test Case 1: Valid image file");\n        createTestFile("test1.jpg", "JPEG_IMAGE_DATA");\n        byte[] result1 = img(URI.create("/img?file=test1.jpg"));\n        System.out.println("Result: " + (result1 != null ? "Success" : "Failed"));\n        \n        System.out.println("\
Test Case 2: Path traversal attempt");\n        byte[] result2 = img(URI.create("/img?file=../etc/passwd"));\n        System.out.println("Result: " + (result2 == null ? "Blocked (Success)" : "Failed"));\n        \n        System.out.println("\
Test Case 3: Invalid extension");\n        byte[] result3 = img(URI.create("/img?file=test.txt"));\n        System.out.println("Result: " + (result3 == null ? "Blocked (Success)" : "Failed"));\n        \n        System.out.println("\
Test Case 4: Empty filename");\n        byte[] result4 = img(URI.create("/img?file="));\n        System.out.println("Result: " + (result4 == null ? "Blocked (Success)" : "Failed"));\n        \n        System.out.println("\
Test Case 5: Valid PNG file");\n        createTestFile("test2.png", "PNG_IMAGE_DATA");\n        byte[] result5 = img(URI.create("/img?file=test2.png"));\n        System.out.println("Result: " + (result5 != null ? "Success" : "Failed"));
    }
    
    private static void createTestFile(String filename, String content) throws IOException {
        Files.write(Paths.get(IMAGES_DIR, filename), content.getBytes());
    }
}
