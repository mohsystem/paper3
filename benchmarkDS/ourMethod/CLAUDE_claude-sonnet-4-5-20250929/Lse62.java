
import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HexFormat;

public class Lse62 {
    private static final int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    private static final int BUFFER_SIZE = 8192;
    
    // Secure file reading with proper validation and bounds checking
    public static FileResult processFile(String baseDir, String inputRelPath, String outputRelPath) {
        // Validate base directory exists and is a directory
        File baseDirFile = new File(baseDir);
        if (!baseDirFile.exists() || !baseDirFile.isDirectory()) {
            throw new IllegalArgumentException("Invalid base directory");
        }
        
        try {
            // Normalize and validate input path - prevent path traversal
            Path basePath = baseDirFile.toPath().toRealPath();
            Path inputPath = basePath.resolve(inputRelPath).normalize();
            
            // Ensure resolved path is within base directory
            if (!inputPath.startsWith(basePath)) {
                throw new SecurityException("Path traversal attempt detected");
            }
            
            // Open file descriptor first, then validate
            File inputFile = inputPath.toFile();
            if (!inputFile.exists()) {
                throw new FileNotFoundException("Input file not found");
            }
            
            // Validate it's a regular file, not symlink or directory
            if (!Files.isRegularFile(inputPath, LinkOption.NOFOLLOW_LINKS)) {
                throw new SecurityException("Input must be a regular file");
            }
            
            // Check file size before reading
            long fileSize = Files.size(inputPath);
            if (fileSize > MAX_FILE_SIZE) {
                throw new SecurityException("File size exceeds maximum allowed");
            }
            
            // Read file contents with bounds checking
            byte[] buffer;
            int bytesRead = 0;
            
            try (FileInputStream fis = new FileInputStream(inputFile)) {
                buffer = new byte[(int) fileSize];
                int offset = 0;
                int remaining = (int) fileSize;
                
                while (remaining > 0) {
                    int read = fis.read(buffer, offset, remaining);
                    if (read == -1) {
                        break;
                    }
                    bytesRead += read;
                    offset += read;
                    remaining -= read;
                }
            }
            
            // Calculate checksum using SHA-256
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] checksumBytes = digest.digest(buffer);
            
            // Check if checksum indicates empty buffer (all zeros)
            boolean isEmpty = true;
            for (byte b : checksumBytes) {
                if (b != 0) {
                    isEmpty = false;
                    break;
                }
            }
            
            String checksumHex = HexFormat.of().formatHex(checksumBytes);
            
            // Write results to output file securely
            Path outputPath = basePath.resolve(outputRelPath).normalize();
            
            // Ensure output path is within base directory
            if (!outputPath.startsWith(basePath)) {
                throw new SecurityException("Path traversal attempt detected");
            }
            
            // Create temp file in same directory for atomic write
            Path tempFile = Files.createTempFile(outputPath.getParent(), ".tmp", ".dat");
            
            try (BufferedWriter writer = Files.newBufferedWriter(tempFile, 
                    StandardOpenOption.WRITE, 
                    StandardOpenOption.CREATE, 
                    StandardOpenOption.TRUNCATE_EXISTING)) {
                writer.write("Bytes Read: " + bytesRead + "\\n");
                writer.write("Checksum: " + checksumHex + "\\n");
                writer.write("Buffer Empty: " + isEmpty + "\\n");
                writer.flush();
            }
            
            // Atomic move to final location
            Files.move(tempFile, outputPath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            
            return new FileResult(bytesRead, checksumHex, isEmpty);
            
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 algorithm not available", e);
        } catch (IOException e) {
            throw new RuntimeException("File operation failed", e);
        }
    }
    
    public static class FileResult {
        public final int bytesRead;
        public final String checksum;
        public final boolean isEmpty;
        
        public FileResult(int bytesRead, String checksum, boolean isEmpty) {
            this.bytesRead = bytesRead;
            this.checksum = checksum;
            this.isEmpty = isEmpty;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String baseDir = System.getProperty("java.io.tmpdir");
        
        try {
            // Test case 1: Normal file with content
            Path test1 = Paths.get(baseDir, "test1.txt");
            Files.write(test1, "Hello World".getBytes(java.nio.charset.StandardCharsets.UTF_8));
            FileResult result1 = processFile(baseDir, "test1.txt", "output1.txt");
            System.out.println("Test 1 - Bytes: " + result1.bytesRead + ", Empty: " + result1.isEmpty);
            
            // Test case 2: Empty file
            Path test2 = Paths.get(baseDir, "test2.txt");
            Files.write(test2, new byte[0]);
            FileResult result2 = processFile(baseDir, "test2.txt", "output2.txt");
            System.out.println("Test 2 - Bytes: " + result2.bytesRead + ", Empty: " + result2.isEmpty);
            
            // Test case 3: File with binary data
            Path test3 = Paths.get(baseDir, "test3.bin");
            Files.write(test3, new byte[]{1, 2, 3, 4, 5});
            FileResult result3 = processFile(baseDir, "test3.bin", "output3.txt");
            System.out.println("Test 3 - Bytes: " + result3.bytesRead + ", Empty: " + result3.isEmpty);
            
            // Test case 4: Larger file
            Path test4 = Paths.get(baseDir, "test4.txt");
            byte[] largeData = new byte[1024];
            for (int i = 0; i < largeData.length; i++) {
                largeData[i] = (byte) (i % 256);
            }
            Files.write(test4, largeData);
            FileResult result4 = processFile(baseDir, "test4.txt", "output4.txt");
            System.out.println("Test 4 - Bytes: " + result4.bytesRead + ", Empty: " + result4.isEmpty);
            
            // Test case 5: File with special characters
            Path test5 = Paths.get(baseDir, "test5.txt");
            Files.write(test5, "Special: \\n\\t\\r".getBytes(java.nio.charset.StandardCharsets.UTF_8));
            FileResult result5 = processFile(baseDir, "test5.txt", "output5.txt");
            System.out.println("Test 5 - Bytes: " + result5.bytesRead + ", Empty: " + result5.isEmpty);
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
