
import java.io.*;
import java.nio.file.*;

public class Task70 {
    public static String readFileContent(String filePath) {
        try {
            return new String(Files.readAllBytes(Paths.get(filePath)));
        } catch (IOException e) {
            return "Error reading file: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testFiles = {
            "test1.txt",
            "test2.txt",
            "test3.txt",
            "nonexistent.txt",
            "test5.txt"
        };
        
        // Create test files
        try {
            Files.write(Paths.get("test1.txt"), "Hello, World!".getBytes());
            Files.write(Paths.get("test2.txt"), "Java Programming".getBytes());
            Files.write(Paths.get("test3.txt"), "File I/O Operations".getBytes());
            Files.write(Paths.get("test5.txt"), "Test Case 5".getBytes());
        } catch (IOException e) {
            System.out.println("Error creating test files");
        }
        
        // If command line argument provided, use it
        if (args.length > 0) {
            String content = readFileContent(args[0]);
            System.out.println("File Content:\\n" + content);
        } else {
            // Run test cases
            System.out.println("Running test cases:\\n");
            for (int i = 0; i < testFiles.length; i++) {
                System.out.println("Test Case " + (i + 1) + ": " + testFiles[i]);
                String content = readFileContent(testFiles[i]);
                System.out.println("Content: " + content);
                System.out.println("---");
            }
        }
    }
}
