
import java.io.*;
import java.nio.file.*;

public class Lse103 {
    public static int createFile(String filename) {
        try {
            // O_CREAT | O_TRUNC | O_WRONLY | O_EXCL behavior
            // Creates file, fails if exists, write-only mode
            File file = new File(filename);
            if (file.exists()) {
                return -1; // O_EXCL: fail if file exists
            }
            
            FileOutputStream fos = new FileOutputStream(file);
            fos.close();
            return 0; // Success
        } catch (IOException e) {
            return -1; // Error
        }
    }
    
    public static int createFileWithTrunc(String filename) {
        try {
            // O_CREAT | O_TRUNC | O_WRONLY (without O_EXCL)
            // Creates or truncates file for writing
            FileOutputStream fos = new FileOutputStream(filename, false);
            fos.close();
            return 0; // Success
        } catch (IOException e) {
            return -1; // Error
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Create new file with O_EXCL
        System.out.println("Test 1 - Create new file: " + createFile("test1.txt"));
        
        // Test case 2: Try to create existing file with O_EXCL (should fail)
        System.out.println("Test 2 - Create existing file with O_EXCL: " + createFile("test1.txt"));
        
        // Test case 3: Create/truncate file without O_EXCL
        System.out.println("Test 3 - Create/truncate file: " + createFileWithTrunc("test2.txt"));
        
        // Test case 4: Truncate existing file
        System.out.println("Test 4 - Truncate existing file: " + createFileWithTrunc("test2.txt"));
        
        // Test case 5: Create file in valid directory
        System.out.println("Test 5 - Create another file: " + createFile("test3.txt"));
        
        // Cleanup
        new File("test1.txt").delete();
        new File("test2.txt").delete();
        new File("test3.txt").delete();
    }
}
