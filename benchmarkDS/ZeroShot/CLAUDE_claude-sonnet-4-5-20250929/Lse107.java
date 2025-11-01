
import java.io.*;
import java.nio.file.*;
import java.nio.file.attribute.*;
import java.util.Set;

public class Lse107 {
    public static void createAndSecureFile(String filename, String content) {
        try {
            // Create file and write content
            Path filePath = Paths.get(filename);
            Files.write(filePath, content.getBytes());
            
            // Change permissions to restrictive (owner read/write only)
            Set<PosixFilePermission> perms = PosixFilePermissions.fromString("rw-------");
            Files.setPosixFilePermissions(filePath, perms);
            
            // Print permissions
            Set<PosixFilePermission> currentPerms = Files.getPosixFilePermissions(filePath);
            System.out.println("Permissions for " + filename + ": " + PosixFilePermissions.toString(currentPerms));
            
        } catch (UnsupportedOperationException e) {
            // For Windows or systems that don't support POSIX permissions
            try {
                File file = new File(filename);
                Files.write(file.toPath(), content.getBytes());
                
                // Set restrictive permissions on Windows
                file.setReadable(false, false);
                file.setWritable(false, false);
                file.setExecutable(false, false);
                file.setReadable(true, true);
                file.setWritable(true, true);
                
                System.out.println("Permissions for " + filename + ":");
                System.out.println("Readable: " + file.canRead());
                System.out.println("Writable: " + file.canWrite());
                System.out.println("Executable: " + file.canExecute());
            } catch (IOException ex) {
                System.err.println("Error: " + ex.getMessage());
            }
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Create secret.txt with encrypted content
        System.out.println("Test Case 1:");
        createAndSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
        System.out.println();
        
        // Test case 2: Create another secure file
        System.out.println("Test Case 2:");
        createAndSecureFile("test1.txt", "Confidential Data");
        System.out.println();
        
        // Test case 3: Create file with different content
        System.out.println("Test Case 3:");
        createAndSecureFile("test2.txt", "Sensitive Information");
        System.out.println();
        
        // Test case 4: Create file with empty content
        System.out.println("Test Case 4:");
        createAndSecureFile("test3.txt", "");
        System.out.println();
        
        // Test case 5: Create file with special characters
        System.out.println("Test Case 5:");
        createAndSecureFile("test4.txt", "Special@#$%^&*()");
        System.out.println();
    }
}
