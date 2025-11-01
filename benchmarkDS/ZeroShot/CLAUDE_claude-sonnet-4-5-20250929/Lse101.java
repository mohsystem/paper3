
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse101 {
    public static void writeToFile(String filename, String content) {
        try {
            // Create file with restricted permissions (owner read/write only)
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            Files.createFile(Paths.get(filename), PosixFilePermissions.asFileAttribute(perms));
        } catch (IOException e) {
            // File might already exist, continue
        } catch (UnsupportedOperationException e) {
            // POSIX permissions not supported on this system
        }
        
        try (FileWriter writer = new FileWriter(filename)) {
            writer.write(content);
        } catch (IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Write to important_config
        writeToFile("important_config", "important_config");
        
        // Test case 2: Write different content
        writeToFile("test_config_1", "test_content_1");
        
        // Test case 3: Write empty string
        writeToFile("test_config_2", "");
        
        // Test case 4: Write multi-line content
        writeToFile("test_config_3", "line1\\nline2\\nline3");
        
        // Test case 5: Write special characters
        writeToFile("test_config_4", "special!@#$%^&*()");
        
        System.out.println("Files written successfully");
    }
}
