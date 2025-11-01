import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Collections;

public class Lse63 {

    /**
     * Reads a file into a buffer, calculates an 8-bit checksum,
     * and writes the number of bytes read and the checksum to an output file.
     *
     * @param inputFilePath  The path to the input file.
     * @param outputFilePath The path to the output file.
     * @return true on success, false on failure.
     */
    public static boolean processFileAndGetChecksum(String inputFilePath, String outputFilePath) {
        try {
            Path inputPath = Paths.get(inputFilePath);
            Path outputPath = Paths.get(outputFilePath);

            // 1. Read the contents of a file into a buffer.
            if (!Files.exists(inputPath)) {
                System.err.println("Error: Input file does not exist: " + inputFilePath);
                return false;
            }
            byte[] buffer = Files.readAllBytes(inputPath);
            int bytesRead = buffer.length;

            // 2. Calculate the checksum of the buffer.
            int checksum = 0;
            for (byte b : buffer) {
                // Treat byte as unsigned (0-255)
                checksum = (checksum + (b & 0xFF)) % 256;
            }

            // 3. Checksum is 0 if buffer is empty. (This is guaranteed)
            // Note: A non-empty buffer can also have a checksum of 0.
            if (bytesRead == 0) {
                 // This confirms the prompt's logic for an empty file.
                 assert checksum == 0;
            }

            // 4. Save the number of bytes read and the checksum to another file.
            String result = "Bytes read: " + bytesRead + ", Checksum: " + checksum;
            Files.write(outputPath, result.getBytes());

            return true;
        } catch (IOException | SecurityException e) {
            System.err.println("An error occurred: " + e.getMessage());
            return false;
        }
    }

    // Main method with 5 test cases
    public static void main(String[] args) {
        System.out.println("--- Java Test Cases ---");

        // Test Case 1: Standard text file
        try {
            Path test1_in = Paths.get("test1_in.txt");
            Files.write(test1_in, "Hello, World!".getBytes());
            processFileAndGetChecksum("test1_in.txt", "test1_out.txt");
            System.out.println("Test Case 1 (Text): " + Files.readString(Paths.get("test1_out.txt")));
            Files.delete(test1_in);
            Files.delete(Paths.get("test1_out.txt"));
        } catch (IOException e) { System.err.println("Test Case 1 failed: " + e.getMessage()); }

        // Test Case 2: Binary data
        try {
            Path test2_in = Paths.get("test2_in.bin");
            Files.write(test2_in, new byte[]{(byte)0xDE, (byte)0xAD, (byte)0xBE, (byte)0xEF});
            processFileAndGetChecksum("test2_in.bin", "test2_out.txt");
            System.out.println("Test Case 2 (Binary): " + Files.readString(Paths.get("test2_out.txt")));
            Files.delete(test2_in);
            Files.delete(Paths.get("test2_out.txt"));
        } catch (IOException e) { System.err.println("Test Case 2 failed: " + e.getMessage()); }
        
        // Test Case 3: Empty file
        try {
            Path test3_in = Paths.get("test3_in.txt");
            Files.write(test3_in, "".getBytes());
            processFileAndGetChecksum("test3_in.txt", "test3_out.txt");
            System.out.println("Test Case 3 (Empty): " + Files.readString(Paths.get("test3_out.txt")));
            Files.delete(test3_in);
            Files.delete(Paths.get("test3_out.txt"));
        } catch (IOException e) { System.err.println("Test Case 3 failed: " + e.getMessage()); }

        // Test Case 4: Non-existent file
        System.out.print("Test Case 4 (Not Found): ");
        processFileAndGetChecksum("non_existent_file.txt", "test4_out.txt");

        // Test Case 5: File with content that results in a zero checksum
        try {
            Path test5_in = Paths.get("test5_in.bin");
            Files.write(test5_in, new byte[]{(byte)0x01, (byte)0xFF});
            processFileAndGetChecksum("test5_in.bin", "test5_out.txt");
            System.out.println("Test Case 5 (Zero Checksum): " + Files.readString(Paths.get("test5_out.txt")));
            Files.delete(test5_in);
            Files.delete(Paths.get("test5_out.txt"));
        } catch (IOException e) { System.err.println("Test Case 5 failed: " + e.getMessage()); }
        
        System.out.println("--- Java Tests Complete ---\n");
    }
}