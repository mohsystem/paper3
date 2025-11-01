
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.stream.Stream;

public class Lse116 {
    // Secure method: Uses Java's File API instead of system()
    public static void listDirectory(String directoryPath) {
        try {
            Path path = Paths.get(directoryPath);
            
            // Validate path exists and is a directory
            if (!Files.exists(path)) {
                System.out.println("Directory does not exist: " + directoryPath);
                return;
            }
            
            if (!Files.isDirectory(path)) {
                System.out.println("Path is not a directory: " + directoryPath);
                return;
            }
            
            System.out.println("Contents of " + directoryPath + ":");
            try (Stream<Path> entries = Files.list(path)) {
                entries.forEach(entry -> System.out.println(entry.getFileName()));
            }
        } catch (IOException e) {
            System.out.println("Error listing directory: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Current directory ===");
        listDirectory(".");
        
        System.out.println("\\n=== Test Case 2: Home directory ===");
        listDirectory(System.getProperty("user.home"));
        
        System.out.println("\\n=== Test Case 3: Temp directory ===");
        listDirectory(System.getProperty("java.io.tmpdir"));
        
        System.out.println("\\n=== Test Case 4: Non-existent directory ===");
        listDirectory("/nonexistent/path");
        
        System.out.println("\\n=== Test Case 5: Invalid path ===");
        listDirectory("");
    }
}
