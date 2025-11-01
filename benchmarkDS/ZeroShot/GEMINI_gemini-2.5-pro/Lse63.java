import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.Arrays;

public class Lse63 {

    // Define a reasonable maximum file size to prevent DoS attacks (e.g., 100 MB)
    private static final long MAX_FILE_SIZE = 100 * 1024 * 1024;

    /**
     * Reads a file, calculates a checksum, and writes the results to another file.
     *
     * @param inputPathStr  The path to the input file.
     * @param outputPathStr The path to the output file.
     * @return true on success, false on failure.
     */
    public static boolean processFile(String inputPathStr, String outputPathStr) {
        Path inputPath = Paths.get(inputPathStr);
        Path outputPath = Paths.get(outputPathStr);

        // 1. Validate input path
        if (!Files.exists(inputPath) || !Files.isReadable(inputPath)) {
            System.err.println("Error: Input file does not exist or is not readable: " + inputPathStr);
            return false;
        }

        try {
            // 2. Securely check file size before reading
            long fileSize = Files.size(inputPath);
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: Input file is too large: " + fileSize + " bytes. Max allowed: " + MAX_FILE_SIZE + " bytes.");
                return false;
            }

            // 3. Read file contents into a buffer
            byte[] buffer = Files.readAllBytes(inputPath);
            long bytesRead = buffer.length;

            // 4. Calculate checksum
            long checksum = 0;
            for (byte b : buffer) {
                // Treat byte as unsigned for calculation
                checksum += (b & 0xFF);
            }

            // The prompt's logic: "If the checksum is 0 then the buffer is empty."
            // This is naturally true if the file is empty (bytesRead == 0, checksum == 0).
            // We don't need a special check; the output will reflect this.
            if (bytesRead == 0) {
                System.out.println("Info: Buffer is empty for file " + inputPathStr);
            }

            // 5. Save the number of bytes read and checksum to the output file
            // Use try-with-resources to ensure the writer is closed automatically.
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(outputPath.toFile()))) {
                writer.write("Bytes read: " + bytesRead + "\n");
                writer.write("Checksum: " + checksum + "\n");
            } catch (IOException e) {
                System.err.println("Error: Could not write to output file: " + outputPathStr);
                e.printStackTrace();
                return false;
            }

        } catch (IOException | SecurityException e) {
            System.err.println("Error: Failed to process file: " + inputPathStr);
            e.printStackTrace();
            return false;
        }

        return true;
    }

    // Main method with 5 test cases
    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");
        runTestCases();
        System.out.println("--- Java Test Cases Finished ---\n");
    }

    private static void runTestCases() {
        // Test Case 1: Empty file
        String emptyIn = "test_empty_in.txt";
        String emptyOut = "test_empty_out.txt";
        createTestFile(emptyIn, new byte[0]);
        System.out.println("Test 1: Empty File");
        runTest("Test 1", emptyIn, emptyOut, true);

        // Test Case 2: ASCII text file
        String asciiIn = "test_ascii_in.txt";
        String asciiOut = "test_ascii_out.txt";
        createTestFile(asciiIn, "Hello World!".getBytes());
        System.out.println("Test 2: ASCII File");
        runTest("Test 2", asciiIn, asciiOut, true);

        // Test Case 3: Binary file
        String binaryIn = "test_binary_in.txt";
        String binaryOut = "test_binary_out.txt";
        createTestFile(binaryIn, new byte[]{0x01, 0x02, 0x00, (byte) 0xFF, (byte) 0xFE});
        System.out.println("Test 3: Binary File");
        runTest("Test 3", binaryIn, binaryOut, true);
        
        // Test Case 4: Non-existent input file
        String nonExistentIn = "non_existent_file.txt";
        String nonExistentOut = "test_no_input_out.txt";
        System.out.println("Test 4: Non-existent Input File");
        runTest("Test 4", nonExistentIn, nonExistentOut, false);
        
        // Test Case 5: Unwritable output file (e.g., invalid path)
        String goodIn = "test_ascii_in.txt"; // Re-use a valid input file
        String unwritableOut = "non_existent_dir/output.txt";
        System.out.println("Test 5: Unwritable Output Path");
        runTest("Test 5", goodIn, unwritableOut, false);

        // Cleanup
        cleanupTestFiles(new String[]{emptyIn, emptyOut, asciiIn, asciiOut, binaryIn, binaryOut, nonExistentOut});
    }

    private static void runTest(String testName, String in, String out, boolean expected) {
        boolean result = processFile(in, out);
        if (result == expected) {
            System.out.println("  " + testName + ": PASSED");
        } else {
            System.out.println("  " + testName + ": FAILED (Expected " + expected + ", got " + result + ")");
        }
    }

    private static void createTestFile(String path, byte[] content) {
        try {
            Files.write(Paths.get(path), content);
        } catch (IOException e) {
            System.err.println("Failed to create test file: " + path);
            e.printStackTrace();
        }
    }

    private static void cleanupTestFiles(String[] files) {
        for (String file : files) {
            try {
                Files.deleteIfExists(Paths.get(file));
            } catch (IOException e) {
                // Ignore cleanup errors
            }
        }
    }
}