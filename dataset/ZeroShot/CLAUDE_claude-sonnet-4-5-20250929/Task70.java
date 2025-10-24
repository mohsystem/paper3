
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Task70 {
    public static String readFileContent(String filePath) {
        try {
            // Normalize and validate the path to prevent directory traversal attacks
            Path path = Paths.get(filePath).normalize().toAbsolutePath();
            
            // Check if file exists and is a regular file
            if (!Files.exists(path)) {
                return "Error: File does not exist";
            }
            
            if (!Files.isRegularFile(path)) {
                return "Error: Path is not a regular file";
            }
            
            // Check if file is readable
            if (!Files.isReadable(path)) {
                return "Error: File is not readable";
            }
            
            // Read and return file content
            return new String(Files.readAllBytes(path));
            
        } catch (IOException e) {
            return "Error: Unable to read file - " + e.getMessage();
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testFiles = {
            "test1.txt",
            "test2.txt",
            "nonexistent.txt",
            "../etc/passwd",
            "test3.txt"
        };
        
        // If command line argument is provided, use it
        if (args.length > 0) {
            String content = readFileContent(args[0]);
            System.out.println("File Content:\\n" + content);
        } else {
            // Run test cases
            System.out.println("Running test cases:\\n");
            for (int i = 0; i < testFiles.length; i++) {
                System.out.println("Test Case " + (i + 1) + ": " + testFiles[i]);
                String content = readFileContent(testFiles[i]);
                System.out.println(content);
                System.out.println("-------------------\\n");
            }
        }
    }
}
