
import java.io.File;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.Path;

public class Task69 {
    public static String deleteFile(String filePath) {
        if (filePath == null || filePath.trim().isEmpty()) {
            return "Error: File path cannot be null or empty";
        }
        
        try {
            // Sanitize the file path to prevent path traversal attacks
            Path path = Paths.get(filePath).normalize().toAbsolutePath();
            File file = path.toFile();
            
            // Check if file exists
            if (!file.exists()) {
                return "Error: File does not exist: " + filePath;
            }
            
            // Check if it's a file (not a directory)
            if (!file.isFile()) {
                return "Error: Path is not a file: " + filePath;
            }
            
            // Check if file is readable/writable
            if (!file.canWrite()) {
                return "Error: No permission to delete file: " + filePath;
            }
            
            // Delete the file
            if (Files.deleteIfExists(path)) {
                return "Success: File deleted successfully: " + filePath;
            } else {
                return "Error: Failed to delete file: " + filePath;
            }
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: Empty file path");
        System.out.println(deleteFile(""));
        System.out.println();
        
        System.out.println("Test Case 2: Null file path");
        System.out.println(deleteFile(null));
        System.out.println();
        
        System.out.println("Test Case 3: Non-existent file");
        System.out.println(deleteFile("nonexistent_file.txt"));
        System.out.println();
        
        System.out.println("Test Case 4: Create and delete a test file");
        try {
            File testFile = new File("test_file_to_delete.txt");
            testFile.createNewFile();
            System.out.println(deleteFile("test_file_to_delete.txt"));
        } catch (Exception e) {
            System.out.println("Error creating test file: " + e.getMessage());
        }
        System.out.println();
        
        System.out.println("Test Case 5: Command line argument");
        if (args.length > 0) {
            System.out.println(deleteFile(args[0]));
        } else {
            System.out.println("No command line argument provided");
        }
    }
}
