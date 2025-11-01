
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Base64;

public class Lse48 {
    private static final String ALLOWED_DIRECTORY = "./images/";
    
    public static byte[] img(String fileParam) {
        if (fileParam == null || fileParam.isEmpty()) {
            System.err.println("File parameter is required");
            return null;
        }
        
        // Secure: Sanitize input to prevent path traversal
        String sanitizedFile = fileParam.replaceAll("[^a-zA-Z0-9._-]", "");
        
        // Prevent path traversal attempts
        if (sanitizedFile.contains("..") || sanitizedFile.contains("/") || sanitizedFile.contains("\\\\")) {\n            System.err.println("Invalid file name");\n            return null;\n        }\n        \n        try {\n            Path filePath = Paths.get(ALLOWED_DIRECTORY, sanitizedFile).normalize();\n            Path allowedPath = Paths.get(ALLOWED_DIRECTORY).toRealPath();\n            \n            // Ensure the file is within allowed directory\n            if (!filePath.toRealPath().startsWith(allowedPath)) {\n                System.err.println("Access denied: Path traversal attempt detected");\n                return null;\n            }\n            \n            // Check if file exists and is readable\n            if (!Files.exists(filePath) || !Files.isReadable(filePath)) {\n                System.err.println("File not found or not readable");\n                return null;\n            }\n            \n            return Files.readAllBytes(filePath);\n        } catch (IOException e) {\n            System.err.println("Error reading file: " + e.getMessage());\n            return null;\n        }\n    }\n    \n    public static void main(String[] args) {\n        // Create test directory and files (for demonstration)\n        try {\n            Files.createDirectories(Paths.get(ALLOWED_DIRECTORY));\n            Files.write(Paths.get(ALLOWED_DIRECTORY, "test1.txt"), "Test content 1".getBytes());\n            Files.write(Paths.get(ALLOWED_DIRECTORY, "test2.jpg"), "Image content".getBytes());\n        } catch (IOException e) {\n            System.err.println("Setup error: " + e.getMessage());\n        }\n        \n        // Test case 1: Valid file\n        System.out.println("Test 1 - Valid file:");\n        byte[] result1 = img("test1.txt");\n        System.out.println(result1 != null ? "Success: " + new String(result1) : "Failed");\n        \n        // Test case 2: Another valid file\n        System.out.println("\
Test 2 - Valid image file:");\n        byte[] result2 = img("test2.jpg");\n        System.out.println(result2 != null ? "Success: " + new String(result2) : "Failed");\n        \n        // Test case 3: Path traversal attempt (should be blocked)\n        System.out.println("\
Test 3 - Path traversal attempt:");\n        byte[] result3 = img("../etc/passwd");\n        System.out.println(result3 != null ? "Success" : "Blocked (Expected)");\n        \n        // Test case 4: Null parameter\n        System.out.println("\
Test 4 - Null parameter:");\n        byte[] result4 = img(null);\n        System.out.println(result4 != null ? "Success" : "Blocked (Expected)");\n        \n        // Test case 5: Non-existent file\n        System.out.println("\
Test 5 - Non-existent file:");\n        byte[] result5 = img("nonexistent.txt");\n        System.out.println(result5 != null ? "Success" : "Failed (Expected)");
    }
}
