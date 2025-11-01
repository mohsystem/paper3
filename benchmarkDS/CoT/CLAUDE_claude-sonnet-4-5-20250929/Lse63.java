
import java.io.*;
import java.nio.file.*;

public class Lse63 {
    private static final int MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB limit
    
    public static class ChecksumResult {
        public long bytesRead;
        public long checksum;
        
        public ChecksumResult(long bytesRead, long checksum) {
            this.bytesRead = bytesRead;
            this.checksum = checksum;
        }
    }
    
    public static ChecksumResult processFile(String inputPath, String outputPath) {
        if (inputPath == null || outputPath == null || inputPath.isEmpty() || outputPath.isEmpty()) {
            throw new IllegalArgumentException("File paths cannot be null or empty");
        }
        
        File inputFile = new File(inputPath);
        long bytesRead = 0;
        long checksum = 0;
        
        try {
            // Validate file exists and is readable
            if (!inputFile.exists() || !inputFile.isFile() || !inputFile.canRead()) {
                throw new IOException("Invalid input file");
            }
            
            // Check file size to prevent memory exhaustion
            long fileSize = inputFile.length();
            if (fileSize > MAX_FILE_SIZE) {
                throw new IOException("File size exceeds maximum allowed size");
            }
            
            // Read file contents
            byte[] buffer = Files.readAllBytes(inputFile.toPath());
            bytesRead = buffer.length;
            
            // Calculate checksum
            for (byte b : buffer) {
                checksum = (checksum + (b & 0xFF)) & 0xFFFFFFFFL; // Prevent overflow
            }
            
            // Write results to output file
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(outputPath))) {
                writer.write("Bytes Read: " + bytesRead + "\\n");
                writer.write("Checksum: " + checksum + "\\n");
                writer.write("Buffer Empty: " + (checksum == 0) + "\\n");
            }
            
        } catch (IOException e) {
            System.err.println("Error processing file: " + e.getMessage());
            return new ChecksumResult(0, 0);
        }
        
        return new ChecksumResult(bytesRead, checksum);
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal file with content
        try {
            Files.write(Paths.get("test1.txt"), "Hello World".getBytes());
            ChecksumResult result1 = processFile("test1.txt", "output1.txt");
            System.out.println("Test 1 - Bytes: " + result1.bytesRead + ", Checksum: " + result1.checksum);
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test case 2: Empty file
        try {
            Files.write(Paths.get("test2.txt"), new byte[0]);
            ChecksumResult result2 = processFile("test2.txt", "output2.txt");
            System.out.println("Test 2 - Bytes: " + result2.bytesRead + ", Checksum: " + result2.checksum);
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test case 3: File with numbers
        try {
            Files.write(Paths.get("test3.txt"), "1234567890".getBytes());
            ChecksumResult result3 = processFile("test3.txt", "output3.txt");
            System.out.println("Test 3 - Bytes: " + result3.bytesRead + ", Checksum: " + result3.checksum);
        } catch (Exception e) {
            System.out.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test case 4: Binary content
        try {
            byte[] binary = {0x01, 0x02, 0x03, 0x04, 0x05};
            Files.write(Paths.get("test4.txt"), binary);
            ChecksumResult result4 = processFile("test4.txt", "output4.txt");
            System.out.println("Test 4 - Bytes: " + result4.bytesRead + ", Checksum: " + result4.checksum);
        } catch (Exception e) {
            System.out.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test case 5: Non-existent file
        try {
            ChecksumResult result5 = processFile("nonexistent.txt", "output5.txt");
            System.out.println("Test 5 - Bytes: " + result5.bytesRead + ", Checksum: " + result5.checksum);
        } catch (Exception e) {
            System.out.println("Test 5 handled error correctly");
        }
    }
}
