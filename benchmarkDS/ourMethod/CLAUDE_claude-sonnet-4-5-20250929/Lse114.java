
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;
import java.util.stream.Stream;

public class Lse114 {
    /**
     * Secure directory listing - DOES NOT use system() to avoid command injection.
     * Uses Java NIO for safe file system operations.
     */
    public static String listDirectory(String directoryPath) {
        // Input validation: reject null, empty, or path traversal attempts
        if (directoryPath == null || directoryPath.trim().isEmpty()) {
            return "Error: Invalid directory path";
        }
        
        // Normalize and validate path to prevent traversal attacks
        try {
            Path path = Paths.get(directoryPath).normalize().toAbsolutePath();
            
            // Additional security: reject if path contains traversal sequences after normalization
            String normalized = path.toString();
            if (normalized.contains("..") || !Files.exists(path)) {
                return "Error: Invalid or non-existent directory";
            }
            
            // Check if it's a directory\n            if (!Files.isDirectory(path)) {\n                return "Error: Path is not a directory";\n            }\n            \n            // List directory contents securely using Java NIO\n            StringBuilder result = new StringBuilder();\n            result.append("Contents of: ").append(path).append("\\n");\n            \n            try (Stream<Path> entries = Files.list(path)) {\n                entries.forEach(entry -> {\n                    result.append(entry.getFileName()).append("\\n");\n                });\n            }\n            \n            return result.toString();\n            \n        } catch (Exception e) {\n            // Generic error message - don't leak internal details
            return "Error: Cannot access directory";
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Current directory
        System.out.println("=== Test 1: Current directory ===");
        System.out.println(listDirectory("."));
        
        // Test case 2: Temp directory
        System.out.println("\\n=== Test 2: Temp directory ===");
        System.out.println(listDirectory(System.getProperty("java.io.tmpdir")));
        
        // Test case 3: Invalid path (security test)
        System.out.println("\\n=== Test 3: Path traversal attempt ===");
        System.out.println(listDirectory("../../../etc"));
        
        // Test case 4: Null input
        System.out.println("\\n=== Test 4: Null input ===");
        System.out.println(listDirectory(null));
        
        // Test case 5: Empty string
        System.out.println("\\n=== Test 5: Empty string ===");
        System.out.println(listDirectory(""));
    }
}
