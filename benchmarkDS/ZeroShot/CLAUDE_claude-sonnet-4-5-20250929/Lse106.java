
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Lse106 {
    public static String createSecretFile(String secretInfo, String filename) {
        try {
            // Write the secret info to file
            FileWriter writer = new FileWriter(filename);
            writer.write(secretInfo);
            writer.close();
            
            // Change permissions to read-only for owner
            Path path = Paths.get(filename);
            
            // For Unix-like systems
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                // For Windows, use File API
                java.io.File file = new java.io.File(filename);
                file.setReadOnly();
            } else {
                // For Unix/Linux/Mac
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                Files.setPosixFilePermissions(path, perms);
            }
            
            return filename;
        } catch (IOException e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        String filename1 = "secret1.txt";
        String result1 = createSecretFile("Super secret password: admin123", filename1);
        System.out.println(result1);
        
        // Test case 2
        String filename2 = "secret2.txt";
        String result2 = createSecretFile("API Key: abc123xyz789", filename2);
        System.out.println(result2);
        
        // Test case 3
        String filename3 = "secret3.txt";
        String result3 = createSecretFile("Database credentials: user=root, pass=secret", filename3);
        System.out.println(result3);
        
        // Test case 4
        String filename4 = "secret4.txt";
        String result4 = createSecretFile("Private key: RSA-2048-BEGIN", filename4);
        System.out.println(result4);
        
        // Test case 5
        String filename5 = "secret5.txt";
        String result5 = createSecretFile("Social Security Number: 123-45-6789", filename5);
        System.out.println(result5);
    }
}
