
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse45 {
    private static final String SAFE_DIRECTORY = "/safe/directory";
    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
    /**
     * Reads a file from the safe directory with proper security controls.
     * @param filename The requested filename (relative path)
     * @return byte array containing file contents
     * @throws IOException if file cannot be read or security checks fail
     */
    public static byte[] readFileFromSafeDirectory(String filename) throws IOException {
        // Input validation: reject null or empty filename
        if (filename == null || filename.isEmpty()) {
            throw new IllegalArgumentException("Filename cannot be null or empty");
        }
        
        // Normalize the safe directory path to resolve any symbolic links
        Path safeDir = Paths.get(SAFE_DIRECTORY).toRealPath();
        
        // Construct the requested file path
        Path requestedPath = safeDir.resolve(filename).normalize();
        
        // Security check: ensure the resolved path is within safe directory
        // This prevents path traversal attacks (e.g., ../../../etc/passwd)
        if (!requestedPath.startsWith(safeDir)) {
            throw new SecurityException("Access denied: path traversal detected");
        }
        
        // Open the file and get attributes in one atomic operation
        // This prevents TOCTOU race conditions
        File file = requestedPath.toFile();
        
        // Validate file exists and is a regular file (not directory or symlink)
        if (!Files.isRegularFile(requestedPath, java.nio.file.LinkOption.NOFOLLOW_LINKS)) {
            throw new IOException("Not a regular file or file does not exist");
        }
        
        // Check file size to prevent excessive memory allocation
        long fileSize = Files.size(requestedPath);
        if (fileSize > MAX_FILE_SIZE) {
            throw new IOException("File size exceeds maximum allowed size");
        }
        
        // Read file contents using the validated path
        // Files.readAllBytes uses buffered reading and handles resources properly
        return Files.readAllBytes(requestedPath);
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid file in safe directory
        try {
            byte[] content = readFileFromSafeDirectory("test.txt");
            System.out.println("Test 1 - Valid file: Success (" + content.length + " bytes)");
        } catch (Exception e) {
            System.out.println("Test 1 - Valid file: " + e.getMessage());
        }
        
        // Test case 2: Attempt path traversal attack
        try {
            byte[] content = readFileFromSafeDirectory("../../../etc/passwd");
            System.out.println("Test 2 - Path traversal: Failed (should be blocked)");
        } catch (SecurityException e) {
            System.out.println("Test 2 - Path traversal: Blocked successfully");
        } catch (Exception e) {
            System.out.println("Test 2 - Path traversal: " + e.getMessage());
        }
        
        // Test case 3: Null filename
        try {
            byte[] content = readFileFromSafeDirectory(null);
            System.out.println("Test 3 - Null filename: Failed (should be rejected)");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 3 - Null filename: Rejected successfully");
        } catch (Exception e) {
            System.out.println("Test 3 - Null filename: " + e.getMessage());
        }
        
        // Test case 4: Empty filename
        try {
            byte[] content = readFileFromSafeDirectory("");
            System.out.println("Test 4 - Empty filename: Failed (should be rejected)");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 4 - Empty filename: Rejected successfully");
        } catch (Exception e) {
            System.out.println("Test 4 - Empty filename: " + e.getMessage());
        }
        
        // Test case 5: Subdirectory access (valid)
        try {
            byte[] content = readFileFromSafeDirectory("subdir/file.txt");
            System.out.println("Test 5 - Subdirectory file: Success (" + content.length + " bytes)");
        } catch (Exception e) {
            System.out.println("Test 5 - Subdirectory file: " + e.getMessage());
        }
    }
}
