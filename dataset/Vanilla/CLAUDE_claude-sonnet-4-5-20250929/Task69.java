
import java.io.File;

public class Task69 {
    public static boolean deleteFile(String filePath) {
        if (filePath == null || filePath.isEmpty()) {
            System.out.println("Error: File path is empty or null");
            return false;
        }
        
        File file = new File(filePath);
        
        if (!file.exists()) {
            System.out.println("Error: File does not exist - " + filePath);
            return false;
        }
        
        if (file.isDirectory()) {
            System.out.println("Error: Path is a directory, not a file - " + filePath);
            return false;
        }
        
        if (file.delete()) {
            System.out.println("File deleted successfully: " + filePath);
            return true;
        } else {
            System.out.println("Error: Failed to delete file - " + filePath);
            return false;
        }
    }
    
    public static void main(String[] args) {
        if (args.length > 0) {
            for (String filePath : args) {
                deleteFile(filePath);
            }
        } else {
            // Test cases
            System.out.println("Running test cases...\\n");
            
            // Test case 1: Create and delete a test file
            try {
                File testFile1 = new File("test_file_1.txt");
                testFile1.createNewFile();
                System.out.println("Test 1:");
                deleteFile("test_file_1.txt");
            } catch (Exception e) {
                System.out.println("Test 1 error: " + e.getMessage());
            }
            
            System.out.println();
            
            // Test case 2: Try to delete non-existent file
            System.out.println("Test 2:");
            deleteFile("non_existent_file.txt");
            
            System.out.println();
            
            // Test case 3: Create and delete another test file
            try {
                File testFile3 = new File("test_file_3.dat");
                testFile3.createNewFile();
                System.out.println("Test 3:");
                deleteFile("test_file_3.dat");
            } catch (Exception e) {
                System.out.println("Test 3 error: " + e.getMessage());
            }
            
            System.out.println();
            
            // Test case 4: Try to delete with empty path
            System.out.println("Test 4:");
            deleteFile("");
            
            System.out.println();
            
            // Test case 5: Try to delete with null path
            System.out.println("Test 5:");
            deleteFile(null);
        }
    }
}
