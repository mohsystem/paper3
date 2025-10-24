
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Task69 {
    
    public static boolean deleteFile(String filePath) {
        if (filePath == null || filePath.trim().isEmpty()) {
            System.out.println("Error: File path cannot be null or empty");
            return false;
        }
        
        try {
            Path path = Paths.get(filePath).normalize().toAbsolutePath();
            File file = path.toFile();
            
            if (!file.exists()) {
                System.out.println("Error: File does not exist: " + filePath);
                return false;
            }
            
            if (!file.isFile()) {
                System.out.println("Error: Path is not a file: " + filePath);
                return false;
            }
            
            if (!file.canWrite()) {
                System.out.println("Error: No permission to delete file: " + filePath);
                return false;
            }
            
            boolean deleted = Files.deleteIfExists(path);
            
            if (deleted) {
                System.out.println("File deleted successfully: " + filePath);
                return true;
            } else {
                System.out.println("Error: Failed to delete file: " + filePath);
                return false;
            }
            
        } catch (IOException e) {
            System.out.println("Error: IOException occurred while deleting file");
            return false;
        } catch (SecurityException e) {
            System.out.println("Error: Security exception - insufficient permissions");
            return false;
        } catch (Exception e) {
            System.out.println("Error: Unexpected error occurred");
            return false;
        }
    }
    
    public static void main(String[] args) {
        if (args.length == 0) {
            System.out.println("Usage: java Task69 <file_path>");
            
            System.out.println("\\n=== Running Test Cases ===\\n");
            
            try {
                File testFile1 = File.createTempFile("test1_", ".txt");
                System.out.println("Test 1: Delete existing file");
                deleteFile(testFile1.getAbsolutePath());
                
                System.out.println("\\nTest 2: Delete non-existent file");
                deleteFile("non_existent_file.txt");
                
                System.out.println("\\nTest 3: Delete with null path");
                deleteFile(null);
                
                System.out.println("\\nTest 4: Delete with empty path");
                deleteFile("");
                
                File testFile5 = File.createTempFile("test5_", ".txt");
                String validPath = testFile5.getAbsolutePath();
                System.out.println("\\nTest 5: Delete valid file with normalized path");
                deleteFile(validPath);
                
            } catch (IOException e) {
                System.out.println("Error creating test files");
            }
            
            return;
        }
        
        deleteFile(args[0]);
    }
}
