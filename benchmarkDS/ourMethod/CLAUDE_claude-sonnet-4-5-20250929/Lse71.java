
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.*;
import java.util.concurrent.Executors;

public class Lse71 {
    // Security: Define maximum file size to prevent DoS (10MB)
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024;
    // Security: Define allowed file extensions whitelist
    private static final Set<String> ALLOWED_EXTENSIONS = new HashSet<>(Arrays.asList(".txt", ".pdf", ".jpg", ".png"));
    // Security: Define upload directory - must be outside web root
    private static final String UPLOAD_DIR = "uploads";
    // Security: Maximum filename length to prevent buffer issues
    private static final int MAX_FILENAME_LENGTH = 255;

    public static void main(String[] args) {
        try {
            // Security: Create upload directory with restricted permissions
            Path uploadPath = Paths.get(UPLOAD_DIR);
            if (!Files.exists(uploadPath)) {
                Files.createDirectory(uploadPath);
                // Security: Set restrictive permissions (owner read/write only)
                try {
                    Set<PosixFilePermission> perms = new HashSet<>();
                    perms.add(PosixFilePermission.OWNER_READ);
                    perms.add(PosixFilePermission.OWNER_WRITE);
                    perms.add(PosixFilePermission.OWNER_EXECUTE);
                    Files.setPosixFilePermissions(uploadPath, perms);
                } catch (UnsupportedOperationException e) {
                    // Windows doesn't support POSIX permissions\n                    uploadPath.toFile().setReadable(true, true);\n                    uploadPath.toFile().setWritable(true, true);\n                }\n            }\n\n            HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);\n            server.createContext("/upload", new UploadHandler());\n            server.setExecutor(Executors.newFixedThreadPool(10));\n            server.start();\n            System.out.println("Server started on port 8080");\n\n            // Test cases\n            runTests();\n\n        } catch (IOException e) {\n            System.err.println("Failed to start server");\n            e.printStackTrace();\n        }\n    }\n\n    static class UploadHandler implements HttpHandler {\n        @Override\n        public void handle(HttpExchange exchange) throws IOException {\n            String response;\n            int statusCode;\n\n            try {\n                // Security: Only accept POST requests\n                if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {\n                    response = "Method not allowed";\n                    statusCode = 405;\n                    sendResponse(exchange, response, statusCode);\n                    return;\n                }\n\n                // Security: Validate Content-Type header\n                String contentType = exchange.getRequestHeaders().getFirst("Content-Type");\n                if (contentType == null || !contentType.startsWith("multipart/form-data")) {\n                    response = "Invalid content type";\n                    statusCode = 400;\n                    sendResponse(exchange, response, statusCode);\n                    return;\n                }\n\n                String boundary = extractBoundary(contentType);\n                if (boundary == null) {\n                    response = "Invalid boundary";\n                    statusCode = 400;\n                    sendResponse(exchange, response, statusCode);\n                    return;\n                }\n\n                // Security: Read input with size limit\n                InputStream input = exchange.getRequestBody();\n                String result = processUpload(input, boundary);\n                \n                response = result;\n                statusCode = result.startsWith("Error") ? 400 : 200;\n\n            } catch (Exception e) {\n                // Security: Return generic error message, log details separately\n                response = "Upload failed";\n                statusCode = 500;\n                System.err.println("Upload error: " + e.getMessage());\n            }\n\n            sendResponse(exchange, response, statusCode);\n        }\n\n        private String extractBoundary(String contentType) {\n            // Security: Validate and extract boundary from Content-Type\n            String[] parts = contentType.split(";");\n            for (String part : parts) {\n                part = part.trim();\n                if (part.startsWith("boundary=")) {\n                    return "--" + part.substring(9);\n                }\n            }\n            return null;\n        }\n\n        private String processUpload(InputStream input, String boundary) {\n            try {\n                // Security: Use limited reader to prevent memory exhaustion\n                BufferedReader reader = new BufferedReader(\n                    new InputStreamReader(input, StandardCharsets.UTF_8)\n                );\n\n                String line;\n                String filename = null;\n                ByteArrayOutputStream fileContent = new ByteArrayOutputStream();\n                boolean inFileContent = false;\n                long totalBytesRead = 0;\n\n                while ((line = reader.readLine()) != null) {\n                    // Security: Check total size to prevent DoS\n                    totalBytesRead += line.getBytes(StandardCharsets.UTF_8).length;\n                    if (totalBytesRead > MAX_FILE_SIZE) {\n                        return "Error: File size exceeds maximum limit";\n                    }\n\n                    if (line.startsWith(boundary)) {\n                        if (inFileContent && filename != null) {\n                            // Save the file\n                            String result = saveFile(filename, fileContent.toByteArray());\n                            if (result != null) {\n                                return result;\n                            }\n                        }\n                        inFileContent = false;\n                        fileContent.reset();\n                        continue;\n                    }\n\n                    if (line.toLowerCase().contains("content-disposition")) {\n                        filename = extractFilename(line);\n                        if (filename == null) {\n                            return "Error: Invalid filename";\n                        }\n                        continue;\n                    }\n\n                    if (line.toLowerCase().contains("content-type")) {\n                        reader.readLine(); // Skip blank line after headers\n                        inFileContent = true;\n                        continue;\n                    }\n\n                    if (inFileContent) {\n                        fileContent.write(line.getBytes(StandardCharsets.UTF_8));\n                        fileContent.write('\
');\n                    }\n                }\n\n                return "File uploaded successfully";\n\n            } catch (IOException e) {\n                return "Error: Upload failed";\n            }\n        }\n\n        private String extractFilename(String header) {\n            // Security: Extract and validate filename from Content-Disposition header\n            int filenameIndex = header.indexOf("filename=\\"");\n            if (filenameIndex == -1) {\n                return null;\n            }\n\n            int startIndex = filenameIndex + 10;\n            int endIndex = header.indexOf("\\"", startIndex);\n            if (endIndex == -1) {\n                return null;\n            }\n\n            String filename = header.substring(startIndex, endIndex);\n            \n            // Security: Validate filename length\n            if (filename.length() > MAX_FILENAME_LENGTH) {\n                return null;\n            }\n\n            // Security: Remove path traversal attempts\n            filename = new File(filename).getName();\n            \n            // Security: Validate filename contains only safe characters\n            if (!filename.matches("^[a-zA-Z0-9_.-]+$")) {\n                return null;\n            }\n\n            // Security: Check file extension against whitelist\n            String extension = "";\n            int dotIndex = filename.lastIndexOf('.');\n            if (dotIndex > 0) {\n                extension = filename.substring(dotIndex).toLowerCase();\n            }\n            \n            if (!ALLOWED_EXTENSIONS.contains(extension)) {\n                return null;\n            }\n\n            return filename;\n        }\n\n        private String saveFile(String filename, byte[] content) {\n            try {\n                // Security: Validate content size\n                if (content.length > MAX_FILE_SIZE) {\n                    return "Error: File too large";\n                }\n\n                // Security: Generate random unique filename to prevent overwrites\n                SecureRandom random = new SecureRandom();\n                byte[] randomBytes = new byte[16];\n                random.nextBytes(randomBytes);\n                MessageDigest md = MessageDigest.getInstance("SHA-256");\n                byte[] hash = md.digest(randomBytes);\n                String uniquePrefix = Base64.getEncoder().encodeToString(hash).substring(0, 16);\n                String safeFilename = uniquePrefix + "_" + filename;\n\n                // Security: Construct path safely within upload directory\n                Path uploadDir = Paths.get(UPLOAD_DIR).toRealPath();\n                Path targetPath = uploadDir.resolve(safeFilename).normalize();\n\n                // Security: Verify resolved path is still within upload directory\n                if (!targetPath.startsWith(uploadDir)) {\n                    return "Error: Invalid file path";\n                }\n\n                // Security: Use atomic write operation with temp file\n                Path tempFile = Files.createTempFile(uploadDir, "upload_", ".tmp");\n                \n                try {\n                    // Security: Write to temp file first\n                    Files.write(tempFile, content);\n                    \n                    // Security: Set restrictive permissions before moving\n                    try {\n                        Set<PosixFilePermission> perms = new HashSet<>();\n                        perms.add(PosixFilePermission.OWNER_READ);\n                        perms.add(PosixFilePermission.OWNER_WRITE);\n                        Files.setPosixFilePermissions(tempFile, perms);\n                    } catch (UnsupportedOperationException e) {\n                        // Windows fallback\n                        tempFile.toFile().setReadable(true, true);\n                        tempFile.toFile().setWritable(true, true);\n                    }\n\n                    // Security: Atomic move to final location\n                    Files.move(tempFile, targetPath, StandardCopyOption.ATOMIC_MOVE, \n                              StandardCopyOption.REPLACE_EXISTING);\n\n                } catch (IOException e) {\n                    // Security: Clean up temp file on failure\n                    try {\n                        Files.deleteIfExists(tempFile);\n                    } catch (IOException ex) {\n                        // Log but don't expose error details
                    }
                    return "Error: Failed to save file";
                }

                return null; // Success

            } catch (Exception e) {
                return "Error: File save failed";
            }
        }

        private void sendResponse(HttpExchange exchange, String response, int statusCode) 
                throws IOException {
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }

    private static void runTests() {
        System.out.println("\\n=== Running Test Cases ===");
        
        // Test 1: Valid filename
        System.out.println("Test 1: Valid filename 'test.txt' - Expected: Pass");
        System.out.println("Result: " + (isValidFilename("test.txt") ? "PASS" : "FAIL"));

        // Test 2: Path traversal attempt
        System.out.println("\\nTest 2: Path traversal '../etc/passwd' - Expected: Fail");
        System.out.println("Result: " + (!isValidFilename("../etc/passwd") ? "PASS" : "FAIL"));

        // Test 3: Invalid extension
        System.out.println("\\nTest 3: Invalid extension 'file.exe' - Expected: Fail");
        System.out.println("Result: " + (!isValidFilename("file.exe") ? "PASS" : "FAIL"));

        // Test 4: Special characters
        System.out.println("\\nTest 4: Special characters 'file<>.txt' - Expected: Fail");
        System.out.println("Result: " + (!isValidFilename("file<>.txt") ? "PASS" : "FAIL"));

        // Test 5: Maximum length
        System.out.println("\\nTest 5: Filename exceeds max length - Expected: Fail");
        String longName = "a".repeat(256) + ".txt";
        System.out.println("Result: " + (!isValidFilename(longName) ? "PASS" : "FAIL"));
    }

    private static boolean isValidFilename(String filename) {
        if (filename == null || filename.length() > MAX_FILENAME_LENGTH) {
            return false;
        }
        
        filename = new File(filename).getName();
        
        if (!filename.matches("^[a-zA-Z0-9_.-]+$")) {
            return false;
        }

        String extension = "";
        int dotIndex = filename.lastIndexOf('.');
        if (dotIndex > 0) {
            extension = filename.substring(dotIndex).toLowerCase();
        }
        
        return ALLOWED_EXTENSIONS.contains(extension);
    }
}
