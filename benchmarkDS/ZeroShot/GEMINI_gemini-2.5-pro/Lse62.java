import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;

public class Lse62 {
    /**
     * Reads a file into a buffer, calculates its checksum, and saves the byte count and checksum to an output file.
     * This implementation reads the input file in chunks to avoid loading very large files into memory.
     *
     * @param inputFilePath  The path to the file to be read.
     * @param outputFilePath The path to the file where results will be written.
     * @return true on success, false on failure.
     */
    public static boolean processFile(String inputFilePath, String outputFilePath) {
        long numberOfBytesRead = 0;
        long checksum = 0;
        final int BUFFER_SIZE = 4096;
        byte[] buffer = new byte[BUFFER_SIZE];

        // Use try-with-resources for automatic closing of streams, preventing resource leaks.
        try (InputStream inputStream = new FileInputStream(inputFilePath)) {
            int bytesReadInChunk;
            // Read the file in chunks until the end is reached.
            while ((bytesReadInChunk = inputStream.read(buffer)) != -1) {
                numberOfBytesRead += bytesReadInChunk;
                for (int i = 0; i < bytesReadInChunk; i++) {
                    // Add the unsigned value of the byte to the checksum.
                    // (buffer[i] & 0xFF) converts the signed byte to an int without sign extension.
                    checksum += buffer[i] & 0xFF;
                }
            }
        } catch (FileNotFoundException e) {
            System.err.println("Error: Input file not found: " + inputFilePath);
            return false;
        } catch (IOException e) {
            System.err.println("Error reading from file: " + inputFilePath + " - " + e.getMessage());
            return false;
        } catch (SecurityException e) {
            System.err.println("Error: Insufficient permissions to read input file: " + inputFilePath);
            return false;
        }

        // Per the prompt, a checksum of 0 indicates an empty buffer.
        // This holds true for an empty file, where both bytes read and checksum will be 0.
        if (checksum == 0) {
            System.out.println("Buffer is empty for file: " + inputFilePath);
        }

        try (Writer writer = new FileWriter(outputFilePath)) {
            writer.write("Bytes read: " + numberOfBytesRead + "\n");
            writer.write("Checksum: " + checksum + "\n");
        } catch (IOException e) {
            System.err.println("Error writing to file: " + outputFilePath + " - " + e.getMessage());
            return false;
        } catch (SecurityException e) {
            System.err.println("Error: Insufficient permissions to write output file: " + outputFilePath);
            return false;
        }

        return true;
    }

    public static void main(String[] args) {
        // Setup for test cases
        try {
            Files.write(Paths.get("test1.txt"), new byte[0]); // Empty file
            Files.write(Paths.get("test2.txt"), "Hello".getBytes()); // Simple text: 72+101+108+108+111 = 500
            Files.write(Paths.get("test3.bin"), new byte[]{(byte) 0x01, (byte) 0x02, (byte) 0xFF}); // Binary data: 1+2+255 = 258
        } catch (IOException e) {
            System.err.println("Failed to create test files: " + e.getMessage());
            return;
        }

        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Empty file
        System.out.println("\nTest Case 1: Empty File");
        runTestCase("test1.txt", "output1.txt");

        // Test Case 2: Small text file
        System.out.println("\nTest Case 2: Small Text File");
        runTestCase("test2.txt", "output2.txt");

        // Test Case 3: Small binary file
        System.out.println("\nTest Case 3: Small Binary File");
        runTestCase("test3.bin", "output3.txt");

        // Test Case 4: Non-existent input file
        System.out.println("\nTest Case 4: Non-existent Input File");
        runTestCase("nonexistent.txt", "output4.txt");

        // Test Case 5: Unwritable output file (simulated by using a directory)
        System.out.println("\nTest Case 5: Unwritable Output File");
        new File("output5.txt").mkdir();
        runTestCase("test1.txt", "output5.txt");

        // Cleanup test files
        System.out.println("\n--- Cleaning up test files ---");
        try {
            Files.deleteIfExists(Paths.get("test1.txt"));
            Files.deleteIfExists(Paths.get("test2.txt"));
            Files.deleteIfExists(Paths.get("test3.bin"));
            Files.deleteIfExists(Paths.get("output1.txt"));
            Files.deleteIfExists(Paths.get("output2.txt"));
            Files.deleteIfExists(Paths.get("output3.txt"));
            Files.deleteIfExists(Paths.get("output5.txt"));
            System.out.println("Cleanup complete.");
        } catch (IOException e) {
            System.err.println("Error during cleanup: " + e.getMessage());
        }
    }

    private static void runTestCase(String inputFile, String outputFile) {
        System.out.println("Processing " + inputFile + " -> " + outputFile);
        boolean success = processFile(inputFile, outputFile);
        System.out.println("Result: " + (success ? "SUCCESS" : "FAILURE"));
        if (success) {
            try {
                String content = new String(Files.readAllBytes(Paths.get(outputFile)));
                System.out.println("Output content:\n---\n" + content.trim() + "\n---");
            } catch (IOException e) {
                System.err.println("Could not read output file " + outputFile);
            }
        }
    }
}