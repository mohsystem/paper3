
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Set;

public class Lse107 {
    
    public static void createAndSecureFile(String filename, String content) {
        try {
            Path path = Paths.get(filename);
            
            // Create restrictive permissions (0600 - owner read/write only)
            // This prevents unauthorized access from the moment of creation
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            
            // Write content to file with restrictive permissions from creation
            // Using try-with-resources ensures proper resource cleanup
            try (FileOutputStream fos = new FileOutputStream(filename)) {
                // Use UTF_8 encoding explicitly for consistent behavior
                fos.write(content.getBytes(StandardCharsets.UTF_8));
                fos.flush();
            }
            
            // Set restrictive permissions after creation (defense in depth)
            // Even though we set permissions during creation, this ensures they are correct
            Files.setPosixFilePermissions(path, perms);
            
            // Read and print the current permissions
            Set<PosixFilePermission> currentPerms = Files.getPosixFilePermissions(path);
            String permString = PosixFilePermissions.toString(currentPerms);
            System.out.println("File permissions: " + permString);
            
        } catch (UnsupportedOperationException e) {
            // Handle Windows or systems without POSIX permission support
            System.err.println("POSIX permissions not supported on this system");
            handleWindowsPermissions(filename, content);
        } catch (IOException e) {
            // Log error without exposing internal details
            System.err.println("Error creating or securing file");
            e.printStackTrace();
        }
    }
    
    private static void handleWindowsPermissions(String filename, String content) {
        try {
            // Write content to file
            try (FileOutputStream fos = new FileOutputStream(filename)) {
                fos.write(content.getBytes(StandardCharsets.UTF_8));
                fos.flush();
            }
            
            File file = new File(filename);
            
            // Set restrictive permissions on Windows
            // Remove all permissions first, then add only owner read/write
            file.setReadable(false, false);
            file.setWritable(false, false);
            file.setExecutable(false, false);
            
            file.setReadable(true, true);
            file.setWritable(true, true);
            
            // Print Windows-style permissions
            System.out.println("File permissions (Windows): " +
                "readable=" + file.canRead() +
                ", writable=" + file.canWrite() +
                ", executable=" + file.canExecute());
                
        } catch (IOException e) {
            System.err.println("Error creating or securing file on Windows");
            e.printStackTrace();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Standard operation
        System.out.println("Test 1: Creating secret.txt");
        createAndSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        // Test case 2: Different filename
        System.out.println("\\nTest 2: Creating secret2.txt");
        createAndSecureFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        
        // Test case 3: Different content
        System.out.println("\\nTest 3: Creating secret3.txt with different content");
        createAndSecureFile("secret3.txt", "TestContent123");
        
        // Test case 4: Empty content
        System.out.println("\\nTest 4: Creating secret4.txt with empty content");
        createAndSecureFile("secret4.txt", "");
        
        // Test case 5: Special characters in content
        System.out.println("\\nTest 5: Creating secret5.txt with special characters");
        createAndSecureFile("secret5.txt", "Special!@#$%^&*()");
    }
}
