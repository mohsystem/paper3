import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Optional;

public class Lse45 {

    private static final String SAFE_DIRECTORY_NAME = "safe_dir";

    /**
     * Securely reads a file from a predefined safe directory.
     * It prevents path traversal attacks by canonicalizing paths and verifying
     * the requested file is strictly within the safe directory.
     *
     * @param filename The name of the file to read.
     * @return An Optional containing the file content as a String if successful,
     *         otherwise an empty Optional.
     */
    public static Optional<String> readFileFromSafeDirectory(String filename) {
        if (filename == null || filename.isEmpty()) {
            System.err.println("Error: Filename cannot be null or empty.");
            return Optional.empty();
        }

        // Prevent null byte injection
        if (filename.indexOf('\0') != -1) {
            System.err.println("Error: Filename contains null byte.");
            return Optional.empty();
        }

        try {
            Path safeDirPath = Paths.get(SAFE_DIRECTORY_NAME).toRealPath();
            Path targetFilePath = safeDirPath.resolve(filename).normalize();

            // The crucial security check: ensure the resolved path is still inside the safe directory.
            if (!targetFilePath.startsWith(safeDirPath)) {
                System.err.println("Security Error: Path traversal attempt detected for file: " + filename);
                return Optional.empty();
            }

            if (!Files.isRegularFile(targetFilePath) || !Files.isReadable(targetFilePath)) {
                 System.err.println("Error: File is not a regular, readable file: " + filename);
                 return Optional.empty();
            }

            return Optional.of(new String(Files.readAllBytes(targetFilePath)));
        } catch (IOException e) {
            System.err.println("Error reading file '" + filename + "': " + e.getMessage());
            return Optional.empty();
        } catch (Exception e) {
            System.err.println("An unexpected error occurred for file '" + filename + "': " + e.getMessage());
            return Optional.empty();
        }
    }

    private static void setupTestEnvironment() {
        System.out.println("--- Setting up test environment ---");
        try {
            // Create safe directory
            Path safeDir = Paths.get(SAFE_DIRECTORY_NAME);
            Files.createDirectories(safeDir);

            // Create a subdirectory
            Path subDir = safeDir.resolve("subdir");
            Files.createDirectories(subDir);

            // Create valid files
            Files.write(safeDir.resolve("test1.txt"), "This is a safe file.".getBytes());
            Files.write(subDir.resolve("test2.txt"), "This is another safe file in a subdirectory.".getBytes());

            // Create a file outside the safe directory for traversal tests
            Files.write(Paths.get("secret.txt"), "This is a secret file.".getBytes());
            System.out.println("Setup complete.");
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
        }
    }

    private static void cleanupTestEnvironment() {
        System.out.println("\n--- Cleaning up test environment ---");
        try {
            // Recursive delete for safe_dir
            Path safeDirPath = Paths.get(SAFE_DIRECTORY_NAME);
            Files.walk(safeDirPath)
                 .sorted((p1, p2) -> -p1.compareTo(p2)) // reverse order for deletion
                 .forEach(p -> {
                     try {
                         Files.delete(p);
                     } catch (IOException e) {
                         // Ignored
                     }
                 });
            
            Files.deleteIfExists(Paths.get("secret.txt"));
            System.out.println("Cleanup complete.");
        } catch (IOException e) {
            System.err.println("Failed to clean up test environment: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        setupTestEnvironment();
        System.out.println("\n--- Running Test Cases ---");

        // Test Case 1: Read a valid file
        System.out.println("\n[Test 1] Reading a valid file 'test1.txt'");
        readFileFromSafeDirectory("test1.txt")
            .ifPresentOrElse(content -> System.out.println("SUCCESS. Content: " + content),
                             () -> System.out.println("FAILED."));

        // Test Case 2: Read a non-existent file
        System.out.println("\n[Test 2] Reading a non-existent file 'nonexistent.txt'");
        readFileFromSafeDirectory("nonexistent.txt")
            .ifPresentOrElse(content -> System.out.println("SUCCESS. Content: " + content),
                             () -> System.out.println("FAILED. (Expected)"));

        // Test Case 3: Path traversal attempt
        System.out.println("\n[Test 3] Path traversal attempt '../secret.txt'");
        readFileFromSafeDirectory("../secret.txt")
            .ifPresentOrElse(content -> System.out.println("SUCCESS. Content: " + content),
                             () -> System.out.println("FAILED. (Expected Security Block)"));

        // Test Case 4: Path traversal from a subdirectory
        System.out.println("\n[Test 4] Path traversal from subdirectory 'subdir/../../secret.txt'");
        readFileFromSafeDirectory("subdir/../../secret.txt")
            .ifPresentOrElse(content -> System.out.println("SUCCESS. Content: " + content),
                             () -> System.out.println("FAILED. (Expected Security Block)"));

        // Test Case 5: Reading a valid file in a subdirectory
        System.out.println("\n[Test 5] Reading a valid file in a subdirectory 'subdir/test2.txt'");
        readFileFromSafeDirectory("subdir/test2.txt")
            .ifPresentOrElse(content -> System.out.println("SUCCESS. Content: " + content),
                             () -> System.out.println("FAILED."));

        cleanupTestEnvironment();
    }
}