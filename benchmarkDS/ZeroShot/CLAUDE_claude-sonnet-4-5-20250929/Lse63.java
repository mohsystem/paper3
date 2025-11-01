
import java.io.*;
import java.nio.file.*;

public class Lse63 {
    public static class ChecksumResult {
        public int bytesRead;
        public long checksum;
        
        public ChecksumResult(int bytesRead, long checksum) {
            this.bytesRead = bytesRead;
            this.checksum = checksum;
        }
    }
    
    public static ChecksumResult processFile(String inputPath, String outputPath) {
        try {
            // Read file contents into buffer
            byte[] buffer = Files.readAllBytes(Paths.get(inputPath));
            int bytesRead = buffer.length;
            
            // Calculate checksum
            long checksum = calculateChecksum(buffer);
            
            // Check if buffer is empty (checksum is 0)
            if (checksum == 0) {
                System.out.println("Buffer is empty or checksum is 0");
            }
            
            // Save results to output file
            saveResults(outputPath, bytesRead, checksum);
            
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
    
    private static void saveResults(String outputPath, int bytesRead, long checksum) throws IOException {
        try (BufferedWriter writer = new BufferedWriter(new FileWriter(outputPath))) {
            writer.write("Bytes Read: " + bytesRead + "\\n");
            writer.write("Checksum: " + checksum + "\\n");
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: File with some content
        try {
            Files.write(Paths.get("test1.txt"), "Hello World".getBytes());
            ChecksumResult result1 = processFile("test1.txt", "output1.txt");
            System.out.println("Test 1 - Bytes: " + result1.bytesRead + ", Checksum: " + result1.checksum);
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 2: Empty file
        try {
            Files.write(Paths.get("test2.txt"), "".getBytes());
            ChecksumResult result2 = processFile("test2.txt", "output2.txt");
            System.out.println("Test 2 - Bytes: " + result2.bytesRead + ", Checksum: " + result2.checksum);
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 3: File with numbers
        try {
            Files.write(Paths.get("test3.txt"), "1234567890".getBytes());
            ChecksumResult result3 = processFile("test3.txt", "output3.txt");
            System.out.println("Test 3 - Bytes: " + result3.bytesRead + ", Checksum: " + result3.checksum);
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 4: File with special characters
        try {
            Files.write(Paths.get("test4.txt"), "!@#$%^&*()".getBytes());
            ChecksumResult result4 = processFile("test4.txt", "output4.txt");
            System.out.println("Test 4 - Bytes: " + result4.bytesRead + ", Checksum: " + result4.checksum);
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 5: File with longer text
        try {
            Files.write(Paths.get("test5.txt"), "The quick brown fox jumps over the lazy dog".getBytes());
            ChecksumResult result5 = processFile("test5.txt", "output5.txt");
            System.out.println("Test 5 - Bytes: " + result5.bytesRead + ", Checksum: " + result5.checksum);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
