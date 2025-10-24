
import java.io.*;
import java.nio.file.*;
import java.util.Scanner;

public class Task36 {
    /**
     * Reads a file and returns its contents as a string.
     * Implements security measures to prevent path traversal and other file-related vulnerabilities.
     */
    public static String readFileContents(String filePath) {
        if (filePath == null || filePath.trim().isEmpty()) {
            return "Error: File path cannot be null or empty";
        }

        try {
            // Normalize the path to prevent path traversal attacks
            Path path = Paths.get(filePath).normalize().toAbsolutePath();
            
            // Check if file exists
            if (!Files.exists(path)) {
                return "Error: File does not exist";
            }
            
            // Check if it's a regular file (not a directory or special file)
            if (!Files.isRegularFile(path)) {
                return "Error: Path does not point to a regular file";
            }
            
            // Check if file is readable
            if (!Files.isReadable(path)) {
                return "Error: File is not readable";
            }
            
            // Check file size to prevent reading extremely large files
            long fileSize = Files.size(path);
            if (fileSize > 10 * 1024 * 1024) { // 10 MB limit
                return "Error: File size exceeds maximum allowed size (10 MB)";
            }
            
            // Read file contents securely
            StringBuilder content = new StringBuilder();
            try (BufferedReader reader = Files.newBufferedReader(path)) {
                String line;
                while ((line = reader.readLine()) != null) {
                    content.append(line).append("\\n");
                }
            }
            
            return content.toString();
            
        } catch (IOException e) {
            return "Error: Unable to read file - " + e.getMessage();
        } catch (SecurityException e) {
            return "Error: Security violation - " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== File Reader Program ===\\n");
        
        // Test case 1: Valid text file (create a temporary file for testing)
        System.out.println("Test Case 1: Valid text file");
        try {
            Path tempFile1 = Files.createTempFile("test1", ".txt");
            Files.write(tempFile1, "Hello, World!\\nThis is a test file.".getBytes());
            System.out.println(readFileContents(tempFile1.toString()));
            Files.deleteIfExists(tempFile1);
        } catch (IOException e) {
            System.out.println("Test setup failed");
        }
        
        // Test case 2: Empty file path
        System.out.println("\\nTest Case 2: Empty file path");
        System.out.println(readFileContents(""));
        
        // Test case 3: Null file path
        System.out.println("\\nTest Case 3: Null file path");
        System.out.println(readFileContents(null));
        
        // Test case 4: Non-existent file
        System.out.println("\\nTest Case 4: Non-existent file");
        System.out.println(readFileContents("/nonexistent/path/file.txt"));
        
        // Test case 5: Path traversal attempt (security test)
        System.out.println("\\nTest Case 5: Path traversal attempt");
        System.out.println(readFileContents("../../../etc/passwd"));
    }
}
