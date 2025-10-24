
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.charset.StandardCharsets;

public class Task70 {
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
    public static String readFileContent(String filePath) {
        if (filePath == null || filePath.trim().isEmpty()) {
            return "Error: File path cannot be null or empty";
        }
        
        try {
            // Normalize and validate the path to prevent path traversal
            Path path = Paths.get(filePath).normalize().toAbsolutePath();
            
            // Check if file exists
            if (!Files.exists(path)) {
                return "Error: File does not exist: " + filePath;
            }
            
            // Check if it's a regular file (not a directory)
            if (!Files.isRegularFile(path)) {
                return "Error: Path is not a regular file: " + filePath;
            }
            
            // Check if file is readable
            if (!Files.isReadable(path)) {
                return "Error: File is not readable: " + filePath;
            }
            
            // Check file size to prevent memory exhaustion
            long fileSize = Files.size(path);
            if (fileSize > MAX_FILE_SIZE) {
                return "Error: File size exceeds maximum allowed size (10MB)";
            }
            
            // Read file content securely
            byte[] fileBytes = Files.readAllBytes(path);
            return new String(fileBytes, StandardCharsets.UTF_8);
            
        } catch (IOException e) {
            return "Error: Unable to read file - " + e.getMessage();
        } catch (SecurityException e) {
            return "Error: Security restriction - " + e.getMessage();
        } catch (Exception e) {
            return "Error: Unexpected error - " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        if (args.length == 0) {
            System.out.println("Usage: java Task70 <file_path>");
            System.out.println("\\nRunning test cases...\\n");
            
            // Test case 1: Valid file (create a test file)
            try {
                Path testFile1 = Files.createTempFile("test1", ".txt");
                Files.write(testFile1, "Hello, World!".getBytes());
                System.out.println("Test 1 - Valid file:");
                System.out.println(readFileContent(testFile1.toString()));
                Files.deleteIfExists(testFile1);
            } catch (IOException e) {
                System.out.println("Test 1 failed: " + e.getMessage());
            }
            
            // Test case 2: Non-existent file
            System.out.println("\\nTest 2 - Non-existent file:");
            System.out.println(readFileContent("/nonexistent/file.txt"));
            
            // Test case 3: Null path
            System.out.println("\\nTest 3 - Null path:");
            System.out.println(readFileContent(null));
            
            // Test case 4: Empty path
            System.out.println("\\nTest 4 - Empty path:");
            System.out.println(readFileContent(""));
            
            // Test case 5: File with special characters
            try {
                Path testFile5 = Files.createTempFile("test_special", ".txt");
                Files.write(testFile5, "Content with special chars: @#$%^&*()".getBytes());
                System.out.println("\\nTest 5 - File with special characters:");
                System.out.println(readFileContent(testFile5.toString()));
                Files.deleteIfExists(testFile5);
            } catch (IOException e) {
                System.out.println("Test 5 failed: " + e.getMessage());
            }
            
        } else {
            String filePath = args[0];
            String content = readFileContent(filePath);
            System.out.println(content);
        }
    }
}
