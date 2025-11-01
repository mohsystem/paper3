import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.SeekableByteChannel;
import java.nio.file.*;
import java.nio.file.attribute.FileAttribute;
import java.nio.file.attribute.PosixFilePermissions;
import java.util.Base64;
import java.util.EnumSet;
import java.util.Set;
import java.util.UUID;

public class Lse69 {

    private static final long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

    /**
     * Reads an image from a validated path, encodes it in Base64, and writes it to a new file atomically.
     *
     * @param inputBaseDir   The trusted base directory for input files.
     * @param inputFilename  The name of the input image file (relative to inputBaseDir).
     * @param outputBaseDir  The trusted base directory for output files.
     * @param outputFilename The name of the output file (relative to outputBaseDir).
     * @return true on success, false on failure.
     */
    public static boolean upload_image(String inputBaseDir, String inputFilename, String outputBaseDir, String outputFilename) {
        // Rule #5: Validate paths to prevent traversal
        Path inputDir = Paths.get(inputBaseDir).toAbsolutePath();
        Path inputFile = inputDir.resolve(inputFilename).normalize();
        if (!inputFile.startsWith(inputDir) || inputFilename.contains("..")) {
            System.err.println("Error: Input path is outside the allowed directory.");
            return false;
        }

        Path outputDir = Paths.get(outputBaseDir).toAbsolutePath();
        Path outputFile = outputDir.resolve(outputFilename).normalize();
        if (!outputFile.startsWith(outputDir) || outputFilename.contains("..")) {
            System.err.println("Error: Output path is outside the allowed directory.");
            return false;
        }
        
        try {
            // Ensure output directory exists
            Files.createDirectories(outputDir);
        } catch (IOException e) {
            System.err.println("Error: Could not create output directory: " + e.getMessage());
            return false;
        }


        byte[] fileContent;
        // Rule #6: Open file first, then validate the handle (TOCTOU-safe)
        try (SeekableByteChannel channel = Files.newByteChannel(inputFile, EnumSet.of(StandardOpenOption.READ), LinkOption.NOFOLLOW_LINKS)) {
            // Rule #3: Check file size against a limit
            if (channel.size() > MAX_FILE_SIZE) {
                System.err.println("Error: File size exceeds the maximum limit of " + MAX_FILE_SIZE + " bytes.");
                return false;
            }
            if (channel.size() == 0) {
                 System.err.println("Error: Input file is empty.");
                 return false;
            }

            // Read file content
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            ByteBuffer buffer = ByteBuffer.allocate(8192);
            while (channel.read(buffer) > 0) {
                buffer.flip();
                baos.write(buffer.array(), 0, buffer.limit());
                buffer.clear();
            }
            fileContent = baos.toByteArray();
        } catch (NoSuchFileException e) {
            System.err.println("Error: Input file not found: " + inputFile);
            return false;
        } catch (FileSystemException e) {
            System.err.println("Error: Input path is a symbolic link or not a regular file: " + e.getMessage());
            return false;
        } catch (IOException e) {
            System.err.println("Error reading input file: " + e.getMessage());
            return false;
        }

        // Base64 encode the content
        String encodedString = Base64.getEncoder().encodeToString(fileContent);

        // Rule #6: Safe and atomic write using a temporary file
        Path tempFile = null;
        try {
            String tempFileName = outputFile.getFileName().toString() + "." + UUID.randomUUID().toString() + ".tmp";
            tempFile = outputDir.resolve(tempFileName);
            
            // Set restrictive permissions on the temp file if on a POSIX system
            if (FileSystems.getDefault().supportedFileAttributeViews().contains("posix")) {
                 FileAttribute<Set<PosixFilePermission>> perms = PosixFilePermissions.asFileAttribute(PosixFilePermissions.fromString("rw-------"));
                 Files.write(tempFile, encodedString.getBytes(java.nio.charset.StandardCharsets.UTF_8), StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);
                 Files.setPosixFilePermissions(tempFile, PosixFilePermissions.fromString("rw-------"));
            } else {
                 Files.write(tempFile, encodedString.getBytes(java.nio.charset.StandardCharsets.UTF_8), StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE);
            }

            // Atomically move the temporary file to the final destination
            Files.move(tempFile, outputFile, StandardCopyOption.ATOMIC_MOVE, StandardCopyOption.REPLACE_EXISTING);
        } catch (IOException e) {
            System.err.println("Error writing output file: " + e.getMessage());
            // Clean up the temporary file if it exists
            if (tempFile != null) {
                try {
                    Files.deleteIfExists(tempFile);
                } catch (IOException ignored) {
                }
            }
            return false;
        }

        return true;
    }

    public static void main(String[] args) throws IOException {
        // Setup test environment
        String inputBase = "test_images_java";
        String outputBase = "test_output_java";
        Path inputBasePath = Paths.get(inputBase);
        Path outputBasePath = Paths.get(outputBase);

        Files.createDirectories(inputBasePath);
        Files.createDirectories(outputBasePath);

        Path validImage = inputBasePath.resolve("valid_image.jpg");
        Path largeImage = inputBasePath.resolve("large_image.jpg");
        Path symlinkImage = inputBasePath.resolve("symlink.jpg");
        Path emptyImage = inputBasePath.resolve("empty.jpg");

        // Create test files
        Files.write(validImage, new byte[]{1, 2, 3, 4, 5});
        Files.createFile(emptyImage);
        try (OutputStream os = Files.newOutputStream(largeImage)) {
            byte[] chunk = new byte[1024];
            for (int i = 0; i < (MAX_FILE_SIZE / 1024) + 1; i++) {
                os.write(chunk);
            }
        }
        try {
            Files.createSymbolicLink(symlinkImage, validImage);
        } catch (UnsupportedOperationException | IOException e) {
            System.out.println("Skipping symlink test: Not supported on this system.");
            symlinkImage = null; // Mark as unavailable
        }

        // Test Cases
        System.out.println("--- Running Java Test Cases ---");

        // 1. Valid upload
        System.out.print("1. Testing valid upload... ");
        boolean success = upload_image(inputBase, "valid_image.jpg", outputBase, "encoded.txt");
        System.out.println(success ? "SUCCESS" : "FAILED");
        assert success;

        // 2. File too large
        System.out.print("2. Testing file too large... ");
        success = upload_image(inputBase, "large_image.jpg", outputBase, "encoded_large.txt");
        System.out.println(!success ? "SUCCESS (Rejected as expected)" : "FAILED");
        assert !success;

        // 3. Symbolic link
        if (symlinkImage != null) {
            System.out.print("3. Testing symbolic link... ");
            success = upload_image(inputBase, "symlink.jpg", outputBase, "encoded_symlink.txt");
            System.out.println(!success ? "SUCCESS (Rejected as expected)" : "FAILED");
            assert !success;
        }

        // 4. Path traversal
        System.out.print("4. Testing path traversal... ");
        success = upload_image(inputBase, "../test_traversal.jpg", outputBase, "encoded_traversal.txt");
        System.out.println(!success ? "SUCCESS (Rejected as expected)" : "FAILED");
        assert !success;

        // 5. Non-existent file
        System.out.print("5. Testing non-existent file... ");
        success = upload_image(inputBase, "not_real.jpg", outputBase, "encoded_not_real.txt");
        System.out.println(!success ? "SUCCESS (Rejected as expected)" : "FAILED");
        assert !success;

        // Cleanup
        Files.deleteIfExists(validImage);
        Files.deleteIfExists(largeImage);
        if (symlinkImage != null) Files.deleteIfExists(symlinkImage);
        Files.deleteIfExists(emptyImage);
        Files.deleteIfExists(inputBasePath);
        Path testOutputFile = outputBasePath.resolve("encoded.txt");
        if(Files.exists(testOutputFile)) Files.delete(testOutputFile);
        Files.deleteIfExists(outputBasePath);
        System.out.println("--- Java Tests Complete ---");
    }
}