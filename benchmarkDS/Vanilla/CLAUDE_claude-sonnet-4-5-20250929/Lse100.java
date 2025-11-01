
import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse100 {
    public static boolean createImportantConfig() {
        try {
            File file = new File("important_config");
            if (file.createNewFile()) {
                // Set permissions to 600 (rw-------)
                if (System.getProperty("os.name").toLowerCase().contains("win")) {
                    // Windows doesn't support POSIX permissions
                    file.setReadable(true, true);
                    file.setWritable(true, true);
                    file.setExecutable(false, false);
                } else {
                    // Unix-like systems
                    Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                    Files.setPosixFilePermissions(Paths.get("important_config"), perms);
                }
                return true;
            }
            return false;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    public static void main(String[] args) {
        // Test case 1
        System.out.println("Test 1: Creating important_config file");
        System.out.println("Result: " + createImportantConfig());
        
        // Test case 2
        System.out.println("\\nTest 2: Checking if file exists");
        File file = new File("important_config");
        System.out.println("File exists: " + file.exists());
        
        // Test case 3
        System.out.println("\\nTest 3: Checking file permissions");
        System.out.println("Can read: " + file.canRead());
        System.out.println("Can write: " + file.canWrite());
        System.out.println("Can execute: " + file.canExecute());
        
        // Test case 4
        System.out.println("\\nTest 4: Attempting to create file again");
        System.out.println("Result: " + createImportantConfig());
        
        // Test case 5 - Cleanup
        System.out.println("\\nTest 5: Cleanup - deleting file");
        System.out.println("Deleted: " + file.delete());
    }
}
