
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse44 {
    private static final String SAFE_DIRECTORY = "/safe/";
    
    public static String readFileFromSafeDirectory(String requestedFile) {
        try {
            // Normalize and resolve the path
            Path safePath = Paths.get(SAFE_DIRECTORY).toAbsolutePath().normalize();
            Path requestedPath = safePath.resolve(requestedFile).normalize();
            
            // Security check: ensure the resolved path is still within the safe directory
            if (!requestedPath.startsWith(safePath)) {
                return "Error: Access denied - Path traversal attempt detected";
            }
            
            // Check if file exists
            if (!Files.exists(requestedPath)) {
                return "Error: File not found";
            }
            
            // Read and return file content
            return Files.readString(requestedPath);
            
        } catch (IOException e) {
            return "Error: Unable to read file - " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1 - Normal file:");
        System.out.println(readFileFromSafeDirectory("test.txt"));
        
        System.out.println("\\nTest 2 - Path traversal attempt:");
        System.out.println(readFileFromSafeDirectory("../../../etc/passwd"));
        
        System.out.println("\\nTest 3 - Another path traversal:");
        System.out.println(readFileFromSafeDirectory("..\\\\..\\\\windows\\\\system32\\\\config\\\\sam"));
        
        System.out.println("\\nTest 4 - Non-existent file:");
        System.out.println(readFileFromSafeDirectory("nonexistent.txt"));
        
        System.out.println("\\nTest 5 - Subdirectory file:");
        System.out.println(readFileFromSafeDirectory("subdir/file.txt"));
    }
}
