
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;
import java.util.Set;

public class Lse103 {
    
    public static boolean createFileWithFlags(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            System.err.println("Invalid filename");
            return false;
        }
        
        Path path = Paths.get(filename).normalize();
        
        // Security: Validate path to prevent directory traversal
        if (path.toString().contains("..")) {
            System.err.println("Path traversal attempt detected");
            return false;
        }
        
        try {
            // O_CREAT | O_EXCL | O_WRONLY equivalent
            Files.newOutputStream(path, 
                StandardOpenOption.CREATE_NEW,
                StandardOpenOption.WRITE).close();
            
            // Set secure permissions (owner read/write only)
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(path, perms);
            } catch (UnsupportedOperationException e) {
                // Windows doesn't support POSIX permissions
            }
            
            System.out.println("File created successfully: " + filename);
            return true;
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
            return false;
        }
    }
    
    public static boolean truncateAndWriteFile(String filename) {
        if (filename == null || filename.trim().isEmpty()) {
            System.err.println("Invalid filename");
            return false;
        }
        
        Path path = Paths.get(filename).normalize();
        
        if (path.toString().contains("..")) {
            System.err.println("Path traversal attempt detected");
            return false;
        }
        
        try {
            // O_CREAT | O_TRUNC | O_WRONLY equivalent
            Files.newOutputStream(path,
                StandardOpenOption.CREATE,
                StandardOpenOption.TRUNCATE_EXISTING,
                StandardOpenOption.WRITE).close();
            
            System.out.println("File truncated successfully: " + filename);
            return true;
        } catch (IOException e) {
            System.err.println("Error: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Create new file with O_EXCL ===");
        createFileWithFlags("test1.txt");
        
        System.out.println("\\n=== Test Case 2: Try to create existing file (should fail) ===");
        createFileWithFlags("test1.txt");
        
        System.out.println("\\n=== Test Case 3: Truncate existing file ===");
        truncateAndWriteFile("test1.txt");
        
        System.out.println("\\n=== Test Case 4: Create file with invalid path ===");
        createFileWithFlags("../../../etc/passwd");
        
        System.out.println("\\n=== Test Case 5: Create file with valid name ===");
        createFileWithFlags("test2.txt");
        
        // Cleanup
        try {
            Files.deleteIfExists(Paths.get("test1.txt"));
            Files.deleteIfExists(Paths.get("test2.txt"));
        } catch (IOException e) {
            System.err.println("Cleanup error: " + e.getMessage());
        }
    }
}
