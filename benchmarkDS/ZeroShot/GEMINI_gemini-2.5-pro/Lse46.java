import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Arrays;

public class Lse46 {

    /**
     * Securely retrieves an image file from a designated 'images' directory.
     * This function simulates a web endpoint handler for a URL like /img?name=...
     * It prevents path traversal attacks by validating the filename and ensuring
     * the resolved file path is within the intended directory.
     *
     * @param name The name of the file requested from the 'images' directory.
     * @return A byte array containing the file's content, or null if the file is
     *         not found, is a directory, or if a security violation is detected.
     */
    public static byte[] img(String name) {
        // 1. Basic validation: reject null, empty, or names containing path separators.
        if (name == null || name.trim().isEmpty() || name.contains("/") || name.contains("\\") || name.contains("..")) {
            System.err.println("Error: Invalid characters or path traversal attempt in filename '" + name + "'.");
            return null;
        }

        try {
            // 2. Define the secure base directory.
            Path imageDir = Paths.get("images").toAbsolutePath();
            if (!Files.exists(imageDir)) {
                 System.err.println("Error: Base 'images' directory does not exist.");
                 return null;
            }

            // 3. Resolve the requested file path against the base directory.
            Path requestedFile = imageDir.resolve(name).normalize();

            // 4. Security Check: Ensure the resolved path is still within the image directory.
            // This is the crucial step to prevent path traversal attacks (e.g., ../../etc/passwd)
            if (!requestedFile.startsWith(imageDir)) {
                System.err.println("Security Error: Path traversal detected for filename '" + name + "'.");
                return null;
            }

            // 5. Check if the file exists and is a regular file.
            if (Files.exists(requestedFile) && Files.isRegularFile(requestedFile)) {
                return Files.readAllBytes(requestedFile);
            } else {
                System.err.println("Error: File not found or is not a regular file: " + requestedFile);
                return null;
            }

        } catch (IOException e) {
            System.err.println("Error reading file: " + e.getMessage());
            return null;
        } catch (Exception e) {
            System.err.println("An unexpected error occurred: " + e.getMessage());
            return null;
        }
    }

    // Main method with test cases
    public static void main(String[] args) {
        // Setup: Create a dummy directory and files for testing
        File imageDir = new File("images");
        File validImage = new File(imageDir, "test.jpg");
        File secretFile = new File("secret.txt");

        try {
            imageDir.mkdir();
            new PrintWriter(validImage).println("jpeg_data").close();
            new PrintWriter(secretFile).println("secret_data").close();
            System.out.println("Test environment created.");
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }

        System.out.println("\n--- Running Test Cases ---");

        String[] testCases = {
            "test.jpg",                     // 1. Valid case
            "../secret.txt",                // 2. Path traversal
            "nonexistent.png",              // 3. File not found
            "test.jpg/../test.jpg",         // 4. More complex invalid name
            "../../../../etc/passwd"        // 5. Classic path traversal attack
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\nTest Case " + (i + 1) + ": Requesting '" + testCases[i] + "'");
            byte[] result = img(testCases[i]);
            if (result != null) {
                System.out.println("Success: Returned " + result.length + " bytes. Content: " + new String(result).trim());
            } else {
                System.out.println("Failure: Request denied as expected.");
            }
        }

        // Teardown: Clean up the created files and directory
        validImage.delete();
        imageDir.delete();
        secretFile.delete();
        System.out.println("\n--- Test environment cleaned up. ---");
    }
}