import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.EnumSet;

public class Lse62 {

    // Define a reasonable maximum file size to prevent resource exhaustion.
    private static final long MAX_FILE_SIZE = 16 * 1024 * 1024; // 16 MB

    /**
     * Reads the contents of a file into a buffer, calculates a checksum,
     * and saves the number of bytes read and the checksum to an output file.
     *
     * @param inputPathStr Path to the input file.
     * @param outputPathStr Path to the output file. The file must not exist.
     * @return true on success, false on failure.
     */
    public static boolean processFile(String inputPathStr, String outputPathStr) {
        Path inputPath = Paths.get(inputPathStr);
        Path outputPath = Paths.get(outputPathStr);

        // Note on TOCTOU: Java's standard nio API doesn't have a direct equivalent of
        // fstat() on an open file handle. The most secure approach is to perform
        // checks on the path with NOFOLLOW_LINKS and then immediately open the file
        // with the same option. This minimizes the race condition window.
        try {
            BasicFileAttributes attrs = Files.readAttributes(inputPath, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            if (!attrs.isRegularFile()) {
                System.err.println("Error: Input is not a regular file.");
                return false;
            }
            if (attrs.size() > MAX_FILE_SIZE) {
                System.err.println("Error: Input file size exceeds the limit of " + MAX_FILE_SIZE + " bytes.");
                return false;
            }
        } catch (NoSuchFileException e) {
            System.err.println("Error: Input file not found: " + inputPathStr);
            return false;
        } catch (IOException e) {
            System.err.println("Error accessing input file attributes: " + e.getMessage());
            return false;
        }

        byte[] buffer;
        long bytesReadCount;
        
        // Use try-with-resources to ensure the channel is closed automatically.
        // Open the file with NOFOLLOW_LINKS to prevent symbolic link attacks.
        try (FileChannel channel = FileChannel.open(inputPath, StandardOpenOption.READ, LinkOption.NOFOLLOW_LINKS)) {
            long fileSize = channel.size();
            // This second size check on the handle is safe against race conditions after opening.
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: File size changed after initial check and now exceeds the limit.");
                return false;
            }

            if (fileSize > Integer.MAX_VALUE) {
                System.err.println("Error: File is too large to fit in a byte array.");
                return false;
            }

            // Allocate buffer based on validated size to prevent overflow.
            buffer = new byte[(int) fileSize];
            if (fileSize > 0) {
                ByteBuffer byteBuffer = ByteBuffer.wrap(buffer);
                int totalBytesRead = 0;
                while (totalBytesRead < fileSize) {
                    int read = channel.read(byteBuffer);
                    if (read == -1) {
                        break; // End of file
                    }
                    totalBytesRead += read;
                }
                
                if (totalBytesRead != fileSize) {
                    System.err.println("Error: Incomplete read. Expected " + fileSize + ", got " + totalBytesRead + " bytes.");
                    return false;
                }
            }
            bytesReadCount = fileSize;
        } catch (IOException e) {
            System.err.println("Error reading input file: " + e.getMessage());
            return false;
        }

        // Calculate a simple checksum of the buffer.
        long checksum = 0;
        for (byte b : buffer) {
            checksum += (b & 0xFF); // Treat byte as unsigned for summation
        }

        // Prepare the output content.
        String outputContent = String.format("Bytes read: %d\nChecksum: %d\n", bytesReadCount, checksum);
        byte[] outputBytes = outputContent.getBytes(StandardCharsets.UTF_8);

        // Create the output file securely.
        // CREATE_NEW is atomic and fails if the file already exists, preventing overwrites.
        try (FileChannel outChannel = FileChannel.open(outputPath,
                EnumSet.of(StandardOpenOption.WRITE, StandardOpenOption.CREATE_NEW))) {
            outChannel.write(ByteBuffer.wrap(outputBytes));
        } catch (FileAlreadyExistsException e) {
            System.err.println("Error: Output file already exists: " + outputPathStr);
            return false;
        } catch (IOException e) {
            System.err.println("Error writing to output file: " + e.getMessage());
            return false;
        }
        
        System.out.println("Successfully processed '" + inputPathStr + "', results in '" + outputPathStr + "'");
        return true;
    }

    private static void runTest(String name, byte[] content, boolean shouldSucceed) {
        System.out.println("--- Test Case: " + name + " ---");
        String inputFile = "test_input.txt";
        String outputFile = "test_output.txt";

        try {
            Files.deleteIfExists(Paths.get(inputFile));
            Files.deleteIfExists(Paths.get(outputFile));
        } catch (IOException e) { /* ignore cleanup errors */ }
        
        if (content != null) {
            try {
                Files.write(Paths.get(inputFile), content);
            } catch (IOException e) {
                System.err.println("Failed to create test input file: " + e.getMessage());
                return;
            }
        } else {
             System.out.println("Testing with non-existent input file.");
        }
        
        boolean result = processFile(inputFile, outputFile);
        
        if (shouldSucceed) {
            if (result) {
                System.out.println("PASS: Process succeeded as expected.");
            } else {
                System.out.println("FAIL: Process failed unexpectedly.");
            }
        } else {
            if (!result) {
                System.out.println("PASS: Process failed as expected.");
            } else {
                System.out.println("FAIL: Process succeeded unexpectedly.");
            }
        }
        
        try {
            Files.deleteIfExists(Paths.get(inputFile));
            Files.deleteIfExists(Paths.get(outputFile));
        } catch (IOException e) { /* ignore cleanup errors */ }
        System.out.println();
    }

    public static void main(String[] args) {
        runTest("Normal file", "Hello World!".getBytes(StandardCharsets.UTF_8), true);
        runTest("Empty file", new byte[0], true);
        runTest("File with null bytes", new byte[]{'a', 0, 'b', 0, 'c'}, true);
        runTest("Non-existent file", null, false);

        System.out.println("--- Test Case: Output file exists ---");
        String inputFile = "test_input.txt";
        String outputFile = "test_output.txt";
        try {
            Files.deleteIfExists(Paths.get(inputFile));
            Files.deleteIfExists(Paths.get(outputFile));
            Files.createFile(Paths.get(inputFile));
            Files.createFile(Paths.get(outputFile));
        } catch (IOException e) {
            System.err.println("Setup for 'output exists' test failed: " + e.getMessage());
            return;
        }

        boolean result = processFile(inputFile, outputFile);
        if (!result) {
            System.out.println("PASS: Process failed as expected because output file exists.");
        } else {
            System.out.println("FAIL: Process succeeded but should have failed.");
        }
        
        try {
            Files.deleteIfExists(Paths.get(inputFile));
            Files.deleteIfExists(Paths.get(outputFile));
        } catch (IOException e) { /* ignore cleanup errors */ }
        System.out.println();
    }
}