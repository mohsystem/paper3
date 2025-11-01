
import java.io.*;
import java.nio.file.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Lse62 {
    
    public static class ChecksumResult {
        public long bytesRead;
        public long checksum;
        
        public ChecksumResult(long bytesRead, long checksum) {
            this.bytesRead = bytesRead;
            this.checksum = checksum;
        }
    }
    
    public static ChecksumResult processFile(String inputFilePath, String outputFilePath) {
        try {
            // Validate input file path
            File inputFile = new File(inputFilePath);
            if (!inputFile.exists() || !inputFile.isFile()) {
                throw new IllegalArgumentException("Invalid input file path");
            }
            
            // Read file contents securely
            byte[] buffer = Files.readAllBytes(Paths.get(inputFilePath));
            long bytesRead = buffer.length;
            
            // Calculate checksum (simple sum of bytes)
            long checksum = 0;
            for (byte b : buffer) {
                checksum += (b & 0xFF);
            }
            
            // Save results to output file
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(outputFilePath))) {
                writer.write("Bytes Read: " + bytesRead + "\\n");
                writer.write("Checksum: " + checksum + "\\n");
                if (checksum == 0) {
                    writer.write("Status: Buffer is empty or all zeros\\n");
                } else {
                    writer.write("Status: Buffer contains data\\n");
                }
            }
            
            return new ChecksumResult(bytesRead, checksum);
            
        } catch (IOException e) {
            System.err.println("Error processing file: " + e.getMessage());
            return new ChecksumResult(0, 0);
        }
    }
    
    public static void main(String[] args) {
        System.out.println("File Checksum Calculator - Test Cases\\n");
        
        // Test Case 1: Empty file
        try {
            File test1 = new File("test1_empty.txt");
            new FileWriter(test1).close();
            ChecksumResult result1 = processFile("test1_empty.txt", "output1.txt");
            System.out.println("Test 1 - Empty file: Bytes=" + result1.bytesRead + ", Checksum=" + result1.checksum);
            test1.delete();
        } catch (IOException e) {
            System.err.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test Case 2: Small text file
        try {
            File test2 = new File("test2_small.txt");
            try (FileWriter writer = new FileWriter(test2)) {
                writer.write("Hello");
            }
            ChecksumResult result2 = processFile("test2_small.txt", "output2.txt");
            System.out.println("Test 2 - Small file: Bytes=" + result2.bytesRead + ", Checksum=" + result2.checksum);
            test2.delete();
        } catch (IOException e) {
            System.err.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test Case 3: File with numbers
        try {
            File test3 = new File("test3_numbers.txt");
            try (FileWriter writer = new FileWriter(test3)) {
                writer.write("12345");
            }
            ChecksumResult result3 = processFile("test3_numbers.txt", "output3.txt");
            System.out.println("Test 3 - Numbers file: Bytes=" + result3.bytesRead + ", Checksum=" + result3.checksum);
            test3.delete();
        } catch (IOException e) {
            System.err.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test Case 4: File with special characters
        try {
            File test4 = new File("test4_special.txt");
            try (FileWriter writer = new FileWriter(test4)) {
                writer.write("@#$%^");
            }
            ChecksumResult result4 = processFile("test4_special.txt", "output4.txt");
            System.out.println("Test 4 - Special chars: Bytes=" + result4.bytesRead + ", Checksum=" + result4.checksum);
            test4.delete();
        } catch (IOException e) {
            System.err.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test Case 5: Larger text file
        try {
            File test5 = new File("test5_large.txt");
            try (FileWriter writer = new FileWriter(test5)) {
                writer.write("The quick brown fox jumps over the lazy dog.");
            }
            ChecksumResult result5 = processFile("test5_large.txt", "output5.txt");
            System.out.println("Test 5 - Larger file: Bytes=" + result5.bytesRead + ", Checksum=" + result5.checksum);
            test5.delete();
        } catch (IOException e) {
            System.err.println("Test 5 failed: " + e.getMessage());
        }
    }
}
