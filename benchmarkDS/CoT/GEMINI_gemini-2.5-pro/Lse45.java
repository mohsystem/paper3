import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.util.stream.Stream;

public class Lse45 {

    // Define the safe directory. In a real application, this should be an absolute path.
    private static final String SAFE_DIRECTORY = "safe_dir";

    /**
     * Reads a file from a predefined safe directory.
     * This function prevents path traversal attacks by resolving the canonical path
     * of the requested file and ensuring it is a descendant of the safe directory.
     *
     * @param filename The name of the file to read. Subdirectories are allowed.
     * @return The content of the file as a String.
     * @throws IOException If there is an error reading the file.
     * @throws SecurityException If a path traversal attack is detected.
     */
    public static String readFileFromSafeDirectory(String filename) throws IOException, SecurityException {
        // Create Path objects for the safe directory and the target file.
        Path safeDirPath = Paths.get(SAFE_DIRECTORY).toAbsolutePath().normalize();
        Path targetFilePath = safeDirPath.resolve(filename).toAbsolutePath().normalize();

        // Security Check: Verify that the resolved target path starts with the safe directory path.
        // This is the core of the path traversal prevention.
        if (!targetFilePath.startsWith(safeDirPath)) {
            throw new SecurityException("Access Denied: Path traversal attempt detected for file: " + filename);
        }

        // Check if the file exists and is a regular file before reading.
        if (!Files.exists(targetFilePath) || !Files.isRegularFile(targetFilePath)) {
             throw new IOException("File not found or is not a regular file: " + filename);
        }

        return Files.readString(targetFilePath);
    }

    public static void main(String[] args) {
        // --- Test Setup ---
        try {
            System.out.println("--- Setting up test environment ---");
            Path safeDir = Paths.get(SAFE_DIRECTORY);
            // Clean up previous runs
            if (Files.exists(safeDir)) {
                try (Stream<Path> walk = Files.walk(safeDir)) {
                    walk.sorted(java.util.Comparator.reverseOrder())
                        .map(Path::toFile)
                        .forEach(File::delete);
                }
            }
            Files.createDirectories(safeDir.resolve("sub"));
            Files.writeString(safeDir.resolve("good_file.txt"), "This is a safe file.", StandardOpenOption.CREATE);
            Files.writeString(safeDir.resolve("sub").resolve("nested_file.txt"), "This is a nested safe file.", StandardOpenOption.CREATE);
            
            Path evilFile = Paths.get("evil.txt");
            Files.writeString(evilFile, "This file is outside the safe directory.", StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING);
            System.out.println("Setup complete.\n");

        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }

        // --- Test Cases ---
        String[] testCases = {
            "good_file.txt",                          // 1. Should succeed
            "sub/nested_file.txt",                    // 2. Should succeed (subdirectory)
            "non_existent.txt",                       // 3. Should fail (not found)
            "../evil.txt",                            // 4. Should fail (path traversal)
            "sub/../../evil.txt"                      // 5. Should fail (complex path traversal)
        };

        System.out.println("--- Running Test Cases ---");
        for (String testCase : testCases) {
            System.out.println("Attempting to read: '" + testCase + "'");
            try {
                String content = readFileFromSafeDirectory(testCase);
                System.out.println("  [SUCCESS] Content: \"" + content + "\"");
            } catch (IOException | SecurityException e) {
                System.out.println("  [FAILURE] " + e.getClass().getSimpleName() + ": " + e.getMessage());
            }
            System.out.println("---------------------------------");
        }
        
        // --- Cleanup ---
        try {
            Files.deleteIfExists(Paths.get("evil.txt"));
        } catch(IOException e) {
             System.err.println("Cleanup failed for evil.txt: " + e.getMessage());
        }
    }
}