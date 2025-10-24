import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Optional;
import java.util.regex.Pattern;

public class Task65 {

    // Rule #1, #5: Validate filename format.
    // Whitelist of allowed characters: alphanumeric, dot, underscore, hyphen. Length 1-255.
    // This prevents path traversal ('../', '/') and other malicious inputs.
    private static final Pattern FILENAME_PATTERN = Pattern.compile("^[a-zA-Z0-9._-]{1,255}$");

    /**
     * Validates a filename and reads its content if it's a regular, accessible file.
     *
     * @param filename The name of the file to read, must be in the current directory.
     * @return An Optional containing the file content as a String, or an empty Optional on error.
     */
    public static Optional<String> readFileContent(String filename) {
        if (filename == null || filename.isEmpty()) {
            System.err.println("Error: Filename cannot be null or empty.");
            return Optional.empty();
        }

        if (!FILENAME_PATTERN.matcher(filename).matches()) {
            System.err.println("Error: Invalid filename. Only alphanumeric, '.', '_', '-' are allowed. No path separators.");
            return Optional.empty();
        }

        try {
            Path path = Paths.get(filename);

            // Rule #5, #6: Check for existence and that it's a regular file (not a directory or symlink).
            // The check and read are close, but a TOCTOU vulnerability is still possible.
            // For higher security, file locking or more advanced mechanisms would be needed.
            if (!Files.exists(path)) {
                System.err.println("Error: File does not exist: " + filename);
                return Optional.empty();
            }

            if (!Files.isRegularFile(path)) {
                System.err.println("Error: Path is not a regular file: " + filename);
                return Optional.empty();
            }

            // Rule #7: Handle potential IOExceptions.
            // Rule #8: Use UTF-8, use modern NIO features.
            return Optional.of(Files.readString(path, StandardCharsets.UTF_8));

        } catch (IOException | SecurityException e) {
            System.err.println("Error reading file '" + filename + "': " + e.getMessage());
            return Optional.empty();
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        System.out.println("--- Running Test Cases ---");
        String validFile = "test1.txt";
        String emptyFile = "empty.txt";
        String nonExistentFile = "no_such_file.txt";
        String directoryName = "test_dir";
        String invalidFilename = "../test1.txt";
        
        File dir = new File(directoryName);

        try {
            // Setup test environment
            dir.mkdir();
            try (PrintWriter writer = new PrintWriter(validFile, StandardCharsets.UTF_8)) {
                writer.println("Hello, Java World!");
            }
            try (PrintWriter writer = new PrintWriter(emptyFile, StandardCharsets.UTF_8)) {
                // write nothing
            }

            // Test Case 1: Valid file
            System.out.println("\n1. Testing with a valid file (" + validFile + "):");
            readFileContent(validFile).ifPresentOrElse(
                content -> System.out.println("Success. Content:\n" + content.trim()),
                () -> System.err.println("Failed unexpectedly.")
            );

            // Test Case 2: Non-existent file
            System.out.println("\n2. Testing with a non-existent file (" + nonExistentFile + "):");
            readFileContent(nonExistentFile).ifPresentOrElse(
                content -> System.err.println("Failed. Read content unexpectedly."),
                () -> System.out.println("Success. Function returned empty as expected.")
            );

            // Test Case 3: Invalid filename (path traversal)
            System.out.println("\n3. Testing with an invalid filename (" + invalidFilename + "):");
            readFileContent(invalidFilename).ifPresentOrElse(
                content -> System.err.println("Failed. Read content unexpectedly."),
                () -> System.out.println("Success. Function returned empty as expected.")
            );

            // Test Case 4: A directory
            System.out.println("\n4. Testing with a directory (" + directoryName + "):");
            readFileContent(directoryName).ifPresentOrElse(
                content -> System.err.println("Failed. Read content unexpectedly."),
                () -> System.out.println("Success. Function returned empty as expected.")
            );

            // Test Case 5: An empty file
            System.out.println("\n5. Testing with an empty file (" + emptyFile + "):");
            readFileContent(emptyFile).ifPresentOrElse(
                content -> System.out.println("Success. Content is empty: '" + content.trim() + "'"),
                () -> System.err.println("Failed unexpectedly.")
            );

        } catch (IOException e) {
            System.err.println("Test setup failed: " + e.getMessage());
        } finally {
            // Cleanup test environment
            new File(validFile).delete();
            new File(emptyFile).delete();
            dir.delete();
        }
        
        System.out.println("\n--- End of Test Cases ---");

        // --- Main Program Logic ---
        if (args.length != 1) {
            System.err.println("Usage: java Task65 <filename>");
            System.exit(1);
        }

        String filename = args[0];
        System.out.println("\nAttempting to read from command line argument: " + filename);
        Optional<String> content = readFileContent(filename);

        content.ifPresentOrElse(
            data -> {
                System.out.println("--- File Content ---");
                System.out.println(data);
                System.out.println("--- End of Content ---");
            },
            () -> {
                System.err.println("Could not read file.");
                System.exit(1);
            }
        );
    }
}