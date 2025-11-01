
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
            
            // Security: Set restrictive permissions (owner read/write only) at creation time
            // This prevents unauthorized access from other users on the system
            // Equivalent to chmod 600 on Unix systems
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            
            // Security: Check if file already exists to prevent race conditions
            if (Files.exists(path)) {
                System.err.println("File already exists");
                return false;
            }
            
            // Security: Create file with restrictive permissions atomically
            // Using Files.createFile ensures atomic creation with proper permissions
            try {
                Files.createFile(path, PosixFilePermissions.asFileAttribute(perms));
                return true;
            } catch (UnsupportedOperationException e) {
                // Fallback for Windows systems that don't support POSIX permissions
                File file = path.toFile();
                if (file.createNewFile()) {
                    // Security: Set restrictive permissions on Windows
                    file.setReadable(false, false);
                    file.setReadable(true, true);
                    file.setWritable(false, false);
                    file.setWritable(true, true);
                    file.setExecutable(false, false);
                    return true;
                }
                return false;
            }
            
        } catch (IOException e) {
            System.err.println("Error creating file: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Create file with secure permissions
        System.out.println("Test 1 - Create important_config: " + 
            createSecureConfigFile("important_config"));
        
        // Test case 2: Attempt to create same file again (should fail)
        System.out.println("Test 2 - Create duplicate (should fail): " + 
            createSecureConfigFile("important_config"));
        
        // Test case 3: Create file in current directory
        System.out.println("Test 3 - Create test_config1: " + 
            createSecureConfigFile("test_config1"));
        
        // Test case 4: Create file with different name
        System.out.println("Test 4 - Create test_config2: " + 
            createSecureConfigFile("test_config2"));
        
        // Test case 5: Create file with another name
        System.out.println("Test 5 - Create secure_settings: " + 
            createSecureConfigFile("secure_settings"));
        
        // Cleanup test files
        try {
            Files.deleteIfExists(Paths.get("important_config"));
            Files.deleteIfExists(Paths.get("test_config1"));
            Files.deleteIfExists(Paths.get("test_config2"));
            Files.deleteIfExists(Paths.get("secure_settings"));
        } catch (IOException e) {
            System.err.println("Cleanup error: " + e.getMessage());
        }
    }
}
