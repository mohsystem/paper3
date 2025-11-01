
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse102 {
    public static void writeImportantConfig(String filename, String content) {
        try {
            // Set file permissions: owner read/write, group read, others read
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-r--r--");
            
            // Create file with permissions if it doesn't exist\n            if (!Files.exists(Paths.get(filename))) {\n                Files.createFile(Paths.get(filename));\n                Files.setPosixFilePermissions(Paths.get(filename), perms);\n            }\n            \n            // Write content to file\n            try (FileOutputStream fos = new FileOutputStream(filename)) {\n                fos.write(content.getBytes());\n            }\n        } catch (IOException e) {\n            System.err.println("Error: " + e.getMessage());\n        } catch (UnsupportedOperationException e) {\n            // For Windows or systems that don't support POSIX permissions
            try (FileOutputStream fos = new FileOutputStream(filename)) {
                fos.write(content.getBytes());
            } catch (IOException ex) {
                System.err.println("Error: " + ex.getMessage());
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        writeImportantConfig("important_config", "important_config");
        
        // Test case 2
        writeImportantConfig("test_config1.txt", "Configuration data 1");
        
        // Test case 3
        writeImportantConfig("test_config2.txt", "Configuration data 2");
        
        // Test case 4
        writeImportantConfig("test_config3.txt", "Secure configuration settings");
        
        // Test case 5
        writeImportantConfig("test_config4.txt", "Application configuration");
    }
}
