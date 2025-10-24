import java.io.File;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.file.AccessDeniedException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.UUID;

public class Task127 {

    /**
     * Securely reads a file from a specified base directory and writes it to a temporary location.
     * Prevents path traversal attacks.
     *
     * @param baseDirStr The trusted base directory from which to read files.
     * @param fileName   The name of the file to read, relative to the base directory.
     * @return The path to the created temporary file, or null on failure.
     */
    public static String copyFileToTemp(String baseDirStr, String fileName) {
        if (fileName == null || fileName.trim().isEmpty()) {
            System.err.println("Error: File name cannot be null or empty.");
            return null;
        }

        Path baseDir = Paths.get(baseDirStr).toAbsolutePath();
        Path sourcePath = baseDir.resolve(fileName).toAbsolutePath().normalize();

        // Security Check: Ensure the resolved path is still within the base directory.
        if (!sourcePath.startsWith(baseDir)) {
            System.err.println("Error: Path traversal attempt detected for file: " + fileName);
            return null;
        }

        try {
            // Securely create a temporary file
            Path tempFile = Files.createTempFile("temp_copy_", ".tmp");

            // Copy the contents. try-with-resources is not needed as Files.copy handles streams.
            Files.copy(sourcePath, tempFile, StandardCopyOption.REPLACE_EXISTING);

            System.out.println("Successfully copied '" + sourcePath + "' to '" + tempFile.toString() + "'");
            return tempFile.toString();
        } catch (java.nio.file.NoSuchFileException e) {
            System.err.println("Error: Source file not found: " + sourcePath);
        } catch (AccessDeniedException e) {
            System.err.println("Error: Permission denied when accessing: " + sourcePath);
        } catch (IOException e) {
            System.err.println("An I/O error occurred: " + e.getMessage());
        } catch (Exception e) {
            System.err.println("An unexpected error occurred: " + e.getMessage());
        }

        return null;
    }

    public static void main(String[] args) {
        // --- Setup Test Environment ---
        String testDirName = "java_test_dir_" + UUID.randomUUID().toString().substring(0, 8);
        Path testDirPath = Paths.get(testDirName);
        Path goodFilePath = testDirPath.resolve("good_file.txt");
        Path emptyFilePath = testDirPath.resolve("empty_file.txt");
        Path unreadableFilePath = testDirPath.resolve("unreadable.txt");
        
        try {
            Files.createDirectories(testDirPath);
            Files.write(goodFilePath, "This is a test file.".getBytes());
            Files.createFile(emptyFilePath);
            Files.createFile(unreadableFilePath);
            File unreadableFile = unreadableFilePath.toFile();
            if(!unreadableFile.setReadable(false)){
                 System.out.println("Warning: Could not make test file unreadable. Permission test might not work as expected.");
            }

            System.out.println("--- Running Test Cases ---");

            // Test Case 1: Happy path - valid file
            System.out.println("\n1. Testing with a valid file:");
            String temp1 = copyFileToTemp(testDirName, "good_file.txt");
            if (temp1 != null) {
                // Clean up the created temp file
                new File(temp1).delete();
            }

            // Test Case 2: File not found
            System.out.println("\n2. Testing with a non-existent file:");
            copyFileToTemp(testDirName, "non_existent_file.txt");

            // Test Case 3: Path traversal attack
            System.out.println("\n3. Testing path traversal attack:");
            // Using a relative path that tries to go up one directory
            copyFileToTemp(testDirName, "../some_other_file.txt");

            // Test Case 4: Empty file
            System.out.println("\n4. Testing with an empty file:");
            String temp4 = copyFileToTemp(testDirName, "empty_file.txt");
            if (temp4 != null) {
                new File(temp4).delete();
            }
            
            // Test Case 5: Permission denied (read)
            System.out.println("\n5. Testing with an unreadable file:");
            copyFileToTemp(testDirName, "unreadable.txt");
            
            System.out.println("\n--- Test Cases Finished ---");

        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
        } finally {
            // --- Cleanup Test Environment ---
            try {
                // Make file writable again to delete it
                unreadableFilePath.toFile().setReadable(true);
                Files.deleteIfExists(goodFilePath);
                Files.deleteIfExists(emptyFilePath);
                Files.deleteIfExists(unreadableFilePath);
                Files.deleteIfExists(testDirPath);
                System.out.println("\nTest environment cleaned up.");
            } catch (IOException e) {
                System.err.println("Failed to clean up test environment: " + e.getMessage());
            }
        }
    }
}