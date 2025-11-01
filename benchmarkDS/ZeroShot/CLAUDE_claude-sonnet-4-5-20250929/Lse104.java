
import java.io.*;
import java.nio.file.*;

public class Lse104 {
    public static void writeToFile(String filename, String content) {
        try {
            // Open file with read/write access, create if doesn't exist\n            // Using Java's FileOutputStream with appropriate flags
            RandomAccessFile file = new RandomAccessFile(filename, "rw");
            
            // Write the content to the file
            file.write(content.getBytes());
            
            // Close the file
            file.close();
            
            System.out.println("Successfully wrote to file: " + filename);
        } catch (IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        writeToFile("config1.txt", "important_config");
        
        // Test case 2
        writeToFile("config2.txt", "database_settings=localhost:5432");
        
        // Test case 3
        writeToFile("config3.txt", "api_key=secret123");
        
        // Test case 4
        writeToFile("config4.txt", "timeout=30");
        
        // Test case 5
        writeToFile("config5.txt", "debug_mode=true");
    }
}
