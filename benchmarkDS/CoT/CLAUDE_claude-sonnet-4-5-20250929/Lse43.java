
import java.io.*;
import java.nio.file.*;

public class Lse43 {
    public static String readFileFromSafeDirectory(String filename) {
        try {
            // Define the safe directory
            Path safeDir = Paths.get("/safe/").toAbsolutePath().normalize();
            
            // Construct the file path
            Path filePath = safeDir.resolve(filename).normalize();
            
            // Security check: ensure the resolved path is within safe directory
            if (!filePath.startsWith(safeDir)) {
                return "Error: Access denied - path traversal detected";
            }
            
            // Check if file exists and is readable
            if (!Files.exists(filePath) || !Files.isReadable(filePath)) {
                return "Error: File does not exist or is not readable";
            }
            
            // Read the file content
            StringBuilder content = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(new FileReader(filePath.toFile()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    content.append(line).append("\\n");
                }
            }
            
            return content.toString();
            
        } catch (IOException e) {
            return "Error: Unable to read file";
        } catch (Exception e) {
            return "Error: An unexpected error occurred";
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal file
        System.out.println("Test 1 - Normal file:");
        System.out.println(readFileFromSafeDirectory("test.txt"));
        
        // Test case 2: Path traversal attempt
        System.out.println("\\nTest 2 - Path traversal attempt:");
        System.out.println(readFileFromSafeDirectory("../etc/passwd"));
        
        // Test case 3: Another path traversal
        System.out.println("\\nTest 3 - Another traversal:");
        System.out.println(readFileFromSafeDirectory("../../secret.txt"));
        
        // Test case 4: Subdirectory file
        System.out.println("\\nTest 4 - Subdirectory:");
        System.out.println(readFileFromSafeDirectory("subdir/file.txt"));
        
        // Test case 5: Non-existent file
        System.out.println("\\nTest 5 - Non-existent file:");
        System.out.println(readFileFromSafeDirectory("nonexistent.txt"));
    }
}
