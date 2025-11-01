
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse100 {
    public static boolean createSecureConfigFile(String filename) {
        try {
            Path path = Paths.get(filename);
            
            // Set permissions: owner read/write only (600)
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            
            // Create file with secure permissions
            if (!Files.exists(path)) {
                Files.createFile(path);
                Files.setPosixFilePermissions(path, perms);
                return true;
            }
            return false;
        } catch (UnsupportedOperationException e) {
            // For Windows systems that don't support POSIX permissions
            try {
                File file = new File(filename);
                if (!file.exists()) {
                    file.createNewFile();
                    file.setReadable(true, true);
                    file.setWritable(true, true);
                    file.setExecutable(false, false);
                    return true;
                }
                return false;
            } catch (IOException ex) {
                ex.printStackTrace();
                return false;
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Create important_config file
        System.out.println("Test 1 - Create important_config: " + 
            createSecureConfigFile("important_config"));
        
        // Test case 2: Try to create the same file again
        System.out.println("Test 2 - Create same file again: " + 
            createSecureConfigFile("important_config"));
        
        // Test case 3: Create config file with different name
        System.out.println("Test 3 - Create test_config: " + 
            createSecureConfigFile("test_config"));
        
        // Test case 4: Create config in subdirectory
        System.out.println("Test 4 - Create config in subdir: " + 
            createSecureConfigFile("configs/app_config"));
        
        // Test case 5: Create config with path
        System.out.println("Test 5 - Create secure_settings: " + 
            createSecureConfigFile("secure_settings"));
    }
}
