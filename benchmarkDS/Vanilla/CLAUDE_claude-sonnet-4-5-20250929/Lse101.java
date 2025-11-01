
import java.io.FileWriter;
import java.io.IOException;

public class Lse101 {
    public static void writeToFile(String filename, String content) {
        try (FileWriter writer = new FileWriter(filename)) {
            writer.write(content);
        } catch (IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        writeToFile("important_config", "important_config");
        
        // Test case 2
        writeToFile("test_file1.txt", "test content 1");
        
        // Test case 3
        writeToFile("test_file2.txt", "test content 2");
        
        // Test case 4
        writeToFile("config.txt", "configuration data");
        
        // Test case 5
        writeToFile("data.txt", "sample data");
        
        System.out.println("Files written successfully!");
    }
}
