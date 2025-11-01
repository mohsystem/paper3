
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse107 {
    public static void createSecureFile(String filename, String content) {
        try {
            File file = new File(filename);
            
            // Create file with content
            FileWriter writer = new FileWriter(file);
            writer.write(content);
            writer.close();
            
            // Set restrictive permissions (owner read/write only - 600)
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            Files.setPosixFilePermissions(Paths.get(filename), perms);
            
            // Print permissions
            Set<PosixFilePermission> currentPerms = Files.getPosixFilePermissions(Paths.get(filename));
            String permString = PosixFilePermissions.toString(currentPerms);
            System.out.println("File permissions: " + permString + " (" + filename + ")");
            
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
        } catch (UnsupportedOperationException e) {
            // For Windows systems
            try {
                File file = new File(filename);
                file.setReadable(true, true);
                file.setWritable(true, true);
                file.setExecutable(false, false);
                System.out.println("File permissions set to owner-only (Windows): " + filename);
            } catch (Exception ex) {
                System.err.println("Error setting permissions: " + ex.getMessage());
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Create secret.txt with encrypted data
        createSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        // Test case 2: Create another secure file
        createSecureFile("test1.txt", "SecureData123");
        
        // Test case 3: Create file with different content
        createSecureFile("test2.txt", "ConfidentialInfo");
        
        // Test case 4: Create file with empty content
        createSecureFile("test3.txt", "");
        
        // Test case 5: Create file with multi-line content
        createSecureFile("test4.txt", "Line1\\nLine2\\nLine3");
    }
}
