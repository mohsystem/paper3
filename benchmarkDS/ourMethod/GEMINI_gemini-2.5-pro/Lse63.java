import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;

public class Lse63 {
    private static final long MAX_FILE_SIZE = 16 * 1024 * 1024; // 16 MB limit

    /**
     * Reads a file, calculates a checksum, and writes the results to an output file.
     *
     * @param inputPathStr  The path to the input file.
     * @param outputPathStr The path to the output file.
     * @return true on success, false on failure.
     */
    public static boolean processFileAndCalculateChecksum(String inputPathStr, String outputPathStr) {
        Path inputPath = Paths.get(inputPathStr);
        Path outputPath = Paths.get(outputPathStr);

        // Note: Java's standard library lacks a direct equivalent to open(O_NOFOLLOW) then fstat().
        // The following checks on the path before opening introduce a Time-of-Check to Time-of-Use (TOCTOU) vulnerability.
        // This is a known limitation in Java's File API. We proceed with this best-effort approach.
        try {
            BasicFileAttributes attrs = Files.readAttributes(inputPath, BasicFileAttributes.class, LinkOption.NOFOLLOW_LINKS);
            if (!attrs.isRegularFile()) {
                System.err.println("Error: Input path is not a regular file.");
                return false;
            }
            if (attrs.size() > MAX_FILE_SIZE) {
                System.err.println("Error: File size " + attrs.size() + " exceeds the maximum limit of " + MAX_FILE_SIZE + " bytes.");
                return false;
            }
        } catch (NoSuchFileException e) {
            System.err.println("Error: Input file not found: " + inputPathStr);
            return false;
        } catch (IOException e) {
            System.err.println("Error accessing input file status: " + e.getMessage());
            return false;
        }

        long bytesRead = 0;
        long checksum = 0;

        try (FileChannel fileChannel = FileChannel.open(inputPath, StandardOpenOption.READ)) {
            long fileSize = fileChannel.size();
            
            // Re-check size after getting the handle, though TOCTOU remains for file type.
            if (fileSize > MAX_FILE_SIZE) {
                System.err.println("Error: File size " + fileSize + " exceeds the maximum limit of " + MAX_FILE_SIZE + " bytes.");
                return false;
            }

            if (fileSize > Integer.MAX_VALUE) {
                System.err.println("Error: File is too large to be read into a single ByteBuffer.");
                return false;
            }
            
            bytesRead = fileSize;
            if (fileSize > 0) {
                ByteBuffer buffer = ByteBuffer.allocate((int) fileSize);
                int readResult;
                int totalRead = 0;
                while(totalRead < fileSize && (readResult = fileChannel.read(buffer)) != -1) {
                    totalRead += readResult;
                }
                
                if (totalRead != fileSize) {
                    System.err.println("Error: Could not read the entire file.");
                    return false;
                }
                
                buffer.flip();
                while (buffer.hasRemaining()) {
                    checksum += (buffer.get() & 0xFF);
                }
            }
        } catch (IOException e) {
            System.err.println("Error reading input file '" + inputPathStr + "': " + e.getMessage());
            return false;
        }

        // Securely write output to a temporary file and then atomically move it
        Path tempFile = null;
        try {
            Path outputDir = outputPath.getParent();
            if (outputDir == null) {
                outputDir = Paths.get(".");
            }
            Files.createDirectories(outputDir);
            tempFile = Files.createTempFile(outputDir, "checksum_", ".tmp");

            String content = "Bytes read: " + bytesRead + "\n" + "Checksum: " + checksum + "\n";
            
            try (OutputStream out = Files.newOutputStream(tempFile, StandardOpenOption.WRITE)) {
                 out.write(content.getBytes(StandardCharsets.UTF_8));
            }

            Files.move(tempFile, outputPath, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
        } catch (IOException e) {
            System.err.println("Error writing to output file '" + outputPathStr + "': " + e.getMessage());
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException ignored) {}
            }
            return false;
        }

        return true;
    }
    
    public static void main(String[] args) {
        Path tempDir;
        try {
            tempDir = Files.createTempDirectory("lse63_java_tests");
        } catch (IOException e) {
            System.err.println("Failed to create temp directory for tests.");
            e.printStackTrace();
            return;
        }
        
        System.out.println("Running tests in: " + tempDir);

        runTestCase(1, tempDir, "empty.txt", new byte[0], true);
        runTestCase(2, tempDir, "hello.txt", "hello world".getBytes(StandardCharsets.UTF_8), true);
        runTestCase(3, tempDir, "data.bin", new byte[]{(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF}, true);
        runTestCase(4, tempDir, "nulls.txt", new byte[]{0, 0, 0}, true);
        runTestCase(5, tempDir, "nonexistent.txt", null, false);
        
        System.out.println("\nAll tests completed.");
    }

    private static void runTestCase(int testNum, Path dir, String filename, byte[] content, boolean shouldSucceed) {
        System.out.println("\n--- Test Case " + testNum + " ---");
        Path inputFile = dir.resolve(filename);
        Path outputFile = dir.resolve(filename + ".out");

        try {
            Files.deleteIfExists(inputFile);
            Files.deleteIfExists(outputFile);
            if (content != null) {
                Files.write(inputFile, content);
            }
        } catch (IOException e) {
            System.err.println("Failed to setup test file: " + inputFile);
            e.printStackTrace();
            return;
        }
        
        System.out.println("Input file: " + inputFile);
        
        boolean result = processFileAndCalculateChecksum(inputFile.toString(), outputFile.toString());
        
        System.out.println("Execution result: " + (result ? "SUCCESS" : "FAILURE"));
        System.out.println("Expected result: " + (shouldSucceed ? "SUCCESS" : "FAILURE"));

        if (result == shouldSucceed) {
            System.out.println("Test PASSED.");
            if (shouldSucceed) {
                try {
                    String outputContent = new String(Files.readAllBytes(outputFile), StandardCharsets.UTF_8);
                    System.out.println("Output content:\n" + outputContent.trim());
                } catch (IOException e) {
                    System.err.println("Failed to read output file for verification.");
                }
            }
        } else {
            System.out.println("Test FAILED.");
        }
        
        try {
            Files.deleteIfExists(inputFile);
            Files.deleteIfExists(outputFile);
        } catch (IOException e) {
            // Ignored in test cleanup
        }
    }
}