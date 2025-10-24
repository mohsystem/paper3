import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.*;
import java.util.UUID;

public class Task121 {

    private static final int BUFFER_SIZE = 4096;

    /**
     * Securely "uploads" a file by copying it from a source path to a destination directory.
     * This implementation prevents path traversal and uses an atomic move to mitigate TOCTOU vulnerabilities.
     *
     * @param sourceFilePath  The path to the file to be uploaded.
     * @param destinationDir  The directory where the file will be saved.
     * @param newFileName     The name for the new file in the destination directory. Must not contain path separators.
     * @return true on success, false on failure.
     */
    public static boolean uploadFile(String sourceFilePath, String destinationDir, String newFileName) {
        // Rule #6: Validate inputs
        if (sourceFilePath == null || destinationDir == null || newFileName == null ||
            sourceFilePath.isEmpty() || destinationDir.isEmpty() || newFileName.isEmpty()) {
            System.err.println("Error: Input paths cannot be null or empty.");
            return false;
        }

        // Rule #9: Prevent path traversal by validating the filename.
        Path fileNamePath = Paths.get(newFileName);
        if (fileNamePath.getParent() != null || !fileNamePath.getFileName().toString().equals(newFileName)) {
            System.err.println("Error: Invalid filename. It must not contain path elements.");
            return false;
        }

        Path sourcePath = Paths.get(sourceFilePath);
        Path destDirPath = Paths.get(destinationDir);
        Path tempFilePath = null;

        try {
            // Further validation of paths
            if (!Files.isRegularFile(sourcePath)) {
                System.err.println("Error: Source file does not exist or is not a regular file.");
                return false;
            }
            if (!Files.isDirectory(destDirPath)) {
                System.err.println("Error: Destination directory does not exist or is not a directory.");
                return false;
            }

            // Rule #9: Resolve paths to prevent traversal attacks (e.g., using symlinks or '..')
            Path canonicalDestDir = destDirPath.toRealPath();
            Path finalDestPath = canonicalDestDir.resolve(fileNamePath);

            // Double check that the final path's parent is the intended directory.
            if (!finalDestPath.getParent().equals(canonicalDestDir)) {
                 System.err.println("Error: Path traversal attempt detected.");
                 return false;
            }

            // Rule #3: Mitigate TOCTOU by writing to a temporary file and then moving atomically.
            // Create a temporary file in the destination directory.
            String tempFileName = ".tmp_" + UUID.randomUUID().toString();
            tempFilePath = canonicalDestDir.resolve(tempFileName);

            try (InputStream in = Files.newInputStream(sourcePath);
                 OutputStream out = Files.newOutputStream(tempFilePath, StandardOpenOption.CREATE_NEW, StandardOpenOption.WRITE)) {
                
                // Rule #1: Use a buffer and check read bytes to prevent overflow issues
                byte[] buffer = new byte[BUFFER_SIZE];
                int bytesRead;
                while ((bytesRead = in.read(buffer)) != -1) {
                    out.write(buffer, 0, bytesRead);
                }
            }

            // Atomically move the temporary file to the final destination.
            // This fails if the file already exists, which is a safe default.
            Files.move(tempFilePath, finalDestPath, StandardCopyOption.ATOMIC_MOVE);
            System.out.println("File uploaded successfully to: " + finalDestPath);
            return true;

        } catch (IOException e) {
            // Rule #15: Catch and handle exceptions
            System.err.println("An I/O error occurred: " + e.getMessage());
        } catch (SecurityException e) {
            System.err.println("A security error occurred: " + e.getMessage());
        } finally {
            // Cleanup the temporary file in case of failure
            if (tempFilePath != null) {
                try {
                    Files.deleteIfExists(tempFilePath);
                } catch (IOException e) {
                    System.err.println("Failed to delete temporary file: " + e.getMessage());
                }
            }
        }
        return false;
    }
    
    public static void main(String[] args) throws IOException {
        // Setup test environment
        Path testRoot = Files.createTempDirectory("test_root_java");
        Path sourceDir = Files.createDirectory(testRoot.resolve("sources"));
        Path uploadDir = Files.createDirectory(testRoot.resolve("uploads"));
        Path sourceFile = sourceDir.resolve("testfile.txt");
        Files.write(sourceFile, "This is a test file.".getBytes());

        System.out.println("Test environment created in: " + testRoot);
        System.out.println("Source file: " + sourceFile);
        System.out.println("Upload directory: " + uploadDir);
        System.out.println("------------------------------------------");

        // Test Case 1: Successful upload
        System.out.println("Test Case 1: Successful upload");
        boolean success1 = uploadFile(sourceFile.toString(), uploadDir.toString(), "newfile.txt");
        System.out.println("Result: " + (success1 ? "SUCCESS" : "FAILURE"));
        System.out.println("------------------------------------------");

        // Test Case 2: Path traversal attempt
        System.out.println("Test Case 2: Path traversal attempt with '../evil.txt'");
        boolean success2 = uploadFile(sourceFile.toString(), uploadDir.toString(), "../evil.txt");
        System.out.println("Result: " + (success2 ? "SUCCESS" : "FAILURE"));
        System.out.println("------------------------------------------");

        // Test Case 3: Source file does not exist
        System.out.println("Test Case 3: Source file does not exist");
        boolean success3 = uploadFile("nonexistent.txt", uploadDir.toString(), "anotherfile.txt");
        System.out.println("Result: " + (success3 ? "SUCCESS" : "FAILURE"));
        System.out.println("------------------------------------------");

        // Test Case 4: Destination directory does not exist
        System.out.println("Test Case 4: Destination directory does not exist");
        boolean success4 = uploadFile(sourceFile.toString(), uploadDir.toString() + "/nonexistent", "anotherfile.txt");
        System.out.println("Result: " + (success4 ? "SUCCESS" : "FAILURE"));
        System.out.println("------------------------------------------");

        // Test Case 5: Filename contains a subdirectory
        System.out.println("Test Case 5: Filename contains a subdirectory 'subdir/file.txt'");
        boolean success5 = uploadFile(sourceFile.toString(), uploadDir.toString(), "subdir/file.txt");
        System.out.println("Result: " + (success5 ? "SUCCESS" : "FAILURE"));
        System.out.println("------------------------------------------");

        // Cleanup
        Files.deleteIfExists(uploadDir.resolve("newfile.txt"));
        Files.delete(uploadDir);
        Files.delete(sourceFile);
        Files.delete(sourceDir);
        Files.delete(testRoot);
    }
}