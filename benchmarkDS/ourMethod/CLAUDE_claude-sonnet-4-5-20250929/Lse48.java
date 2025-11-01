
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;

public class Lse48 {
    // Secure base directory for serving images - must be set to a trusted directory
    private static final Path BASE_DIR = Paths.get("./images").toAbsolutePath().normalize();
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit

    public static byte[] img(String fileParam) throws IOException {
        // Input validation: reject null or empty file parameter
        if (fileParam == null || fileParam.trim().isEmpty()) {
            throw new IllegalArgumentException("File parameter is required");
        }

        // Normalize and resolve the requested path against the base directory
        // This prevents path traversal attacks
        Path requestedPath = BASE_DIR.resolve(fileParam).normalize();

        // Security check: ensure the resolved path is within BASE_DIR
        // This prevents directory traversal attacks like ../../../etc/passwd
        if (!requestedPath.startsWith(BASE_DIR)) {
            throw new SecurityException("Access denied: path traversal attempt detected");
        }

        // Open the file with secure flags - no symlink following
        // Use Files.newInputStream which is safer than raw File operations
        if (!Files.exists(requestedPath, java.nio.file.LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("File not found");
        }

        // Validate that it's a regular file (not a directory or symlink)\n        if (!Files.isRegularFile(requestedPath, java.nio.file.LinkOption.NOFOLLOW_LINKS)) {\n            throw new SecurityException("Access denied: not a regular file");\n        }\n\n        // Check file size before reading to prevent memory exhaustion\n        long fileSize = Files.size(requestedPath);\n        if (fileSize > MAX_FILE_SIZE) {\n            throw new IOException("File too large");\n        }\n\n        // Read the file content securely\n        // Files.readAllBytes is safe for bounded sizes\n        byte[] fileContent = Files.readAllBytes(requestedPath);\n        \n        return fileContent;\n    }\n\n    public static void main(String[] args) throws Exception {\n        // Create base directory if it doesn't exist
        Files.createDirectories(BASE_DIR);
        
        // Test cases
        System.out.println("Test Case 1: Valid image file");
        try {
            // Create a test file
            Path testFile1 = BASE_DIR.resolve("test1.jpg");
            Files.write(testFile1, new byte[]{(byte)0xFF, (byte)0xD8, (byte)0xFF, (byte)0xE0});
            byte[] result1 = img("test1.jpg");
            System.out.println("Success: Read " + result1.length + " bytes");
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }

        System.out.println("\\nTest Case 2: Path traversal attempt (should fail)");
        try {
            byte[] result2 = img("../../../etc/passwd");
            System.out.println("Error: Should have been blocked");
        } catch (SecurityException e) {
            System.out.println("Success: " + e.getMessage());
        }

        System.out.println("\\nTest Case 3: Null file parameter (should fail)");
        try {
            byte[] result3 = img(null);
            System.out.println("Error: Should have been blocked");
        } catch (IllegalArgumentException e) {
            System.out.println("Success: " + e.getMessage());
        }

        System.out.println("\\nTest Case 4: Non-existent file (should fail)");
        try {
            byte[] result4 = img("nonexistent.jpg");
            System.out.println("Error: Should have failed");
        } catch (IOException e) {
            System.out.println("Success: " + e.getMessage());
        }

        System.out.println("\\nTest Case 5: Empty filename (should fail)");
        try {
            byte[] result5 = img("");
            System.out.println("Error: Should have been blocked");
        } catch (IllegalArgumentException e) {
            System.out.println("Success: " + e.getMessage());
        }
    }
}
