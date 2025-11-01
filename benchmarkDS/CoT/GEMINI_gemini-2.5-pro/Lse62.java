import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;

public class Lse62 {

    /**
     * Reads a file into a buffer, calculates a checksum, and saves the byte count
     * and checksum to an output file.
     *
     * @param inputFilePath  The path to the input file.
     * @param outputFilePath The path to the output file where results will be saved.
     * @return true on success, false on failure.
     */
    public static boolean processFileAndCalculateChecksum(String inputFilePath, String outputFilePath) {
        try {
            // 1. Read the contents of the file into a buffer.
            // Using java.nio.file.Files which is modern and handles resources safely.
            // Path objects are used to represent file paths.
            byte[] buffer = Files.readAllBytes(Paths.get(inputFilePath));

            // 2. Calculate the checksum of the buffer.
            // A simple sum of all byte values. Using long to prevent overflow.
            // Java bytes are signed (-128 to 127), so we use (b & 0xFF) to get the unsigned value (0-255).
            long checksum = 0;
            for (byte b : buffer) {
                checksum += (b & 0xFF);
            }

            // The number of bytes read is the length of the buffer.
            long bytesRead = buffer.length;

            // Per instructions, if checksum is 0, the buffer is empty. This holds true for our algorithm.
            if (checksum == 0 && bytesRead > 0) {
                 System.err.println("Warning: Checksum is 0 for a non-empty file. This might indicate an issue or data peculiarity.");
            }

            // 3. Save the number of bytes read and the checksum to a file.
            String result = "Bytes: " + bytesRead + ", Checksum: " + checksum;

            // Using Files.writeString for simplicity and safety (handles closing file).
            Files.writeString(Paths.get(outputFilePath), result, StandardOpenOption.CREATE, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);

            return true;

        } catch (IOException e) {
            // Handle exceptions such as file not found, permission denied, etc.
            System.err.println("An I/O error occurred: " + e.getMessage());
            return false;
        } catch (OutOfMemoryError e) {
            System.err.println("Error: File is too large to be read into memory.");
            return false;
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---

        // Test Case 1: A normal text file.
        System.out.println("--- Test Case 1: Normal File ---");
        try {
            String inputFile1 = "test_input_1.txt";
            String outputFile1 = "test_output_1.txt";
            Files.writeString(Paths.get(inputFile1), "Hello");
            boolean success1 = processFileAndCalculateChecksum(inputFile1, outputFile1);
            System.out.println("Test 1 Success: " + success1);
            if (success1) {
                System.out.println("Output: " + Files.readString(Paths.get(outputFile1))); // Expected: Bytes: 5, Checksum: 500
            }
            new File(inputFile1).delete();
            new File(outputFile1).delete();
        } catch (IOException e) {
            System.err.println("Test 1 setup failed: " + e.getMessage());
        }
        System.out.println();

        // Test Case 2: An empty file.
        System.out.println("--- Test Case 2: Empty File ---");
        try {
            String inputFile2 = "test_input_2.txt";
            String outputFile2 = "test_output_2.txt";
            new File(inputFile2).createNewFile(); // Creates an empty file
            boolean success2 = processFileAndCalculateChecksum(inputFile2, outputFile2);
            System.out.println("Test 2 Success: " + success2);
            if (success2) {
                System.out.println("Output: " + Files.readString(Paths.get(outputFile2))); // Expected: Bytes: 0, Checksum: 0
            }
            new File(inputFile2).delete();
            new File(outputFile2).delete();
        } catch (IOException e) {
            System.err.println("Test 2 setup failed: " + e.getMessage());
        }
        System.out.println();

        // Test Case 3: A binary file.
        System.out.println("--- Test Case 3: Binary File ---");
        try {
            String inputFile3 = "test_input_3.bin";
            String outputFile3 = "test_output_3.txt";
            byte[] binaryData = {0x01, (byte)0x80, (byte)0xFF}; // 1, 128, 255
            Files.write(Paths.get(inputFile3), binaryData);
            boolean success3 = processFileAndCalculateChecksum(inputFile3, outputFile3);
            System.out.println("Test 3 Success: " + success3);
            if (success3) {
                System.out.println("Output: " + Files.readString(Paths.get(outputFile3))); // Expected: Bytes: 3, Checksum: 384
            }
            new File(inputFile3).delete();
            new File(outputFile3).delete();
        } catch (IOException e) {
            System.err.println("Test 3 setup failed: " + e.getMessage());
        }
        System.out.println();

        // Test Case 4: Non-existent input file.
        System.out.println("--- Test Case 4: Non-existent Input File ---");
        String inputFile4 = "non_existent_file.txt";
        String outputFile4 = "test_output_4.txt";
        boolean success4 = processFileAndCalculateChecksum(inputFile4, outputFile4);
        System.out.println("Test 4 Success: " + success4); // Expected: false
        new File(outputFile4).delete(); // Cleanup if created
        System.out.println();


        // Test Case 5: Unwritable output file path.
        System.out.println("--- Test Case 5: Unwritable Output Path ---");
        try {
            String inputFile5 = "test_input_5.txt";
            // Path includes a non-existent directory, which should cause an error.
            String outputFile5 = "non_existent_dir/test_output_5.txt";
            Files.writeString(Paths.get(inputFile5), "test");
            boolean success5 = processFileAndCalculateChecksum(inputFile5, outputFile5);
            System.out.println("Test 5 Success: " + success5); // Expected: false
            new File(inputFile5).delete();
        } catch (IOException e) {
             System.err.println("Test 5 setup failed: " + e.getMessage());
        }
        System.out.println();
    }
}