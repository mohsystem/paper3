
import java.io.*;
import java.nio.file.*;
import java.security.SecureRandom;

public class Task127 {
    private static final int BUFFER_SIZE = 8192;
    private static final String TEMP_PREFIX = "secure_temp_";
    
    public static String copyFileToTemp(String sourceFilePath) throws IOException {
        // Validate input
        if (sourceFilePath == null || sourceFilePath.trim().isEmpty()) {
            throw new IllegalArgumentException("Source file path cannot be null or empty");
        }
        
        // Normalize and validate path to prevent traversal attacks
        Path sourcePath = Paths.get(sourceFilePath).normalize().toAbsolutePath();
        
        // Check if file exists and is readable
        if (!Files.exists(sourcePath)) {
            throw new FileNotFoundException("Source file does not exist: " + sourcePath);
        }
        
        if (!Files.isRegularFile(sourcePath)) {
            throw new IllegalArgumentException("Source path is not a regular file: " + sourcePath);
        }
        
        if (!Files.isReadable(sourcePath)) {
            throw new IOException("Source file is not readable: " + sourcePath);
        }
        
        // Create secure temporary file
        Path tempFile = null;
        try {
            tempFile = Files.createTempFile(TEMP_PREFIX, ".tmp");
            
            // Copy file content securely
            try (InputStream inputStream = Files.newInputStream(sourcePath);
                 OutputStream outputStream = Files.newOutputStream(tempFile)) {
                
                byte[] buffer = new byte[BUFFER_SIZE];
                int bytesRead;
                
                while ((bytesRead = inputStream.read(buffer)) != -1) {
                    outputStream.write(buffer, 0, bytesRead);
                }
            }
            
            return tempFile.toString();
            
        } catch (IOException e) {
            // Clean up temporary file on error
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException cleanupEx) {
                    e.addSuppressed(cleanupEx);
                }
            }
            throw new IOException("Error copying file to temporary location: " + e.getMessage(), e);
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid file
        System.out.println("Test 1: Valid file");
        try {
            Path testFile1 = Files.createTempFile("test1_", ".txt");
            Files.write(testFile1, "Test content 1".getBytes());
            String result = copyFileToTemp(testFile1.toString());
            System.out.println("Success: Copied to " + result);
            Files.deleteIfExists(testFile1);
            Files.deleteIfExists(Paths.get(result));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
        
        // Test case 2: Non-existent file
        System.out.println("\\nTest 2: Non-existent file");
        try {
            copyFileToTemp("/non/existent/file.txt");
        } catch (Exception e) {
            System.out.println("Expected error: " + e.getMessage());
        }
        
        // Test case 3: Null input
        System.out.println("\\nTest 3: Null input");
        try {
            copyFileToTemp(null);
        } catch (Exception e) {
            System.out.println("Expected error: " + e.getMessage());
        }
        
        // Test case 4: Empty string
        System.out.println("\\nTest 4: Empty string");
        try {
            copyFileToTemp("");
        } catch (Exception e) {
            System.out.println("Expected error: " + e.getMessage());
        }
        
        // Test case 5: Large file
        System.out.println("\\nTest 5: Large file");
        try {
            Path testFile5 = Files.createTempFile("test5_", ".txt");
            byte[] largeContent = new byte[100000];
            new SecureRandom().nextBytes(largeContent);
            Files.write(testFile5, largeContent);
            String result = copyFileToTemp(testFile5.toString());
            System.out.println("Success: Large file copied to " + result);
            Files.deleteIfExists(testFile5);
            Files.deleteIfExists(Paths.get(result));
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
        }
    }
}
