
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse45 {
    private static final String SAFE_DIRECTORY = "/var/safe_files/";
    
    public static String readFileFromSafeDirectory(String filename) {
        try {
            // Sanitize the filename to prevent path traversal attacks
            String sanitizedFilename = Paths.get(filename).getFileName().toString();
            
            // Construct the full path within the safe directory
            Path safePath = Paths.get(SAFE_DIRECTORY).normalize().toAbsolutePath();
            Path filePath = safePath.resolve(sanitizedFilename).normalize().toAbsolutePath();
            
            // Verify the file is still within the safe directory
            if (!filePath.startsWith(safePath)) {
                return "Error: Path traversal attempt detected";
            }
            
            // Check if file exists
            if (!Files.exists(filePath)) {
                return "Error: File not found";
            }
            
            // Read and return file content
            return Files.readString(filePath);
            
        } catch (IOException e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + readFileFromSafeDirectory("document.txt"));
        System.out.println("Test 2: " + readFileFromSafeDirectory("data.json"));
        System.out.println("Test 3: " + readFileFromSafeDirectory("../etc/passwd"));
        System.out.println("Test 4: " + readFileFromSafeDirectory("nonexistent.txt"));
        System.out.println("Test 5: " + readFileFromSafeDirectory("config.ini"));
    }
}
