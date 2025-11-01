import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Comparator;

public class Lse43 {

    // Define the base directory. For testing, we use a temporary directory.
    // In a real application, this would be a fixed path like "/safe/".
    private static final String SAFE_DIR_PATH = "test_root/safe/";

    /**
     * Reads a file from a designated "safe" directory, preventing path traversal.
     *
     * @param filename The name of the file to read. Must not contain path separators.
     * @return The content of the file as a String, or an error message.
     */
    public static String readFileFromSafeDirectory(String filename) {
        // 1. Initial validation: Filename should not contain path separators.
        if (filename.contains("/") || filename.contains("\\")) {
            return "Error: Invalid characters in filename. Path traversal attempt rejected.";
        }

        File safeDir = new File(SAFE_DIR_PATH);
        File targetFile = new File(safeDir, filename);

        try {
            // 2. Canonical path validation: The most robust way to prevent traversal.
            // It resolves symbolic links, '..', '.', etc.
            String canonicalSafePath = safeDir.getCanonicalPath();
            String canonicalTargetPath = targetFile.getCanonicalPath();

            // Ensure the target file's path is genuinely inside the safe directory.
            if (!canonicalTargetPath.startsWith(canonicalSafePath)) {
                return "Error: Path traversal attempt detected.";
            }

            // 3. Read the file if validation passes.
            // Using Files.readString for simplicity (Java 11+).
            return Files.readString(Paths.get(canonicalTargetPath));

        } catch (IOException e) {
            // This can be a FileNotFoundException, AccessDeniedException, etc.
            return "Error: Could not read file. " + e.getMessage();
        }
    }

    // Main method with test cases
    public static void main(String[] args) {
        // --- Test Environment Setup ---
        System.out.println("--- Setting up test environment ---");
        try {
            // Create a temporary directory structure for testing
            Path testRoot = Paths.get("test_root");
            if (Files.exists(testRoot)) {
                // Clean up previous run if necessary
                Files.walk(testRoot)
                     .sorted(Comparator.reverseOrder())
                     .map(Path::toFile)
                     .forEach(File::delete);
            }
            Files.createDirectories(Paths.get(SAFE_DIR_PATH));
            Files.writeString(Paths.get(SAFE_DIR_PATH, "good.txt"), "This is a safe file.");
            Files.writeString(Paths.get(SAFE_DIR_PATH, "another.txt"), "This is another safe file.");
            Files.writeString(Paths.get("test_root/outside.txt"), "This file is outside the safe directory.");
            System.out.println("Setup complete.");
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }
        System.out.println("-------------------------------------\n");


        String[] testFilenames = {
            "good.txt",                      // Test Case 1: Valid file
            "another.txt",                   // Test Case 2: Another valid file
            "nonexistent.txt",               // Test Case 3: File does not exist
            "../outside.txt",                // Test Case 4: Path traversal attempt
            "good.txt/../../outside.txt"     // Test Case 5: Complex path traversal attempt
        };

        for (int i = 0; i < testFilenames.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": Attempting to read '" + testFilenames[i] + "'");
            String result = readFileFromSafeDirectory(testFilenames[i]);
            System.out.println("Result: " + result + "\n");
        }
        
        // --- Cleanup ---
        try {
            Path testRoot = Paths.get("test_root");
            Files.walk(testRoot)
                 .sorted(Comparator.reverseOrder())
                 .map(Path::toFile)
                 .forEach(File::delete);
            // System.out.println("Test environment cleaned up.");
        } catch(IOException e) {
            // System.err.println("Cleanup failed: " + e.getMessage());
        }
    }
}