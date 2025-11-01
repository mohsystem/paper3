
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.SecureRandom;
import java.util.regex.Pattern;

public class Lse70 {
    private static final Path UPLOAD_DIR = Paths.get("uploads").toAbsolutePath().normalize();
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB max file size
    private static final Pattern SAFE_FILENAME = Pattern.compile("^[a-zA-Z0-9_.-]+$");
    private static final int MAX_FILENAME_LENGTH = 255;

    public static void main(String[] args) throws IOException {
        // Create uploads directory if it doesn't exist with restrictive permissions\n        if (!Files.exists(UPLOAD_DIR)) {\n            Files.createDirectories(UPLOAD_DIR);\n            // Set restrictive permissions (owner read/write/execute only)\n            try {\n                Files.setPosixFilePermissions(UPLOAD_DIR, \n                    java.nio.file.attribute.PosixFilePermissions.fromString("rwx------"));\n            } catch (UnsupportedOperationException e) {\n                // Windows doesn't support POSIX permissions
            }
        }

        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/upload", new UploadHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 8000");

        // Test cases
        runTestCases();
    }

    static class UploadHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equals(exchange.getRequestMethod())) {
                String response = uploadFile(exchange.getRequestBody(), 
                    exchange.getRequestHeaders().getFirst("Content-Type"));
                exchange.sendResponseHeaders(200, response.length());
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
            } else {
                String response = "Method not allowed";
                exchange.sendResponseHeaders(405, response.length());
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
            }
        }
    }

    public static String uploadFile(InputStream inputStream, String contentType) {
        // Validate content type (basic check)
        if (contentType == null || !contentType.startsWith("multipart/form-data")) {
            return "Error: Invalid content type";
        }

        try {
            // Read input with size limit to prevent memory exhaustion
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            byte[] data = new byte[8192];
            int bytesRead;
            long totalRead = 0;

            while ((bytesRead = inputStream.read(data, 0, data.length)) != -1) {
                totalRead += bytesRead;
                // Enforce maximum file size to prevent DoS
                if (totalRead > MAX_FILE_SIZE) {
                    return "Error: File too large";
                }
                buffer.write(data, 0, bytesRead);
            }

            byte[] fileContent = buffer.toByteArray();
            
            // Generate a cryptographically secure random filename to prevent overwrite attacks
            SecureRandom random = new SecureRandom();
            byte[] randomBytes = new byte[16];
            random.nextBytes(randomBytes);
            StringBuilder sb = new StringBuilder();
            for (byte b : randomBytes) {
                sb.append(String.format("%02x", b));
            }
            String safeFilename = sb.toString() + ".bin";

            // Validate filename length
            if (safeFilename.length() > MAX_FILENAME_LENGTH) {
                return "Error: Filename too long";
            }

            // Construct the target path and normalize it
            Path targetPath = UPLOAD_DIR.resolve(safeFilename).normalize();

            // Security check: ensure the resolved path is still within UPLOAD_DIR
            // This prevents directory traversal attacks
            if (!targetPath.startsWith(UPLOAD_DIR)) {
                return "Error: Invalid file path";
            }

            // Write to a temporary file first with restrictive permissions
            Path tempFile = Files.createTempFile(UPLOAD_DIR, "upload_", ".tmp");
            try {
                // Set restrictive permissions immediately on creation (owner read/write only)
                try {
                    Files.setPosixFilePermissions(tempFile, 
                        java.nio.file.attribute.PosixFilePermissions.fromString("rw-------"));
                } catch (UnsupportedOperationException e) {
                    // Windows doesn't support POSIX permissions
                }

                // Write content to temp file
                Files.write(tempFile, fileContent, StandardOpenOption.WRITE);

                // Atomic move to final location using ATOMIC_MOVE and REPLACE_EXISTING
                // This prevents partial writes and race conditions
                Files.move(tempFile, targetPath, 
                    StandardCopyOption.ATOMIC_MOVE, 
                    StandardCopyOption.REPLACE_EXISTING);

                return "File uploaded successfully: " + safeFilename;
            } catch (Exception e) {
                // Clean up temp file on error
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException ignored) {
                }
                return "Error: Upload failed";
            }

        } catch (IOException e) {
            return "Error: Upload failed";
        }
    }

    private static void runTestCases() {
        System.out.println("\\n=== Running Test Cases ===");
        
        // Test case 1: Valid small file upload
        try {
            ByteArrayInputStream test1 = new ByteArrayInputStream("Test content 1".getBytes(StandardCharsets.UTF_8));
            String result1 = uploadFile(test1, "multipart/form-data");
            System.out.println("Test 1 (valid upload): " + result1);
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }

        // Test case 2: Another valid upload
        try {
            ByteArrayInputStream test2 = new ByteArrayInputStream("Test content 2".getBytes(StandardCharsets.UTF_8));
            String result2 = uploadFile(test2, "multipart/form-data");
            System.out.println("Test 2 (valid upload): " + result2);
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }

        // Test case 3: Invalid content type
        try {
            ByteArrayInputStream test3 = new ByteArrayInputStream("Test content 3".getBytes(StandardCharsets.UTF_8));
            String result3 = uploadFile(test3, "text/plain");
            System.out.println("Test 3 (invalid content type): " + result3);
        } catch (Exception e) {
            System.out.println("Test 3 failed: " + e.getMessage());
        }

        // Test case 4: Large file (simulated)
        try {
            byte[] largeContent = new byte[1024];
            ByteArrayInputStream test4 = new ByteArrayInputStream(largeContent);
            String result4 = uploadFile(test4, "multipart/form-data");
            System.out.println("Test 4 (1KB file): " + result4);
        } catch (Exception e) {
            System.out.println("Test 4 failed: " + e.getMessage());
        }

        // Test case 5: Empty file
        try {
            ByteArrayInputStream test5 = new ByteArrayInputStream(new byte[0]);
            String result5 = uploadFile(test5, "multipart/form-data");
            System.out.println("Test 5 (empty file): " + result5);
        } catch (Exception e) {
            System.out.println("Test 5 failed: " + e.getMessage());
        }
    }
}
