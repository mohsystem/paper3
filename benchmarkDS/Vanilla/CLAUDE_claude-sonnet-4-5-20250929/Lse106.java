
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
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
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                Files.setPosixFilePermissions(Paths.get(filename), perms);
            } catch (UnsupportedOperationException e) {
                // For Windows, use File.setReadOnly()
                java.io.File file = new java.io.File(filename);
                file.setReadOnly();
            }
            
            return filename;
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        String result1 = createSecretFile("Super secret password: 12345", "secret1.txt");
        System.out.println(result1);
        
        // Test case 2
        String result2 = createSecretFile("Top secret data: classified", "secret2.txt");
        System.out.println(result2);
        
        // Test case 3
        String result3 = createSecretFile("Hidden treasure location: X marks the spot", "secret3.txt");
        System.out.println(result3);
        
        // Test case 4
        String result4 = createSecretFile("API Key: abc123xyz789", "secret4.txt");
        System.out.println(result4);
        
        // Test case 5
        String result5 = createSecretFile("Secret formula: H2O + C6H12O6", "secret5.txt");
        System.out.println(result5);
    }
}
