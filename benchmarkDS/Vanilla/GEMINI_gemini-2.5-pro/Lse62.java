import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Lse62 {

    /**
     * Reads the contents of a file into a buffer, calculates the checksum of the buffer,
     * and saves the number of bytes read and the checksum to another file.
     *
     * @param inputFilePath  The path to the file to be read.
     * @param outputFilePath The path to the file where results will be saved.
     */
    public static void processFile(String inputFilePath, String outputFilePath) {
        long checksum = 0;
        long bytesRead = 0;
        Path inputPath = Paths.get(inputFilePath);

        try {
            // Check if the file exists and is not empty before reading
            if (Files.exists(inputPath) && Files.size(inputPath) > 0) {
                byte[] buffer = Files.readAllBytes(inputPath);
                bytesRead = buffer.length;

                // Calculate checksum by summing all byte values (treated as unsigned)
                for (byte b : buffer) {
                    checksum += b & 0xFF;
                }
            }
            // If file doesn't exist or is empty, bytesRead and checksum will remain 0.
            // This satisfies the condition "If the checksum is 0 the buffer is empty."

            // Save the number of bytes read and the checksum to the output file
            try (FileWriter writer = new FileWriter(outputFilePath)) {
                writer.write("Bytes read: " + bytesRead + "\n");
                writer.write("Checksum: " + checksum + "\n");
            }

        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
            try (FileWriter writer = new FileWriter(outputFilePath)) {
                writer.write("Error processing file: " + e.getMessage());
            } catch (IOException ex) {
                System.err.println("Failed to write error to output file: " + ex.getMessage());
            }
        }
    }

    // Helper function to create a test file
    private static void createTestFile(String path, byte[] content) throws IOException {
        Files.write(Paths.get(path), content);
    }

    // Helper function to read and print the content of an output file for verification
    private static void printOutputFile(String path) {
        try {
            System.out.println("--- Contents of " + path + " ---");
            String content = Files.readString(Paths.get(path));
            System.out.print(content);
            System.out.println("------------------------------------");
        } catch (IOException e) {
            System.err.println("Could not read output file " + path + ": " + e.getMessage());
        }
    }

    public static void main(String[] args) throws IOException {
        // Test Case 1: A non-empty text file
        String test1_in = "test1_in.txt";
        String test1_out = "test1_out.txt";
        createTestFile(test1_in, "Hello World!".getBytes());
        processFile(test1_in, test1_out);
        printOutputFile(test1_out);

        // Test Case 2: An empty file
        String test2_in = "test2_in.txt";
        String test2_out = "test2_out.txt";
        createTestFile(test2_in, new byte[0]);
        processFile(test2_in, test2_out);
        printOutputFile(test2_out);

        // Test Case 3: A file with binary data
        String test3_in = "test3_in.bin";
        String test3_out = "test3_out.bin";
        byte[] binaryData = {(byte) 0xDE, (byte) 0xAD, (byte) 0xBE, (byte) 0xEF};
        createTestFile(test3_in, binaryData);
        processFile(test3_in, test3_out);
        printOutputFile(test3_out);

        // Test Case 4: A file with a single byte
        String test4_in = "test4_in.txt";
        String test4_out = "test4_out.txt";
        createTestFile(test4_in, new byte[]{'A'});
        processFile(test4_in, test4_out);
        printOutputFile(test4_out);

        // Test Case 5: A file that doesn't exist
        String test5_in = "non_existent_file.txt";
        String test5_out = "test5_out.txt";
        Files.deleteIfExists(Paths.get(test5_in));
        processFile(test5_in, test5_out);
        printOutputFile(test5_out);
    }
}