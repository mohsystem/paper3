
import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse104 {
    // Secure file write with proper permissions and error handling
    public static boolean writeConfigFile(String filename, String content) {
        if (filename == null || content == null) {
            return false;
        }
        
        // Normalize and validate path to prevent directory traversal
        Path basePath = Paths.get(System.getProperty("user.dir")).toAbsolutePath().normalize();
        Path filePath = basePath.resolve(filename).normalize();
        
        // Ensure resolved path is within base directory
        if (!filePath.startsWith(basePath)) {
            System.err.println("Error: Path traversal detected");
            return false;
        }
        
        try {
            // Set restrictive permissions (owner read/write only) - 0600
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            
            // Write to temp file first, then atomic rename
            Path tempFile = Files.createTempFile(basePath, ".tmp", ".config");
            
            try {
                // Set restrictive permissions on temp file
                try {
                    Files.setPosixFilePermissions(tempFile, perms);
                } catch (UnsupportedOperationException e) {
                    // Windows doesn't support POSIX permissions, use alternative
                    tempFile.toFile().setReadable(false, false);
                    tempFile.toFile().setWritable(false, false);
                    tempFile.toFile().setExecutable(false, false);
                    tempFile.toFile().setReadable(true, true);
                    tempFile.toFile().setWritable(true, true);
                }
                
                // Write content using try-with-resources for automatic resource management
                try (FileOutputStream fos = new FileOutputStream(tempFile.toFile())) {
                    byte[] bytes = content.getBytes(StandardCharsets.UTF_8);
                    fos.write(bytes);
                    fos.flush();
                    fos.getFD().sync(); // Ensure data is written to disk
                }
                
                // Atomic move to final location
                Files.move(tempFile, filePath, StandardOpenOption.ATOMIC_MOVE);
                
                return true;
            } catch (Exception e) {
                // Clean up temp file on error
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException ignored) {
                }
                throw e;
            }
        } catch (IOException e) {
            System.err.println("Error: File operation failed");
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal write
        System.out.println("Test 1: " + writeConfigFile("config1.txt", "important_config"));
        
        // Test case 2: Empty content
        System.out.println("Test 2: " + writeConfigFile("config2.txt", ""));
        
        // Test case 3: Multi-line content
        System.out.println("Test 3: " + writeConfigFile("config3.txt", "line1\\nline2\\nline3"));
        
        // Test case 4: Null filename (should fail)
        System.out.println("Test 4: " + writeConfigFile(null, "important_config"));
        
        // Test case 5: Path traversal attempt (should fail)
        System.out.println("Test 5: " + writeConfigFile("../../../etc/passwd", "important_config"));
    }
}
