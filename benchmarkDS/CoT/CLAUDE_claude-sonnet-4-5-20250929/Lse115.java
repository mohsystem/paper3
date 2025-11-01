
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.stream.Stream;

public class Lse115 {
    // SECURE: Uses Java's built-in file system API instead of executing shell commands
    public static void listDirectory(String directoryPath) {
        try {
            Path path = Paths.get(directoryPath);
            
            // Validate path exists and is a directory
            if (!Files.exists(path) || !Files.isDirectory(path)) {
                System.out.println("Invalid directory: " + directoryPath);
                return;
            }
            
            System.out.println("Contents of " + directoryPath + ":");
            try (Stream<Path> paths = Files.list(path)) {
                paths.forEach(p -> System.out.println(p.getFileName()));
            }
        } catch (IOException e) {
            System.err.println("Error reading directory: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test cases using safe directory listing
        System.out.println("Test 1: Current directory");
        listDirectory(".");
        
        System.out.println("\\nTest 2: Parent directory");
        listDirectory("..");
        
        System.out.println("\\nTest 3: Temp directory");
        listDirectory(System.getProperty("java.io.tmpdir"));
        
        System.out.println("\\nTest 4: User home");
        listDirectory(System.getProperty("user.home"));
        
        System.out.println("\\nTest 5: Invalid directory");
        listDirectory("/nonexistent/path");
    }
}
