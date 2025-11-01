
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
            // Set permissions: owner read/write, group read, others read
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-r--r--");
            
            // Create and write to file
            FileOutputStream fos = new FileOutputStream(filename);
            fos.write(content.getBytes());
            fos.close();
            
            // Set file permissions (Unix-like systems only)
            try {
                Files.setPosixFilePermissions(Paths.get(filename), perms);
            } catch (UnsupportedOperationException e) {
                // Windows doesn't support POSIX permissions
                System.out.println("POSIX permissions not supported on this system");
            }
        } catch (IOException e) {
            e.printStackTrace();
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
        writeImportantConfig("test_config3.txt", "Configuration data 3");
        
        // Test case 5
        writeImportantConfig("test_config4.txt", "Configuration data 4");
        
        System.out.println("Files created successfully");
    }
}
