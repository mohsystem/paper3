import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Optional;

public class Task129 {

    // The base directory from which files can be served.
    private static final String BASE_DIRECTORY = "public";

    /**
     * Securely retrieves the content of a file from a predefined base directory.
     * It prevents path traversal attacks by ensuring the requested file's canonical
     * path is within the base directory.
     *
     * @param userProvidedFilename The name of the file provided by the user.
     * @return An Optional containing the file content if successful, otherwise an empty Optional.
     */
    public static Optional<String> retrieveFileContent(String userProvidedFilename) {
        if (userProvidedFilename == null || userProvidedFilename.isEmpty()) {
            System.err.println("Error: Filename cannot be null or empty.");
            return Optional.empty();
        }

        Path baseDirPath = Paths.get(BASE_DIRECTORY).toAbsolutePath();
        
        // Sanitize input: only consider the filename part, stripping any path.
        // This is a crucial step to prevent directory traversal.
        Path safeFilename = Paths.get(userProvidedFilename).getFileName();
        
        Path fullPath = baseDirPath.resolve(safeFilename).normalize();

        try {
            // Ensure the base directory exists
            if (!Files.exists(baseDirPath)) {
                 System.err.println("Error: Base directory '" + BASE_DIRECTORY + "' does not exist.");
                 return Optional.empty();
            }

            // Get the canonical (real) path of the target file.
            // This resolves any '..', '.', or symbolic links.
            Path realFullPath = fullPath.toRealPath();
            Path realBaseDirPath = baseDirPath.toRealPath();

            // Security Check: The real path of the file must start with the real path of the base directory.
            if (!realFullPath.startsWith(realBaseDirPath)) {
                System.err.println("Security Alert: Path traversal attempt detected for file: " + userProvidedFilename);
                return Optional.empty();
            }
            
            if (Files.isReadable(realFullPath) && !Files.isDirectory(realFullPath)) {
                 return Optional.of(new String(Files.readAllBytes(realFullPath)));
            } else {
                 System.err.println("Error: File is not readable or is a directory: " + realFullPath);
                 return Optional.empty();
            }

        } catch (IOException e) {
            // Catches file not found, permissions errors, etc.
            System.err.println("Error retrieving file '" + userProvidedFilename + "': " + e.getMessage());
            return Optional.empty();
        }
    }

    // Main method with test cases
    public static void main(String[] args) {
        // --- Test Setup ---
        try {
            // Create directories
            Files.createDirectories(Paths.get("public/safe_dir"));
            Files.createDirectories(Paths.get("secret"));
            // Create test files
            try (PrintWriter out = new PrintWriter("public/file1.txt")) {
                out.println("This is file 1.");
            }
            try (PrintWriter out = new PrintWriter("public/safe_dir/file2.txt")) {
                out.println("This is file 2.");
            }
            try (PrintWriter out = new PrintWriter("secret/secret.txt")) {
                out.println("This is a secret.");
            }
            System.out.println("Test environment created successfully.");
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }

        // --- Test Cases ---
        String[] testCases = {
            "file1.txt",                          // 1. Valid file
            "safe_dir/file2.txt",                 // 2. Valid file in subdir (should fail due to basename sanitization)
            "../secret/secret.txt",               // 3. Path Traversal
            System.getProperty("os.name").startsWith("Windows") 
                ? "C:\\Windows\\System32\\drivers\\etc\\hosts" // 4. Absolute Path (Windows)
                : "/etc/passwd",                      // 4. Absolute Path (Unix-like)
            "nonexistent.txt"                     // 5. Non-existent file
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + ": Requesting '" + testCases[i] + "' ---");
            Optional<String> content = retrieveFileContent(testCases[i]);
            if (content.isPresent()) {
                System.out.println("Success! File content:\n" + content.get());
            } else {
                System.out.println("Failed as expected.");
            }
        }
    }
}