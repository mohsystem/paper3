
import java.io.*;
import java.nio.file.*;

public class Lse63 {
    public static class Result {
        int bytesRead;
        int checksum;
        
        public Result(int bytesRead, int checksum) {
            this.bytesRead = bytesRead;
            this.checksum = checksum;
        }
    }
    
    public static Result processFile(String inputFilePath, String outputFilePath) {
        try {
            // Read file contents into buffer
            byte[] buffer = Files.readAllBytes(Paths.get(inputFilePath));
            int bytesRead = buffer.length;
            
            // Calculate checksum
            int checksum = 0;
            for (byte b : buffer) {
                checksum += (b & 0xFF);
            }
            
            // Write results to output file
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(outputFilePath))) {
                writer.write("Bytes Read: " + bytesRead + "\\n");
                writer.write("Checksum: " + checksum + "\\n");
            }
            
            return new Result(bytesRead, checksum);
        } catch (IOException e) {
            e.printStackTrace();
            return new Result(0, 0);
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Empty file
        try {
            Files.write(Paths.get("test1.txt"), new byte[0]);
            Result r1 = processFile("test1.txt", "output1.txt");
            System.out.println("Test 1 - Bytes: " + r1.bytesRead + ", Checksum: " + r1.checksum);
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 2: Small text file
        try {
            Files.write(Paths.get("test2.txt"), "Hello".getBytes());
            Result r2 = processFile("test2.txt", "output2.txt");
            System.out.println("Test 2 - Bytes: " + r2.bytesRead + ", Checksum: " + r2.checksum);
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 3: Numbers
        try {
            Files.write(Paths.get("test3.txt"), "12345".getBytes());
            Result r3 = processFile("test3.txt", "output3.txt");
            System.out.println("Test 3 - Bytes: " + r3.bytesRead + ", Checksum: " + r3.checksum);
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 4: Special characters
        try {
            Files.write(Paths.get("test4.txt"), "!@#$%".getBytes());
            Result r4 = processFile("test4.txt", "output4.txt");
            System.out.println("Test 4 - Bytes: " + r4.bytesRead + ", Checksum: " + r4.checksum);
        } catch (IOException e) {
            e.printStackTrace();
        }
        
        // Test case 5: Binary data
        try {
            byte[] binaryData = {1, 2, 3, 4, 5, -1, -2, -3};
            Files.write(Paths.get("test5.txt"), binaryData);
            Result r5 = processFile("test5.txt", "output5.txt");
            System.out.println("Test 5 - Bytes: " + r5.bytesRead + ", Checksum: " + r5.checksum);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
