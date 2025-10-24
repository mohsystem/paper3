
import java.io.*;
import java.nio.file.*;
import java.util.Scanner;

public class Task36 {
    public static String readFileContents(String filePath) {
        StringBuilder content = new StringBuilder();
        try {
            // Validate file path to prevent path traversal attacks
            Path path = Paths.get(filePath).normalize();
            File file = path.toFile();
            
            // Security checks
            if (!file.exists()) {
                return "Error: File does not exist";
            }
            if (!file.isFile()) {
                return "Error: Path is not a file";
            }
            if (!file.canRead()) {
                return "Error: File is not readable";
            }
            
            // Read file contents
            try (BufferedReader reader = new BufferedReader(new FileReader(file))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    content.append(line).append("\\n");
                }
            }
        } catch (IOException e) {
            return "Error reading file: " + e.getMessage();
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
        return content.toString();
    }
    
    public static void main(String[] args) {
        // Create test files
        try {
            Files.write(Paths.get("test1.txt"), "Hello World!\\nThis is test file 1.".getBytes());
            Files.write(Paths.get("test2.txt"), "Line 1\\nLine 2\\nLine 3".getBytes());
            Files.write(Paths.get("test3.txt"), "Single line file".getBytes());
            Files.write(Paths.get("test4.txt"), "".getBytes());
            Files.write(Paths.get("test5.txt"), "Test file 5\\nWith multiple\\nLines of text".getBytes());
        } catch (IOException e) {
            System.out.println("Error creating test files");
        }
        
        // Test cases
        System.out.println("Test Case 1:");
        System.out.println(readFileContents("test1.txt"));
        
        System.out.println("Test Case 2:");
        System.out.println(readFileContents("test2.txt"));
        
        System.out.println("Test Case 3:");
        System.out.println(readFileContents("test3.txt"));
        
        System.out.println("Test Case 4 (empty file):");
        System.out.println(readFileContents("test4.txt"));
        
        System.out.println("Test Case 5:");
        System.out.println(readFileContents("test5.txt"));
        
        // Cleanup
        try {
            Files.deleteIfExists(Paths.get("test1.txt"));
            Files.deleteIfExists(Paths.get("test2.txt"));
            Files.deleteIfExists(Paths.get("test3.txt"));
            Files.deleteIfExists(Paths.get("test4.txt"));
            Files.deleteIfExists(Paths.get("test5.txt"));
        } catch (IOException e) {
            System.out.println("Error cleaning up test files");
        }
    }
}
