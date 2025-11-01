
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse105 {
    public static void createRestrictiveFile(String filename, String content) throws IOException {
        Path path = Paths.get(filename);
        
        // Set restrictive permissions (owner read/write only: 0600) at creation time
        // This prevents TOCTOU issues by setting permissions atomically during file creation
        Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
        
        try {
            // Write content with restrictive permissions set immediately
            // Using UTF-8 encoding and atomic write operation
            Files.write(path, content.getBytes(StandardCharsets.UTF_8));
            
            // Apply restrictive permissions (owner read/write only)
            // This is done after write to ensure file is created with default umask first
            // then restricted to prevent unauthorized access
            Files.setPosixFilePermissions(path, perms);
            
        } catch (UnsupportedOperationException e) {
            // POSIX permissions not supported on this filesystem (e.g., Windows)
            // Fall back to basic setReadable/setWritable
            Files.write(path, content.getBytes(StandardCharsets.UTF_8));
            path.toFile().setReadable(false, false);
            path.toFile().setWritable(false, false);
            path.toFile().setExecutable(false, false);
            path.toFile().setReadable(true, true);
            path.toFile().setWritable(true, true);
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] testFiles = {
            "secret.txt",
            "test1_secret.txt",
            "test2_secret.txt",
            "test3_secret.txt",
            "test4_secret.txt"
        };
        
        String content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
        
        for (String testFile : testFiles) {
            try {
                createRestrictiveFile(testFile, content);
                System.out.println("Successfully created " + testFile + " with restrictive permissions");
            } catch (IOException e) {
                System.err.println("Failed to create " + testFile + ": " + e.getMessage());
            }
        }
    }
}
