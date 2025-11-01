
import java.io.*;
import java.nio.file.*;
import java.nio.charset.StandardCharsets;

public class Lse43 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    private static final String BASE_DIR = "/safe/";

    /**
     * Securely reads a file from /safe/ directory
     * @param filename The name of the file to read (relative to /safe/)
     * @return File contents as String, or null on error
     */
    public static String readSafeFile(String filename) {
        // Input validation - reject null or empty
        if (filename == null || filename.trim().isEmpty()) {
            System.err.println("Invalid filename");
            return null;
        }

        // Reject obvious traversal attempts
        if (filename.contains("..") || filename.contains("/") || filename.contains("\\\\")) {\n            System.err.println("Invalid filename characters");\n            return null;\n        }\n\n        try {\n            // Construct path within base directory\n            Path basePath = Paths.get(BASE_DIR).toRealPath();\n            Path filePath = basePath.resolve(filename).normalize();\n\n            // Verify resolved path is still within base directory (path traversal check)\n            if (!filePath.startsWith(basePath)) {\n                System.err.println("Path traversal detected");\n                return null;\n            }\n\n            // Open file with follow links disabled\n            File file = filePath.toFile();\n            \n            // Check if regular file and size before reading\n            if (!file.isFile()) {\n                System.err.println("Not a regular file");\n                return null;\n            }\n\n            if (file.length() > MAX_FILE_SIZE) {\n                System.err.println("File too large");\n                return null;\n            }\n\n            // Read file securely\n            byte[] bytes = Files.readAllBytes(filePath);\n            return new String(bytes, StandardCharsets.UTF_8);\n\n        } catch (IOException e) {\n            System.err.println("File operation failed");\n            return null;\n        } catch (Exception e) {\n            System.err.println("Unexpected error");\n            return null;\n        }\n    }\n\n    public static void main(String[] args) {\n        // Test case 1: Valid file\n        System.out.println("Test 1: Valid file");\n        String result1 = readSafeFile("test.txt");\n        System.out.println("Result: " + (result1 != null ? "Success" : "Failed"));\n\n        // Test case 2: Path traversal attempt\n        System.out.println("\
Test 2: Path traversal with ..");\n        String result2 = readSafeFile("../etc/passwd");\n        System.out.println("Result: " + (result2 == null ? "Blocked" : "Failed"));\n\n        // Test case 3: Absolute path attempt\n        System.out.println("\
Test 3: Absolute path");\n        String result3 = readSafeFile("/etc/passwd");\n        System.out.println("Result: " + (result3 == null ? "Blocked" : "Failed"));\n\n        // Test case 4: Null input\n        System.out.println("\
Test 4: Null input");\n        String result4 = readSafeFile(null);\n        System.out.println("Result: " + (result4 == null ? "Blocked" : "Failed"));\n\n        // Test case 5: Empty filename\n        System.out.println("\
Test 5: Empty filename");\n        String result5 = readSafeFile("");\n        System.out.println("Result: " + (result5 == null ? "Blocked" : "Failed"));
    }
}
