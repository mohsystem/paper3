
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse106 {
    public static String createSecureFile(String secretInfo, String filename) {
        try {
            File file = new File(filename);
            
            // Create file with restricted permissions (owner read-only)
            if (System.getProperty("os.name").toLowerCase().contains("win")) {
                // Windows: set read-only
                file.createNewFile();
                file.setReadable(true, true);
                file.setWritable(true, true);
            } else {
                // Unix-like: set 400 permissions
                Set<PosixFilePermission> perms = PosixFilePermissions.fromString("r--------");
                Files.createFile(Paths.get(filename), PosixFilePermissions.asFileAttribute(perms));
            }
            
            // Temporarily make writable to write content
            file.setWritable(true, true);
            
            // Write secret info
            try (FileWriter writer = new FileWriter(file)) {
                writer.write(secretInfo);
            }
            
            // Set back to read-only
            file.setReadOnly();
            
            return filename;
        } catch (IOException e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testCases = {
            "SuperSecret123!",
            "Confidential Data",
            "API_KEY=abc123xyz",
            "Password: P@ssw0rd!",
            "Token: eyJhbGciOiJIUzI1NiJ9"
        };
        
        for (int i = 0; i < testCases.length; i++) {
            String filename = "secret_file_" + (i + 1) + ".txt";
            String result = createSecureFile(testCases[i], filename);
            System.out.println(result);
        }
    }
}
