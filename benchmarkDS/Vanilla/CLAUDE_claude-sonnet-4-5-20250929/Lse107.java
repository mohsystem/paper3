
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse107 {
    public static void createAndSecureFile(String filename, String content) {
        try {
            // Create file and write content
            FileWriter writer = new FileWriter(filename);
            writer.write(content);
            writer.close();
            
            // Change permissions to restrictive (owner read/write only)
            Path path = Paths.get(filename);
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            Files.setPosixFilePermissions(path, perms);
            
            // Print permissions
            Set<PosixFilePermission> currentPerms = Files.getPosixFilePermissions(path);
            String permString = PosixFilePermissions.toString(currentPerms);
            System.out.println("File permissions: " + permString);
            
        } catch (IOException e) {
            System.out.println("Error: " + e.getMessage());
        } catch (UnsupportedOperationException e) {
            System.out.println("POSIX permissions not supported on this system");
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        createAndSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        System.out.println("\\nTest Case 2:");
        createAndSecureFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        System.out.println("\\nTest Case 3:");
        createAndSecureFile("secret3.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        System.out.println("\\nTest Case 4:");
        createAndSecureFile("secret4.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        System.out.println("\\nTest Case 5:");
        createAndSecureFile("secret5.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    }
}
