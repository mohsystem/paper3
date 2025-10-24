
import java.io.*;
import java.nio.file.*;

public class Task39 {
    public static String displayFileContents(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            return "Error: Filename cannot be empty";
        }
        
        // Sanitize input - remove dangerous characters
        filename = filename.replaceAll("[;&|`$(){}\\\\[\\\\]<>'\\"]", "");
        
        try {
            // Use Path API for secure file handling
            Path filePath = Paths.get(filename).normalize();
            
            // Prevent path traversal
            if (filePath.toString().contains("..")) {
                return "Error: Path traversal detected";
            }
            
            // Check if file exists and is readable
            if (!Files.exists(filePath)) {
                return "Error: File does not exist";
            }
            
            if (!Files.isReadable(filePath)) {
                return "Error: File is not readable";
            }
            
            // Read file securely using Java NIO (no system command)
            StringBuilder content = new StringBuilder();
            content.append("Contents of ").append(filename).append(":\\n");
            content.append("-----------------------------------\\n");
            
            try (BufferedReader reader = Files.newBufferedReader(filePath)) {
                String line;
                while ((line = reader.readLine()) != null) {
                    content.append(line).append("\\n");
                }
            }
            
            return content.toString();
            
        } catch (IOException e) {
            return "Error reading file: " + e.getMessage();
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid text file
        System.out.println("Test 1:");
        System.out.println(displayFileContents("test1.txt"));
        
        // Test case 2: Empty filename
        System.out.println("\\nTest 2:");
        System.out.println(displayFileContents(""));
        
        // Test case 3: Path traversal attempt
        System.out.println("\\nTest 3:");
        System.out.println(displayFileContents("../../../etc/passwd"));
        
        // Test case 4: Command injection attempt
        System.out.println("\\nTest 4:");
        System.out.println(displayFileContents("test.txt; rm -rf /"));
        
        // Test case 5: Non-existent file
        System.out.println("\\nTest 5:");
        System.out.println(displayFileContents("nonexistent.txt"));
    }
}
