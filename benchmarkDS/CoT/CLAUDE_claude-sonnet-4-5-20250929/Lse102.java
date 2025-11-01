
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse102 {
    public static boolean writeImportantConfig(String content) {
        String filename = "important_config";
        
        try {
            // Set permissions: owner read/write, group read, others read (0644)
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-r--r--");
            
            // Write content to file
            try (FileOutputStream fos = new FileOutputStream(filename)) {
                fos.write(content.getBytes());
            }
            
            // Set file permissions (Unix-like systems only)
            try {
                Files.setPosixFilePermissions(Paths.get(filename), perms);
            } catch (UnsupportedOperationException e) {
                // Windows doesn't support POSIX permissions
                System.out.println("POSIX permissions not supported on this system");
            }
            
            return true;
        } catch (IOException e) {
            System.err.println("Error writing to file: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + writeImportantConfig("important_config"));
        System.out.println("Test 2: " + writeImportantConfig("test_data_1"));
        System.out.println("Test 3: " + writeImportantConfig("configuration_value"));
        System.out.println("Test 4: " + writeImportantConfig("secure_settings"));
        System.out.println("Test 5: " + writeImportantConfig(""));
    }
}
