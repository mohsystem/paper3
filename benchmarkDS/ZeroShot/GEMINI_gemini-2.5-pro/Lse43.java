import java.io.BufferedReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.NoSuchFileException;
import java.util.stream.Collectors;

public class Lse43 {

    private static final String SAFE_DIRECTORY = "/safe/";

    /**
     * Securely reads a file from the SAFE_DIRECTORY.
     * It prevents path traversal attacks by resolving the canonical path and
     * ensuring it starts with the safe base directory.
     *
     * @param filename The name of the file to read. Must not contain path separators.
     * @return The content of the file as a String.
     * @throws IOException if an I/O error occurs.
     * @throws SecurityException if a path traversal attempt is detected.
     */
    public static String readFileFromSafeDirectory(String filename) throws IOException, SecurityException {
        // Basic validation: filename should not contain path separators.
        if (filename == null || filename.contains("/") || filename.contains("\\")) {
            throw new SecurityException("Invalid filename format. Path separators are not allowed.");
        }

        try {
            Path baseDir = Paths.get(SAFE_DIRECTORY).toAbsolutePath();
            Path requestedPath = baseDir.resolve(filename).normalize();

            // To prevent TOCTOU (Time-of-check to time-of-use) vulnerabilities,
            // we get the real path after resolving symlinks.
            Path realPath = requestedPath.toRealPath();

            // Security Check: Ensure the canonical path is still within the safe directory.
            if (!realPath.startsWith(baseDir)) {
                throw new SecurityException("Path traversal attempt detected for file: " + filename);
            }

            try (BufferedReader reader = Files.newBufferedReader(realPath)) {
                return reader.lines().collect(Collectors.joining(System.lineSeparator()));
            }

        } catch (InvalidPathException e) {
            throw new SecurityException("Invalid characters in filename: " + filename, e);
        } catch (NoSuchFileException e) {
            // Re-throw as a more general IOException or return empty to hide file existence info
            throw new IOException("File not found or not accessible: " + filename, e);
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");
        System.out.println("NOTE: For these tests to pass, create the /safe directory and files:");
        System.out.println("  sudo mkdir -p /safe/docs");
        System.out.println("  sudo chown -R $(whoami) /safe");
        System.out.println("  echo \"This is a safe report.\" > /safe/report.txt");
        System.out.println("  echo \"This is a nested manual.\" > /safe/docs/manual.pdf\n");

        String[] testFilenames = {
            "report.txt",                 // 1. Valid file
            "docs/manual.pdf",            // 2. Valid file in a subdirectory (will be blocked by our simple check)
            "nonexistent.txt",            // 3. Non-existent file
            "../../../../etc/passwd",     // 4. Path traversal attempt
            "/etc/passwd"                 // 5. Absolute path attempt (will be blocked)
        };

        for (String filename : testFilenames) {
            System.out.println("--- Testing with filename: '" + filename + "' ---");
            try {
                String content = readFileFromSafeDirectory(filename);
                System.out.println("Successfully read file. Content:");
                System.out.println(content);
            } catch (IOException | SecurityException e) {
                System.err.println("Caught expected exception: " + e.getClass().getName() + " - " + e.getMessage());
            }
            System.out.println("----------------------------------------\n");
        }
    }
}