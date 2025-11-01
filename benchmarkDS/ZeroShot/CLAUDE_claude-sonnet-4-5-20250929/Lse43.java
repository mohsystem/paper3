
import java.io.*;
import java.nio.file.*;

public class Lse43 {
    /**
     * Securely reads a file from the /safe/ directory
     * @param filename The name of the file to read (without path)
     * @return The content of the file as a String, or error message
     */
    public static String readSafeFile(String filename) {
        try {
            // Validate filename to prevent path traversal attacks
            if (filename == null || filename.isEmpty()) {
                return "Error: Filename cannot be empty";
            }
            
            // Remove any path traversal attempts
            String sanitizedFilename = new File(filename).getName();
            
            // Construct the safe path
            Path safePath = Paths.get("/safe/", sanitizedFilename).normalize();
            
            // Verify the path is still within /safe/ directory
            if (!safePath.startsWith("/safe/")) {
                return "Error: Access denied - Path traversal attempt detected";
            }
            
            // Check if file exists
            if (!Files.exists(safePath)) {
                return "Error: File not found";
            }
            
            // Read the file
            return new String(Files.readAllBytes(safePath));
            
        } catch (IOException e) {
            return "Error: Unable to read file - " + e.getMessage();
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: Normal file");
        System.out.println(readSafeFile("test.txt"));
        System.out.println();
        
        System.out.println("Test Case 2: Path traversal attempt with ../");
        System.out.println(readSafeFile("../etc/passwd"));
        System.out.println();
        
        System.out.println("Test Case 3: Absolute path attempt");
        System.out.println(readSafeFile("/etc/passwd"));
        System.out.println();
        
        System.out.println("Test Case 4: Empty filename");
        System.out.println(readSafeFile(""));
        System.out.println();
        
        System.out.println("Test Case 5: Null filename");
        System.out.println(readSafeFile(null));
    }
}
