
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.HashMap;
import java.util.Map;

public class Lse46 {
    // Trusted base directory for images - must be absolute and validated
    private static final Path BASE_DIR;
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
    static {
        try {
            // Initialize base directory as absolute canonical path
            BASE_DIR = Paths.get("images").toAbsolutePath().normalize();
            // Ensure the directory exists
            Files.createDirectories(BASE_DIR);
        } catch (IOException e) {
            throw new ExceptionInInitializerError("Failed to initialize base directory: " + e.getMessage());
        }
    }
    
    // Decorator pattern: associates /img URL with img() handler
    public static byte[] img(String name) throws IOException {
        // Input validation: reject null, empty, or suspicious names
        if (name == null || name.isEmpty()) {
            throw new IllegalArgumentException("File name cannot be null or empty");
        }
        
        // Decode URL-encoded input
        String decodedName = URLDecoder.decode(name, StandardCharsets.UTF_8);
        
        // Sanitize: reject path traversal attempts
        if (decodedName.contains("..") || decodedName.contains("/") || 
            decodedName.contains("\\\\") || decodedName.startsWith(".")) {\n            throw new SecurityException("Invalid file name: path traversal not allowed");\n        }\n        \n        // Construct path relative to base directory\n        Path requestedPath = BASE_DIR.resolve(decodedName).normalize();\n        \n        // Verify the resolved path is still within base directory\n        if (!requestedPath.startsWith(BASE_DIR)) {\n            throw new SecurityException("Access denied: path outside base directory");\n        }\n        \n        // Open file with O_NOFOLLOW equivalent (no symlink following)\n        // Using Files.newInputStream with LinkOption.NOFOLLOW_LINKS\n        try (InputStream input = Files.newInputStream(requestedPath, \n                StandardOpenOption.READ, LinkOption.NOFOLLOW_LINKS)) {\n            \n            // Validate file attributes after opening\n            BasicFileAttributes attrs = Files.readAttributes(requestedPath, \n                BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);\n            \n            // Reject if not a regular file\n            if (!attrs.isRegularFile()) {\n                throw new SecurityException("Not a regular file");\n            }\n            \n            // Check file size limit\n            long size = attrs.size();\n            if (size > MAX_FILE_SIZE || size < 0) {\n                throw new SecurityException("File size exceeds limit or is invalid");\n            }\n            \n            // Read file content with bounded buffer\n            ByteArrayOutputStream output = new ByteArrayOutputStream();\n            byte[] buffer = new byte[8192];\n            int bytesRead;\n            long totalRead = 0;\n            \n            while ((bytesRead = input.read(buffer)) != -1) {\n                totalRead += bytesRead;\n                // Additional runtime check against size limit\n                if (totalRead > MAX_FILE_SIZE) {\n                    throw new SecurityException("File size exceeds limit during read");\n                }\n                output.write(buffer, 0, bytesRead);\n            }\n            \n            return output.toByteArray();\n        } catch (NoSuchFileException e) {\n            throw new FileNotFoundException("File not found");\n        }\n    }\n    \n    // HTTP handler for /img endpoint\n    static class ImgHandler implements HttpHandler {\n        @Override\n        public void handle(HttpExchange exchange) throws IOException {\n            try {\n                // Only accept GET requests\n                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {\n                    sendError(exchange, 405, "Method not allowed");\n                    return;\n                }\n                \n                // Parse query parameters\n                String query = exchange.getRequestURI().getQuery();\n                Map<String, String> params = parseQuery(query);\n                String name = params.get("name");\n                \n                if (name == null) {\n                    sendError(exchange, 400, "Missing name parameter");\n                    return;\n                }\n                \n                // Call img() function to get file content\n                byte[] fileContent = img(name);\n                \n                // Send response\n                exchange.getResponseHeaders().set("Content-Type", "application/octet-stream");\n                exchange.sendResponseHeaders(200, fileContent.length);\n                try (OutputStream os = exchange.getResponseBody()) {\n                    os.write(fileContent);\n                }\n            } catch (IllegalArgumentException | SecurityException e) {\n                sendError(exchange, 400, "Bad request");\n            } catch (FileNotFoundException e) {\n                sendError(exchange, 404, "Not found");\n            } catch (Exception e) {\n                sendError(exchange, 500, "Internal error");\n            }\n        }\n        \n        private void sendError(HttpExchange exchange, int code, String message) throws IOException {\n            byte[] response = message.getBytes(StandardCharsets.UTF_8);\n            exchange.sendResponseHeaders(code, response.length);\n            try (OutputStream os = exchange.getResponseBody()) {\n                os.write(response);\n            }\n        }\n        \n        private Map<String, String> parseQuery(String query) {\n            Map<String, String> result = new HashMap<>();\n            if (query != null && !query.isEmpty()) {\n                for (String param : query.split("&")) {\n                    String[] pair = param.split("=", 2);\n                    if (pair.length == 2) {\n                        result.put(pair[0], pair[1]);\n                    }\n                }\n            }\n            return result;\n        }\n    }\n    \n    public static void main(String[] args) throws Exception {\n        // Create test files\n        Files.write(BASE_DIR.resolve("test1.txt"), "Test file 1".getBytes(StandardCharsets.UTF_8));\n        Files.write(BASE_DIR.resolve("test2.jpg"), "Image data".getBytes(StandardCharsets.UTF_8));\n        Files.write(BASE_DIR.resolve("safe.png"), "Safe image".getBytes(StandardCharsets.UTF_8));\n        \n        // Test case 1: Valid file\n        try {\n            byte[] content = img("test1.txt");\n            System.out.println("Test 1 passed: " + new String(content, StandardCharsets.UTF_8));\n        } catch (Exception e) {\n            System.out.println("Test 1 failed: " + e.getMessage());\n        }\n        \n        // Test case 2: Another valid file\n        try {\n            byte[] content = img("test2.jpg");\n            System.out.println("Test 2 passed: " + new String(content, StandardCharsets.UTF_8));\n        } catch (Exception e) {\n            System.out.println("Test 2 failed: " + e.getMessage());\n        }\n        \n        // Test case 3: Path traversal attempt (should fail)\n        try {\n            byte[] content = img("../etc/passwd");\n            System.out.println("Test 3 FAILED: Path traversal succeeded");\n        } catch (SecurityException e) {\n            System.out.println("Test 3 passed: Path traversal blocked");\n        }\n        \n        // Test case 4: Null input (should fail)\n        try {\n            byte[] content = img(null);\n            System.out.println("Test 4 FAILED: Null accepted");\n        } catch (IllegalArgumentException e) {\n            System.out.println("Test 4 passed: Null rejected");\n        }\n        \n        // Test case 5: Non-existent file (should fail)\n        try {\n            byte[] content = img("nonexistent.txt");\n            System.out.println("Test 5 FAILED: Non-existent file returned data");\n        } catch (FileNotFoundException e) {\n            System.out.println("Test 5 passed: Non-existent file handled");\n        }\n        \n        System.out.println("\
Starting HTTP server on port 8000...");\n        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);\n        server.createContext("/img", new ImgHandler());
        server.setExecutor(null);
        server.start();
    }
}
