import java.io.BufferedReader;
import java.io.IOException;
import java.nio.file.AccessDeniedException;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Task36 {

    // Define a safe base directory for file operations.
    private static final Path SAFE_DIRECTORY = Paths.get("safe_dir").toAbsolutePath();

    /**
     * Securely reads a file and prints its contents to the console.
     * The file is expected to be inside a pre-defined safe directory.
     *
     * @param filename The name of the file to read (not the full path).
     */
    public static void readFileAndPrint(String filename) {
        // Basic validation: filename should not be null, empty, or contain path separators.
        if (filename == null || filename.trim().isEmpty() || filename.contains("/") || filename.contains("\\")) {
            System.err.println("Error: Invalid filename provided.");
            return;
        }

        try {
            // Create the full path by resolving the filename against the safe directory.
            Path filePath = SAFE_DIRECTORY.resolve(filename);

            // Security Check 1: Canonicalize the path and verify it's still within the safe directory.
            // This prevents path traversal attacks (e.g., "safe_dir/../some_other_file").
            Path canonicalPath = filePath.toRealPath();
            if (!canonicalPath.startsWith(SAFE_DIRECTORY)) {
                System.err.println("Error: Access denied. Path is outside the safe directory.");
                return;
            }

            // Security Check 2: Ensure the resolved path is a regular file.
            if (!Files.isRegularFile(canonicalPath)) {
                System.err.println("Error: Path does not point to a regular file.");
                return;
            }

            System.out.println("--- Reading file: " + filename + " ---");
            // Use try-with-resources to ensure the reader is closed automatically.
            // Read line by line to avoid loading large files into memory.
            try (BufferedReader reader = Files.newBufferedReader(canonicalPath)) {
                String line;
                while ((line = reader.readLine()) != null) {
                    System.out.println(line);
                }
            }
            System.out.println("--- End of file: " + filename + " ---\n");

        } catch (NoSuchFileException e) {
            System.err.println("Error: File not found: " + filename);
        } catch (AccessDeniedException e) {
            System.err.println("Error: Permission denied for file: " + filename);
        } catch (IOException e) {
            System.err.println("An error occurred while reading the file: " + filename);
        } catch (Exception e) {
            System.err.println("An unexpected error occurred.");
        }
    }

    public static void main(String[] args) {
        // --- Test Case Setup ---
        try {
            Files.createDirectories(SAFE_DIRECTORY);
            Files.write(SAFE_DIRECTORY.resolve("test1.txt"), "This is the first test file.".getBytes());
            Files.write(SAFE_DIRECTORY.resolve("test2.txt"), "This is the second test file.\nIt has multiple lines.".getBytes());
            Files.createDirectories(SAFE_DIRECTORY.resolve("subdir"));
            Files.write(Paths.get("insecure.txt"), "This file is outside the safe directory.".getBytes());
            System.out.println("Setup complete. Safe directory is at: " + SAFE_DIRECTORY);
            System.out.println("Running tests...\n");
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }

        // --- Test Cases ---
        System.out.println("Test Case 1: Reading a valid file.");
        readFileAndPrint("test1.txt");

        System.out.println("Test Case 2: Reading another valid file.");
        readFileAndPrint("test2.txt");

        System.out.println("Test Case 3: Attempting to read a non-existent file.");
        readFileAndPrint("nonexistent.txt");
        System.out.println();

        System.out.println("Test Case 4: Path traversal attempt.");
        readFileAndPrint("../insecure.txt");
        System.out.println();
        
        System.out.println("Test Case 5: Attempting to read a directory.");
        readFileAndPrint("subdir");
        System.out.println();
    }
}