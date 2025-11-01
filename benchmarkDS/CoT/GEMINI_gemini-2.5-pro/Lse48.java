import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;

public class Lse48 {

    // A predefined base directory to prevent access to other parts of the filesystem.
    private static final String IMAGE_BASE_DIRECTORY = "test_images";

    /**
     * Simulates fetching an image file based on a 'file' parameter.
     * This function securely reads a file from a predefined base directory,
     * preventing path traversal attacks.
     *
     * @param fileName The name of the file requested. This should be a simple
     *                 filename without any path components.
     * @return A byte array containing the file's data, or null if the file
     *         cannot be read or the request is invalid/unsafe.
     */
    public static byte[] img(String fileName) {
        if (fileName == null || fileName.isEmpty()) {
            System.err.println("Error: File name is null or empty.");
            return null;
        }

        // Normalize the base directory path
        Path baseDir = Paths.get(IMAGE_BASE_DIRECTORY).toAbsolutePath();

        // Create a path for the requested file and resolve it against the base directory.
        // The `getFileName()` part is a crucial security step to strip any directory info from the input.
        Path requestedFile = Paths.get(fileName).getFileName();
        Path finalPath = baseDir.resolve(requestedFile).normalize();

        // Security Check: Ensure the resolved path is still inside the base directory.
        // This is the primary defense against path traversal attacks (e.g., "../").
        if (!finalPath.startsWith(baseDir)) {
            System.err.println("Security Error: Path traversal attempt detected for file: " + fileName);
            return null;
        }

        // Check if the file exists and is a regular file.
        if (!Files.exists(finalPath) || !Files.isRegularFile(finalPath)) {
            System.err.println("Error: File not found or is not a regular file: " + finalPath);
            return null;
        }

        try {
            // Read all bytes from the file.
            return Files.readAllBytes(finalPath);
        } catch (IOException e) {
            System.err.println("Error reading file " + finalPath + ": " + e.getMessage());
            return null;
        }
    }

    // Main method with test cases
    public static void main(String[] args) {
        // --- Test Setup ---
        try {
            Path baseDir = Paths.get(IMAGE_BASE_DIRECTORY);
            Files.createDirectories(baseDir);
            Files.write(baseDir.resolve("test1.jpg"), "ImageData1".getBytes());
            Files.write(Paths.get("secret.txt"), "SecretData".getBytes());
            System.out.println("Test environment set up.");
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }

        System.out.println("\n--- Running Test Cases ---");

        // Test Case 1: Valid file
        System.out.println("Test 1: Requesting a valid file 'test1.jpg'");
        byte[] data1 = img("test1.jpg");
        if (data1 != null && new String(data1).equals("ImageData1")) {
            System.out.println("  Result: SUCCESS, content matches.");
        } else {
            System.out.println("  Result: FAILED.");
        }

        // Test Case 2: Non-existent file
        System.out.println("\nTest 2: Requesting a non-existent file 'notfound.jpg'");
        byte[] data2 = img("notfound.jpg");
        if (data2 == null) {
            System.out.println("  Result: SUCCESS, function returned null as expected.");
        } else {
            System.out.println("  Result: FAILED.");
        }

        // Test Case 3: Path traversal attempt
        System.out.println("\nTest 3: Path traversal attempt '../secret.txt'");
        byte[] data3 = img("../secret.txt");
        if (data3 == null) {
            System.out.println("  Result: SUCCESS, path traversal blocked.");
        } else {
            System.out.println("  Result: FAILED.");
        }

        // Test Case 4: Path traversal with nested directories
        System.out.println("\nTest 4: Path traversal attempt '..\\secret.txt'");
        byte[] data4 = img("..\\secret.txt");
         if (data4 == null) {
            System.out.println("  Result: SUCCESS, path traversal blocked.");
        } else {
            System.out.println("  Result: FAILED.");
        }

        // Test Case 5: Absolute path attempt (on Unix-like systems)
        System.out.println("\nTest 5: Absolute path attempt '/etc/passwd'");
        byte[] data5 = img("/etc/passwd");
         if (data5 == null) {
            System.out.println("  Result: SUCCESS, absolute path blocked.");
        } else {
            System.out.println("  Result: FAILED.");
        }


        // --- Test Cleanup ---
        try {
            Path baseDir = Paths.get(IMAGE_BASE_DIRECTORY);
            Files.deleteIfExists(baseDir.resolve("test1.jpg"));
            Files.deleteIfExists(baseDir);
            Files.deleteIfExists(Paths.get("secret.txt"));
            System.out.println("\n--- Test environment cleaned up. ---");
        } catch (IOException e) {
            System.err.println("Failed to clean up test environment: " + e.getMessage());
        }
    }
}