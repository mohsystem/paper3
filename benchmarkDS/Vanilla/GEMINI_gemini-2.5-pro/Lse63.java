import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.LinkedHashMap;
import java.util.Map;

public class Lse63 {

    /**
     * Reads the contents of a file into a buffer, calculates a simple checksum,
     * and saves the number of bytes read and the checksum to another file.
     *
     * @param inputFilePath  The path to the input file.
     * @param outputFilePath The path where the results will be saved.
     */
    public static void processFile(String inputFilePath, String outputFilePath) {
        try {
            byte[] buffer = Files.readAllBytes(Paths.get(inputFilePath));
            int bytesRead = buffer.length;
            int checksum = 0;

            // If the buffer is empty, bytesRead is 0, and the checksum remains 0.
            if (bytesRead > 0) {
                for (byte b : buffer) {
                    // Treat byte as unsigned (0-255) for checksum calculation
                    checksum += (b & 0xFF);
                }
                checksum %= 256;
            }

            String result = "Bytes read: " + bytesRead + "\n" +
                            "Checksum: " + checksum;

            Files.write(Paths.get(outputFilePath), result.getBytes());

        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        // Use LinkedHashMap to maintain insertion order for predictable test output
        Map<String, byte[]> testCases = new LinkedHashMap<>();
        testCases.put("test1_in.txt", new byte[]{}); // Empty file
        testCases.put("test2_in.txt", new byte[]{'A'}); // Single character (ASCII 65)
        testCases.put("test3_in.txt", "Hello".getBytes()); // Simple string
        testCases.put("test4_in.txt", new byte[]{(byte) 0x80, (byte) 0x80}); // Non-empty, checksum 0 (128+128=256)
        testCases.put("test5_in.txt", new byte[]{0x00}); // Null byte

        int testNum = 1;
        for (Map.Entry<String, byte[]> entry : testCases.entrySet()) {
            String inputFileName = entry.getKey();
            byte[] content = entry.getValue();
            String outputFileName = "test" + testNum + "_out.txt";

            System.out.println("--- Test Case " + testNum + " ---");
            System.out.println("Input File: " + inputFileName);

            File inputFile = new File(inputFileName);
            File outputFile = new File(outputFileName);
            try {
                // 1. Create input file with specified content
                Files.write(inputFile.toPath(), content, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
                
                // 2. Process the file
                processFile(inputFileName, outputFileName);

                // 3. Read and display the output file content for verification
                if (outputFile.exists()) {
                    String outputContent = new String(Files.readAllBytes(outputFile.toPath()));
                    System.out.println("Output from " + outputFileName + ":");
                    System.out.print(outputContent);
                } else {
                    System.out.println("Output file " + outputFileName + " was not created.");
                }

            } catch (IOException e) {
                System.err.println("Error during test case " + testNum + ": " + e.getMessage());
            } finally {
                // 4. Clean up created files
                if (inputFile.exists()) {
                    inputFile.delete();
                }
                if (outputFile.exists()) {
                    outputFile.delete();
                }
                System.out.println("\n---------------------\n");
                testNum++;
            }
        }
    }
}