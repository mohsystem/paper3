
import java.io.*;
import java.nio.file.*;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Lse63 {
    private static final int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    private static final String BASE_DIR = ".";
    
    /**
     * Reads file content and calculates checksum.
     * @param inputPath Path to input file relative to BASE_DIR
     * @param outputPath Path to output file relative to BASE_DIR
     * @return true if operation successful, false otherwise
     */
    public static boolean processFile(String inputPath, String outputPath) {
        // Validate and normalize paths to prevent path traversal
        Path basePath;
        Path normalizedInputPath;
        Path normalizedOutputPath;
        
        try {
            basePath = Paths.get(BASE_DIR).toRealPath();
            normalizedInputPath = basePath.resolve(inputPath).normalize();
            normalizedOutputPath = basePath.resolve(outputPath).normalize();
            
            // Ensure paths stay within base directory
            if (!normalizedInputPath.startsWith(basePath) || 
                !normalizedOutputPath.startsWith(basePath)) {
                System.err.println("Error: Path traversal attempt detected");
                return false;
            }
        } catch (IOException e) {
            System.err.println("Error: Invalid path");
            return false;
        }
        
        byte[] buffer = null;
        long bytesRead = 0;
        long checksum = 0;
        
        // Open input file with restricted options - no symlink following
        try (InputStream inputStream = Files.newInputStream(normalizedInputPath, 
                StandardOpenOption.READ, LinkOption.NOFOLLOW_LINKS)) {
            
            // Validate file is regular file and within size limit
            BasicFileAttributes attrs = Files.readAttributes(normalizedInputPath, 
                BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            
            if (!attrs.isRegularFile()) {
                System.err.println("Error: Not a regular file");
                return false;
            }
            
            long fileSize = attrs.size();
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: File exceeds maximum size");
                return false;
            }
            
            // Safe allocation with checked size
            buffer = new byte[(int) fileSize];
            
            // Read file content with bounds checking
            int totalRead = 0;
            int remaining = buffer.length;
            while (remaining > 0) {
                int read = inputStream.read(buffer, totalRead, remaining);
                if (read == -1) break;
                totalRead += read;
                remaining -= read;
            }
            bytesRead = totalRead;
            
            // Calculate simple checksum (sum of all bytes mod 256)
            for (int i = 0; i < bytesRead; i++) {
                checksum = (checksum + (buffer[i] & 0xFF)) & 0xFF;
            }
            
        } catch (IOException e) {
            System.err.println("Error: Failed to read input file");
            return false;
        }
        
        // Write result to output file using atomic operation
        String result = String.format("Bytes read: %d%nChecksum: %d%nBuffer empty: %s%n",
            bytesRead, checksum, (checksum == 0 ? "true" : "false"));
        
        try {
            // Create temp file in same directory for atomic move
            Path tempFile = Files.createTempFile(normalizedOutputPath.getParent(), 
                ".tmp", ".txt");
            
            // Set restrictive permissions (owner read/write only)
            try {
                Files.setPosixFilePermissions(tempFile, 
                    java.util.Set.of(
                        java.nio.file.attribute.PosixFilePermission.OWNER_READ,
                        java.nio.file.attribute.PosixFilePermission.OWNER_WRITE
                    ));
            } catch (UnsupportedOperationException e) {
                // POSIX permissions not supported on this platform
            }
            
            // Write to temp file
            try (BufferedWriter writer = Files.newBufferedWriter(tempFile, 
                    StandardCharsets.UTF_8, StandardOpenOption.WRITE)) {
                writer.write(result);
                writer.flush();
            }
            
            // Atomic move to final destination
            Files.move(tempFile, normalizedOutputPath, 
                StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            
        } catch (IOException e) {
            System.err.println("Error: Failed to write output file");
            return false;
        } finally {
            // Clear sensitive buffer data
            if (buffer != null) {
                java.util.Arrays.fill(buffer, (byte) 0);
            }
        }
        
        return true;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal file
        try {
            Files.write(Paths.get("test1.txt"), "Hello World".getBytes(StandardCharsets.UTF_8));
            System.out.println("Test 1: " + (processFile("test1.txt", "output1.txt") ? "PASS" : "FAIL"));
        } catch (IOException e) {
            System.out.println("Test 1: FAIL - Setup error");
        }
        
        // Test case 2: Empty file
        try {
            Files.write(Paths.get("test2.txt"), new byte[0]);
            System.out.println("Test 2: " + (processFile("test2.txt", "output2.txt") ? "PASS" : "FAIL"));
        } catch (IOException e) {
            System.out.println("Test 2: FAIL - Setup error");
        }
        
        // Test case 3: File with zero checksum
        try {
            Files.write(Paths.get("test3.txt"), new byte[]{0, 0, 0});
            System.out.println("Test 3: " + (processFile("test3.txt", "output3.txt") ? "PASS" : "FAIL"));
        } catch (IOException e) {
            System.out.println("Test 3: FAIL - Setup error");
        }
        
        // Test case 4: Large content
        try {
            byte[] largeContent = new byte[1000];
            for (int i = 0; i < largeContent.length; i++) {
                largeContent[i] = (byte) (i % 256);
            }
            Files.write(Paths.get("test4.txt"), largeContent);
            System.out.println("Test 4: " + (processFile("test4.txt", "output4.txt") ? "PASS" : "FAIL"));
        } catch (IOException e) {
            System.out.println("Test 4: FAIL - Setup error");
        }
        
        // Test case 5: Path traversal attempt (should fail)
        System.out.println("Test 5: " + (!processFile("../test5.txt", "output5.txt") ? "PASS" : "FAIL"));
    }
}
