
import java.io.*;
import java.nio.file.*;
import java.nio.charset.StandardCharsets;

public class Lse104 {
    public static void writeConfigToFile(String filename, String content) {
        if (filename == null || filename.trim().isEmpty()) {
            throw new IllegalArgumentException("Filename cannot be null or empty");
        }
        
        if (content == null) {
            content = "";
        }
        
        // Normalize path to prevent directory traversal
        Path path = Paths.get(filename).normalize();
        
        try {
            // Create file with restricted permissions (owner read/write only)
            Files.write(path, content.getBytes(StandardCharsets.UTF_8),
                StandardOpenOption.CREATE,
                StandardOpenOption.WRITE,
                StandardOpenOption.TRUNCATE_EXISTING);
            
            // Set file permissions to 600 (owner read/write only) on Unix-like systems
            try {
                Files.setPosixFilePermissions(path, 
                    java.nio.file.attribute.PosixFilePermissions.fromString("rw-------"));
            } catch (UnsupportedOperationException e) {
                // Windows doesn't support POSIX permissions, use alternate method
                File file = path.toFile();
                file.setReadable(false, false);
                file.setWritable(false, false);
                file.setExecutable(false, false);
                file.setReadable(true, true);
                file.setWritable(true, true);
            }
            
            System.out.println("Successfully wrote to file: " + filename);
        } catch (IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Normal write");
        writeConfigToFile("config1.txt", "important_config");
        
        System.out.println("\\nTest Case 2: Empty content");
        writeConfigToFile("config2.txt", "");
        
        System.out.println("\\nTest Case 3: Long content");
        writeConfigToFile("config3.txt", "important_config_with_more_data_12345");
        
        System.out.println("\\nTest Case 4: Special characters");
        writeConfigToFile("config4.txt", "important_config\\nline2\\ttab");
        
        System.out.println("\\nTest Case 5: Overwrite existing");
        writeConfigToFile("config1.txt", "important_config_updated");
    }
}
