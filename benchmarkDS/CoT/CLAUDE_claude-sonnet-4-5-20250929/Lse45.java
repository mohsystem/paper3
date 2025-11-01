
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse45 {
    private static final String SAFE_DIRECTORY = "/var/safe_files/";
    
    public static String readFileFromSafeDirectory(String filename) {
        try {
            // Input validation
            if (filename == null || filename.isEmpty()) {
                return "Error: Invalid filename";
            }
            
            // Sanitize filename - remove any path separators
            String sanitizedFilename = filename.replaceAll("[/\\\\\\\\]", "");
            
            // Create paths
            Path safeDir = Paths.get(SAFE_DIRECTORY).toRealPath();
            Path requestedFile = safeDir.resolve(sanitizedFilename).normalize();
            
            // Security check: ensure the resolved path is within safe directory
            if (!requestedFile.toRealPath().startsWith(safeDir)) {
                return "Error: Access denied - file outside safe directory";
            }
            
            // Check if file exists
            if (!Files.exists(requestedFile)) {
                return "Error: File not found";
            }
            
            // Check if it's a regular file
            if (!Files.isRegularFile(requestedFile)) {
                return "Error: Not a regular file";
            }
            
            // Read and return file content
            return new String(Files.readAllBytes(requestedFile));
            
        } catch (IOException e) {
            return "Error: Unable to read file";
        } catch (SecurityException e) {
            return "Error: Access denied";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1 - Valid file:");
        System.out.println(readFileFromSafeDirectory("document.txt"));
        
        System.out.println("\\nTest 2 - Path traversal attempt:");
        System.out.println(readFileFromSafeDirectory("../etc/passwd"));
        
        System.out.println("\\nTest 3 - Absolute path attempt:");
        System.out.println(readFileFromSafeDirectory("/etc/passwd"));
        
        System.out.println("\\nTest 4 - Null filename:");
        System.out.println(readFileFromSafeDirectory(null));
        
        System.out.println("\\nTest 5 - Non-existent file:");
        System.out.println(readFileFromSafeDirectory("nonexistent.txt"));
    }
}
