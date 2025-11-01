
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse100 {
    public static boolean createSecureConfigFile(String filename) {
        try {
            File file = new File(filename);
            
            // Create file with restrictive permissions
            if (file.createNewFile()) {
                // Set permissions to 0600 (read/write for owner only)
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                Files.setPosixFilePermissions(Paths.get(filename), perms);
                return true;
            }
            return false;
        } catch (IOException e) {
            System.err.println("Error creating file: " + e.getMessage());
            return false;
        } catch (UnsupportedOperationException e) {
            // Fallback for Windows systems
            try {
                File file = new File(filename);
                if (file.createNewFile()) {
                    file.setReadable(true, true);
                    file.setWritable(true, true);
                    file.setExecutable(false, false);
                    return true;
                }
            } catch (IOException ex) {
                System.err.println("Error creating file: " + ex.getMessage());
            }
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Create file with default name
        System.out.println("Test 1: " + createSecureConfigFile("important_config"));
        
        // Test case 2: Create file with different name
        System.out.println("Test 2: " + createSecureConfigFile("important_config_2"));
        
        // Test case 3: Try to create existing file
        System.out.println("Test 3: " + createSecureConfigFile("important_config"));
        
        // Test case 4: Create file in current directory
        System.out.println("Test 4: " + createSecureConfigFile("./config_test"));
        
        // Test case 5: Create file with specific path
        System.out.println("Test 5: " + createSecureConfigFile("important_config_final"));
    }
}
