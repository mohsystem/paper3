import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.NoSuchFileException;
import java.util.Arrays;
import java.util.List;

public class Lse44 {

    // The base directory from which files can be read.
    // Using a relative path for easier testing without root permissions.
    private static final String SAFE_DIRECTORY = "safe";

    /**
     * Securely reads a file from a designated 'safe' directory.
     * It prevents directory traversal attacks by normalizing the path and
     * ensuring the resolved path is still within the safe directory.
     *
     * @param filename The name of the file to read. Must not contain path separators.
     * @return The content of the file as a String, or an error message.
     */
    public static String readFileFromSafeDirectory(String filename) {
        // Validation 1: Reject null, empty, or filenames containing path separators.
        // This is a first-line defense against path traversal.
        if (filename == null || filename.trim().isEmpty() || filename.contains("/") || filename.contains("\\")) {
            return "Error: Invalid filename. It must not contain path separators.";
        }

        try {
            Path safeDir = Paths.get(SAFE_DIRECTORY).toRealPath();
            Path requestedFile = safeDir.resolve(filename);

            // Validation 2: Canonicalize the path and verify it's a subpath of the safe directory.
            // toRealPath() resolves '..', '.', and symbolic links.
            Path canonicalPath = requestedFile.toRealPath();

            if (!canonicalPath.startsWith(safeDir)) {
                return "Error: Directory traversal attempt detected.";
            }
            
            // Ensure we are reading a regular file.
            if (!Files.isRegularFile(canonicalPath)) {
                return "Error: Path does not point to a regular file.";
            }

            return new String(Files.readAllBytes(canonicalPath));
        } catch (NoSuchFileException e) {
            return "Error: File not found.";
        } catch (IOException e) {
            return "Error: Cannot read file. " + e.getMessage();
        } catch (SecurityException e) {
            return "Error: Security violation while accessing file. " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        // --- Test Environment Setup ---
        Path safeDir = Paths.get(SAFE_DIRECTORY);
        Path testFile = safeDir.resolve("document1.txt");
        try {
            Files.createDirectories(safeDir);
            Files.write(testFile, "This is a safe document.".getBytes());
            System.out.println("Test environment created.");
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }

        // --- Test Cases ---
        List<String> testFilenames = Arrays.asList(
            "document1.txt",                          // 1. Valid file
            "nonexistent.txt",                        // 2. File not found
            "../Lse44.java",                          // 3. Simple traversal attempt
            "document1.txt/../../Lse44.java",         // 4. Nested traversal attempt
            "/etc/passwd"                             // 5. Absolute path attempt
        );

        System.out.println("\n--- Running Test Cases ---");
        for (int i = 0; i < testFilenames.size(); i++) {
            String filename = testFilenames.get(i);
            System.out.printf("Test Case %d: Requesting \"%s\"\n", i + 1, filename);
            String result = readFileFromSafeDirectory(filename);
            System.out.println("Result: " + result);
            System.out.println("--------------------");
        }

        // --- Test Environment Teardown ---
        try {
            Files.delete(testFile);
            Files.delete(safeDir);
            System.out.println("\nTest environment cleaned up.");
        } catch (IOException e) {
            System.err.println("Failed to clean up test environment: " + e.getMessage());
        }
    }
}