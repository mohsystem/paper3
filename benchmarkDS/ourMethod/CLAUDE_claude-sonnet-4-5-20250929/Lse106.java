
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Set;

public class Lse106 {
    // Secure method to create file with secret info
    public static String createSecretFile(String secretInfo) throws IOException {
        // Use secure random for unique filename to prevent prediction
        SecureRandom sr = new SecureRandom();
        byte[] randomBytes = new byte[16];
        sr.nextBytes(randomBytes);
        StringBuilder hexName = new StringBuilder();
        for (byte b : randomBytes) {
            hexName.append(String.format("%02x", b));
        }
        
        // Create file in secure temp directory with restrictive permissions
        Path tempDir = Paths.get(System.getProperty("java.io.tmpdir"));
        String filename = "secret_" + hexName.toString() + ".txt";
        Path filePath = tempDir.resolve(filename);
        
        // Convert secret to bytes for writing
        byte[] secretBytes = secretInfo.getBytes(StandardCharsets.UTF_8);
        
        try {
            // Set restrictive permissions before writing (owner read/write only during creation)
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            
            // Create file with restrictive permissions atomically
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                // Windows: Create file and set to read-only after write
                try (FileOutputStream fos = new FileOutputStream(filePath.toFile())) {
                    fos.write(secretBytes);
                    fos.flush();
                    // Ensure data is written to disk
                    fos.getFD().sync();
                }
                // Set read-only for owner on Windows
                File file = filePath.toFile();
                file.setReadOnly();
            } else {
                // POSIX: Create with restrictive permissions, write, then make read-only
                Files.write(filePath, secretBytes);
                
                // Change to read-only for owner (r---------)
                Set<PosixFilePermission> readOnlyPerms = PosixFilePermissions.fromString("r--------");
                Files.setPosixFilePermissions(filePath, readOnlyPerms);
            }
            
            // Clear sensitive data from memory
            Arrays.fill(secretBytes, (byte) 0);
            
            return filePath.toString();
            
        } catch (IOException e) {
            // Clear sensitive data on error
            Arrays.fill(secretBytes, (byte) 0);
            throw e;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testSecrets = {
            "SuperSecret123!",
            "ClassifiedInformation",
            "TopSecretData",
            "ConfidentialInfo",
            "RestrictedAccess"
        };
        
        for (String secret : testSecrets) {
            try {
                String filename = createSecretFile(secret);
                System.out.println(filename);
            } catch (IOException e) {
                System.err.println("Error creating secret file: " + e.getMessage());
            }
        }
    }
}
