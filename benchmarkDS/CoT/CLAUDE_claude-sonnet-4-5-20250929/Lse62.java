
import java.io.*;
import java.nio.file.*;

public class Lse62 {
    private static final int MAX_BUFFER_SIZE = 1024 * 1024; // 1MB limit
    
    public static class ChecksumResult {
        public long bytesRead;
        public long checksum;
        
        public ChecksumResult(long bytesRead, long checksum) {
            this.bytesRead = bytesRead;
            this.checksum = checksum;
        }
    }
    
    public static ChecksumResult processFile(String inputFilePath, String outputFilePath) {
        if (inputFilePath == null || outputFilePath == null) {
            throw new IllegalArgumentException("File paths cannot be null");
        }
        
        // Validate and sanitize file paths
        try {
            Path inPath = Paths.get(inputFilePath).normalize();
            Path outPath = Paths.get(outputFilePath).normalize();
            
            // Check if input file exists
            if (!Files.exists(inPath)) {
                throw new FileNotFoundException("Input file does not exist: " + inputFilePath);
            }
            
            // Check file size to prevent excessive memory usage
            long fileSize = Files.size(inPath);
            if (fileSize > MAX_BUFFER_SIZE) {
                throw new IOException("File too large. Maximum size: " + MAX_BUFFER_SIZE);
            }
            
            // Read file into buffer
            byte[] buffer = Files.readAllBytes(inPath);
            long bytesRead = buffer.length;
            
            // Calculate checksum
            long checksum = calculateChecksum(buffer);
            
            // Save results to output file
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(outPath.toFile()))) {
                writer.write("Bytes Read: " + bytesRead + "\\n");
                writer.write("Checksum: " + checksum + "\\n");
                if (checksum == 0) {
                    writer.write("Status: Buffer is empty or checksum is zero\\n");
                }
            }
            
            return new ChecksumResult(bytesRead, checksum);
            
        } catch (IOException e) {
            System.err.println("Error processing file: " + e.getMessage());
            return new ChecksumResult(0, 0);
        }
    }
    
    private static long calculateChecksum(byte[] buffer) {
        long checksum = 0;
        for (byte b : buffer) {
            checksum += (b & 0xFF);
        }
        return checksum;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal file
        createTestFile("test1.txt", "Hello World");
        processFile("test1.txt", "output1.txt");
        
        // Test case 2: Empty file
        createTestFile("test2.txt", "");
        processFile("test2.txt", "output2.txt");
        
        // Test case 3: File with numbers
        createTestFile("test3.txt", "1234567890");
        processFile("test3.txt", "output3.txt");
        
        // Test case 4: Binary-like content
        createTestFile("test4.txt", "\\0\\0\\0\\0");
        processFile("test4.txt", "output4.txt");
        
        // Test case 5: Larger text
        createTestFile("test5.txt", "The quick brown fox jumps over the lazy dog");
        processFile("test5.txt", "output5.txt");
        
        System.out.println("All test cases executed. Check output files.");
    }
    
    private static void createTestFile(String filename, String content) {
        try {
            Files.write(Paths.get(filename), content.getBytes());
        } catch (IOException e) {
            System.err.println("Error creating test file: " + e.getMessage());
        }
    }
}
