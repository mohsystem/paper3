
import java.io.*;
import java.nio.file.*;

public class Lse44 {
    public static String readFileFromSafeDirectory(String filename) {
        try {
            // Construct the safe directory path
            String safeDirectory = "/safe/";
            Path filePath = Paths.get(safeDirectory, filename).normalize();
            
            // Security check: ensure the resolved path is still within /safe/
            if (!filePath.startsWith("/safe/")) {
                return "Error: Access denied - path traversal attempt detected";
            }
            
            // Read the file content
            StringBuilder content = new StringBuilder();
            BufferedReader reader = new BufferedReader(new FileReader(filePath.toFile()));
            String line;
            while ((line = reader.readLine()) != null) {
                content.append(line).append("\\n");
            }
            reader.close();
            return content.toString();
        } catch (IOException e) {
            return "Error reading file: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: Reading 'document.txt'");
        System.out.println(readFileFromSafeDirectory("document.txt"));
        
        System.out.println("\\nTest Case 2: Reading 'data.txt'");
        System.out.println(readFileFromSafeDirectory("data.txt"));
        
        System.out.println("\\nTest Case 3: Path traversal attempt '../etc/passwd'");
        System.out.println(readFileFromSafeDirectory("../etc/passwd"));
        
        System.out.println("\\nTest Case 4: Reading subdirectory file 'subdir/file.txt'");
        System.out.println(readFileFromSafeDirectory("subdir/file.txt"));
        
        System.out.println("\\nTest Case 5: Non-existent file 'nonexistent.txt'");
        System.out.println(readFileFromSafeDirectory("nonexistent.txt"));
    }
}
