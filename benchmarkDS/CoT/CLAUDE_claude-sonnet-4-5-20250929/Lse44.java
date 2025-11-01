
import java.io.*;
import java.nio.file.*;

public class Lse44 {
    private static final String SAFE_DIRECTORY = "/safe/";
    
    public static String readSafeFile(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            return "Error: Filename cannot be empty";
        }
        
        try {
            // Prevent path traversal attacks
            Path safePath = Paths.get(SAFE_DIRECTORY).toRealPath();
            Path requestedPath = safePath.resolve(filename).normalize();
            
            // Verify the resolved path is within safe directory
            if (!requestedPath.startsWith(safePath)) {
                return "Error: Access denied - file outside safe directory";
            }
            
            // Check if file exists and is readable
            if (!Files.exists(requestedPath) || !Files.isReadable(requestedPath)) {
                return "Error: File not found or not readable";
            }
            
            // Read file content
            return new String(Files.readAllBytes(requestedPath));
            
        } catch (IOException e) {
            return "Error: Unable to read file";
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1 - Normal file:");
        System.out.println(readSafeFile("test.txt"));
        
        System.out.println("\\nTest 2 - Path traversal attempt:");
        System.out.println(readSafeFile("../etc/passwd"));
        
        System.out.println("\\nTest 3 - Empty filename:");
        System.out.println(readSafeFile(""));
        
        System.out.println("\\nTest 4 - Null filename:");
        System.out.println(readSafeFile(null));
        
        System.out.println("\\nTest 5 - Non-existent file:");
        System.out.println(readSafeFile("nonexistent.txt"));
    }
}
