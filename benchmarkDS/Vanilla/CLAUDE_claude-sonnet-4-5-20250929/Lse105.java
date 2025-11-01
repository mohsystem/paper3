
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Lse105 {
    public static void createSecretFile(String filename, String content) {
        try {
            // Write content to file
            FileWriter writer = new FileWriter(filename);
            writer.write(content);
            writer.close();
            
            // Set restrictive permissions (owner read/write only)
            Set<PosixFilePermission> perms = new HashSet<>();
            perms.add(PosixFilePermission.OWNER_READ);
            perms.add(PosixFilePermission.OWNER_WRITE);
            
            try {
                Files.setPosixFilePermissions(Paths.get(filename), perms);
            } catch (UnsupportedOperationException e) {
                // Windows doesn't support POSIX permissions
                java.io.File file = new java.io.File(filename);
                file.setReadable(true, true);
                file.setWritable(true, true);
                file.setExecutable(false, false);
            }
            
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        createSecretFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        // Test case 2
        createSecretFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        // Test case 3
        createSecretFile("secret3.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        // Test case 4
        createSecretFile("secret4.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        // Test case 5
        createSecretFile("secret5.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        System.out.println("Files created with restrictive permissions");
    }
}
