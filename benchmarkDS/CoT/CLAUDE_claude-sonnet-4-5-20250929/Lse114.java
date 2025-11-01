
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.stream.Stream;

public class Lse114 {
    // SECURE ALTERNATIVE: Use Java NIO instead of system commands
    public static void listDirectorySecure(String directoryPath) {
        try {
            Path path = Paths.get(directoryPath);
            
            // Validate path exists and is a directory
            if (!Files.exists(path) || !Files.isDirectory(path)) {
                System.out.println("Invalid directory path");
                return;
            }
            
            // Securely list directory contents
            try (Stream<Path> stream = Files.list(path)) {
                stream.forEach(file -> System.out.println(file.getFileName()));
            }
        } catch (Exception e) {
            System.out.println("Error accessing directory: " + e.getMessage());
        }
    }
    
    // WARNING: INSECURE - For educational purposes only
    // DO NOT USE IN PRODUCTION
    public static void listDirectoryUnsafe(String directory) {
        // This approach is vulnerable to command injection
        System.out.println("WARNING: This method is insecure and deprecated");
        // Intentionally not implementing unsafe system call
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Directory Listing ===\\n");
        
        // Test case 1: Current directory
        System.out.println("Test 1 - Current directory:");
        listDirectorySecure(".");
        
        // Test case 2: Parent directory
        System.out.println("\\nTest 2 - Parent directory:");
        listDirectorySecure("..");
        
        // Test case 3: Temp directory
        System.out.println("\\nTest 3 - Temp directory:");
        listDirectorySecure(System.getProperty("java.io.tmpdir"));
        
        // Test case 4: Invalid path
        System.out.println("\\nTest 4 - Invalid path:");
        listDirectorySecure("/nonexistent/path");
        
        // Test case 5: User home directory
        System.out.println("\\nTest 5 - Home directory:");
        listDirectorySecure(System.getProperty("user.home"));
    }
}
