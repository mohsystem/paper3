
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse44 {
    private static final String SAFE_DIR = "/safe/";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
    /**
     * Reads a file from the /safe/ directory with path traversal protection.
     * @param filename The requested filename (relative path within /safe/)
     * @return byte array containing file contents, or null on error
     */
    public static byte[] readSafeFile(String filename) {
        if (filename == null || filename.isEmpty()) {
            System.err.println("Error: Filename cannot be null or empty");
            return null;
        }
        
        try {
            // Normalize the base directory path
            Path baseDir = Paths.get(SAFE_DIR).toRealPath();
            
            // Resolve the requested file against the base directory
            Path requestedPath = baseDir.resolve(filename).normalize();
            
            // Verify the resolved path is still within the base directory
            if (!requestedPath.startsWith(baseDir)) {
                System.err.println("Error: Path traversal attempt detected");
                return null;
            }
            
            // Open the file and validate it's a regular file
            File file = requestedPath.toFile();
            if (!file.exists()) {
                System.err.println("Error: File does not exist");
                return null;
            }
            
            if (!file.isFile()) {
                System.err.println("Error: Not a regular file");
                return null;
            }
            
            // Check file size before reading
            long fileSize = file.length();
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: File size exceeds limit");
                return null;
            }
            
            // Read file contents
            return Files.readAllBytes(requestedPath);
            
        } catch (IOException e) {
            System.err.println("Error: Unable to read file");
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid file in safe directory
        System.out.println("Test 1 - Valid file:");
        byte[] result1 = readSafeFile("test.txt");
        System.out.println(result1 != null ? "Success: Read " + result1.length + " bytes" : "Failed");
        
        // Test case 2: Path traversal attempt with ../
        System.out.println("\\nTest 2 - Path traversal attempt:");
        byte[] result2 = readSafeFile("../etc/passwd");
        System.out.println(result2 != null ? "SECURITY BREACH" : "Blocked as expected");
        
        // Test case 3: Absolute path attempt
        System.out.println("\\nTest 3 - Absolute path attempt:");
        byte[] result3 = readSafeFile("/etc/passwd");
        System.out.println(result3 != null ? "SECURITY BREACH" : "Blocked as expected");
        
        // Test case 4: Null filename
        System.out.println("\\nTest 4 - Null filename:");
        byte[] result4 = readSafeFile(null);
        System.out.println(result4 != null ? "Unexpected success" : "Handled correctly");
        
        // Test case 5: Empty filename
        System.out.println("\\nTest 5 - Empty filename:");
        byte[] result5 = readSafeFile("");
        System.out.println(result5 != null ? "Unexpected success" : "Handled correctly");
    }
}
