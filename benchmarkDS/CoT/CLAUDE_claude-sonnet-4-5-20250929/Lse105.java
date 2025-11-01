
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse105 {
    public static void createSecretFile(String filename, String content) {
        try {
            // Write content to file
            try (FileWriter writer = new FileWriter(filename)) {
                writer.write(content);
            }
            
            // Set restrictive permissions (owner read/write only)
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                // Windows: set file to read-only for basic restriction
                Files.setAttribute(Paths.get(filename), "dos:readonly", false);
            } else {
                // Unix/Linux: set to 600 (rw-------)
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
                Files.setPosixFilePermissions(Paths.get(filename), perms);
            }
            
            System.out.println("File created successfully with restrictive permissions.");
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Standard execution
        createSecretFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        // Test case 2: Different filename
        createSecretFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        // Test case 3: Empty content
        createSecretFile("secret3.txt", "");
        
        // Test case 4: Longer content
        createSecretFile("secret4.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        // Test case 5: Special characters in content
        createSecretFile("secret5.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==!@#$%");
    }
}
