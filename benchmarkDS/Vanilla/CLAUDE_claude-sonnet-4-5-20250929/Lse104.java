
import java.io.*;
import java.nio.file.*;

public class Lse104 {
    public static void writeToFile(String filename, String content) {
        try {
            // Open file with read/write access, create if not exists
            File file = new File(filename);
            FileOutputStream fos = new FileOutputStream(file, false);
            fos.write(content.getBytes());
            fos.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        writeToFile("test1.txt", "important_config");
        
        // Test case 2
        writeToFile("test2.txt", "another important_config");
        
        // Test case 3
        writeToFile("test3.txt", "config data 123");
        
        // Test case 4
        writeToFile("test4.txt", "sensitive information");
        
        // Test case 5
        writeToFile("test5.txt", "final test config");
        
        System.out.println("All test cases completed.");
    }
}
