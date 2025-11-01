import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.stream.Stream;

public class Lse45 {

    private static final String SAFE_DIR = "safe_dir";

    /**
     * Reads the content of a file from a predefined safe directory.
     * Prevents directory traversal attacks.
     *
     * @param filename The name of the file to read.
     * @return The content of the file as a String, or an error message.
     */
    public static String readFileFromSafeDir(String filename) {
        if (filename == null || filename.isEmpty()) {
            return "Error: Filename cannot be null or empty.";
        }

        Path safeDirPath = Paths.get(SAFE_DIR).toAbsolutePath();
        Path requestedFilePath = safeDirPath.resolve(filename).toAbsolutePath();

        // Security Check: Ensure the resolved path is still inside the safe directory.
        if (!requestedFilePath.normalize().startsWith(safeDirPath)) {
            return "Error: Directory traversal attempt detected.";
        }
        
        // Security Check: Ensure filename is just a plain name, not a path
        if (Paths.get(filename).getParent() != null) {
             return "Error: Invalid characters in filename.";
        }

        if (!Files.exists(requestedFilePath)) {
            return "Error: File not found: " + filename;
        }

        if (Files.isDirectory(requestedFilePath)) {
            return "Error: Requested path is a directory, not a file.";
        }

        try {
            return Files.readString(requestedFilePath);
        } catch (IOException e) {
            return "Error: Could not read file: " + e.getMessage();
        }
    }

    /**
     * Sets up a test environment with a safe directory and some files.
     */
    private static void setupTestEnvironment() {
        try {
            Path safeDir = Paths.get(SAFE_DIR);
            if (Files.exists(safeDir)) { // Clean up previous runs
                try (Stream<Path> walk = Files.walk(safeDir)) {
                    walk.sorted(java.util.Comparator.reverseOrder())
                        .map(Path::toFile)
                        .forEach(File::delete);
                }
            }
            Files.createDirectories(safeDir);
            Files.writeString(safeDir.resolve("file1.txt"), "This is file 1 from Java.");
            Files.writeString(safeDir.resolve("file2.txt"), "Content of the second file.");
            System.out.println("Test environment created in './" + SAFE_DIR + "'");
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        setupTestEnvironment();
        System.out.println("\n--- Running Java Test Cases ---");

        // Test Case 1: Read a valid file
        System.out.println("1. Reading 'file1.txt':");
        System.out.println("   Result: " + readFileFromSafeDir("file1.txt"));
        System.out.println();

        // Test Case 2: Read another valid file
        System.out.println("2. Reading 'file2.txt':");
        System.out.println("   Result: " + readFileFromSafeDir("file2.txt"));
        System.out.println();

        // Test Case 3: Attempt to read a non-existent file
        System.out.println("3. Reading 'nonexistent.txt':");
        System.out.println("   Result: " + readFileFromSafeDir("nonexistent.txt"));
        System.out.println();

        // Test Case 4: Attempt directory traversal (simple)
        System.out.println("4. Attempting to read '../some_file.txt':");
        System.out.println("   Result: " + readFileFromSafeDir("../some_file.txt"));
        System.out.println();

        // Test Case 5: Attempt directory traversal (absolute path)
        String absolutePath = Paths.get(".").toAbsolutePath().toString(); // Use a known file outside the dir
        System.out.println("5. Attempting to read an absolute path file like '" + absolutePath + "':");
        System.out.println("   Result: " + readFileFromSafeDir(absolutePath));
        System.out.println();
    }
}