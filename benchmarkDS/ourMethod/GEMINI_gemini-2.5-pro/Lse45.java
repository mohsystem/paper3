import java.io.IOException;
import java.nio.file.*;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.Arrays;

public class Lse45 {

    private static final long MAX_FILE_SIZE = 1024 * 1024; // 1 MB

    /**
     * Reads a file from a designated safe directory, preventing path traversal
     * and other file-based vulnerabilities.
     *
     * @param safeDirStr The path to the trusted directory.
     * @param filename   The name of the file to read (must not contain path separators).
     * @return The file content as a byte array, or null on failure.
     */
    public static byte[] readFileFromSafeDirectory(String safeDirStr, String filename) {
        // 1. Validate filename to ensure it's a simple name, not a path.
        if (filename == null || filename.contains("/") || filename.contains("\\")) {
            System.err.println("Error: Filename must be a simple name and not contain path separators.");
            return null;
        }

        Path safeDir = Paths.get(safeDirStr).toAbsolutePath();
        Path requestedFile = safeDir.resolve(filename);

        // 2. After resolving, normalize and ensure the path is still within the safe directory.
        Path normalizedPath = requestedFile.normalize();
        if (!normalizedPath.startsWith(safeDir)) {
            System.err.println("Error: Path traversal attempt detected.");
            return null;
        }

        try {
            // 3. Atomically get file attributes without following symlinks.
            // This approach minimizes the TOCTOU (Time-of-Check to Time-of-Use) window
            // by performing a single, atomic check on the path's properties.
            BasicFileAttributes attrs = Files.readAttributes(
                normalizedPath,
                BasicFileAttributes.class,
                LinkOption.NOFOLLOW_LINKS
            );

            // 4. Validate attributes: must be a regular file and within size limits.
            if (!attrs.isRegularFile()) {
                System.err.println("Error: Requested path is not a regular file.");
                return null;
            }

            if (attrs.size() > MAX_FILE_SIZE) {
                System.err.println("Error: File size exceeds the maximum limit of " + MAX_FILE_SIZE + " bytes.");
                return null;
            }

            // 5. Read the file content. Since attributes have been checked on the
            // same path without following links, this read is safe.
            return Files.readAllBytes(normalizedPath);

        } catch (NoSuchFileException e) {
            System.err.println("Error: File not found: " + filename);
        } catch (IOException | SecurityException e) {
            System.err.println("Error reading file: " + e.getMessage());
        }
        return null;
    }

    // Main method with test cases
    public static void main(String[] args) {
        String safeDirName = "java_safe_dir";
        Path safeDirPath = Paths.get(safeDirName);
        Path secretFilePath = Paths.get("java_secret_file.txt");

        try {
            // Setup test environment
            Files.createDirectories(safeDirPath);
            Files.write(safeDirPath.resolve("good.txt"), "This is a safe file.".getBytes());
            Files.write(secretFilePath, "This is a secret.".getBytes());

            // Create a large file
            byte[] largeContent = new byte[(int) MAX_FILE_SIZE + 1];
            Files.write(safeDirPath.resolve("large.txt"), largeContent);

            // Create a symbolic link (may fail on Windows without admin rights)
            try {
                Files.createSymbolicLink(safeDirPath.resolve("symlink.txt"), secretFilePath.toAbsolutePath());
            } catch (IOException | UnsupportedOperationException e) {
                System.out.println("Warning: Could not create symbolic link. Symlink test will be skipped.");
            }

            System.out.println("--- Running Test Cases ---");

            // Test Case 1: Read a valid file
            System.out.println("\n1. Reading a valid file (good.txt):");
            byte[] content = readFileFromSafeDirectory(safeDirName, "good.txt");
            if (content != null) {
                System.out.println("   Success. Content: " + new String(content));
            } else {
                System.out.println("   Failed.");
            }

            // Test Case 2: Read a non-existent file
            System.out.println("\n2. Reading a non-existent file (nonexistent.txt):");
            content = readFileFromSafeDirectory(safeDirName, "nonexistent.txt");
            if (content == null) {
                System.out.println("   Success: Read operation failed as expected.");
            } else {
                System.out.println("   Failed: Code read a non-existent file.");
            }

            // Test Case 3: Read a file that is too large
            System.out.println("\n3. Reading a large file (large.txt):");
            content = readFileFromSafeDirectory(safeDirName, "large.txt");
            if (content == null) {
                System.out.println("   Success: Read operation failed as expected.");
            } else {
                System.out.println("   Failed: Code read an oversized file.");
            }

            // Test Case 4: Attempt path traversal
            System.out.println("\n4. Attempting path traversal (../" + secretFilePath.getFileName() + "):");
            content = readFileFromSafeDirectory(safeDirName, "../" + secretFilePath.getFileName());
            if (content == null) {
                System.out.println("   Success: Path traversal was blocked.");
            } else {
                System.out.println("   Failed: Path traversal was successful.");
            }

            // Test Case 5: Attempt to read a symbolic link
            if (Files.isSymbolicLink(safeDirPath.resolve("symlink.txt"))) {
                System.out.println("\n5. Reading a symbolic link (symlink.txt):");
                content = readFileFromSafeDirectory(safeDirName, "symlink.txt");
                if (content == null) {
                    System.out.println("   Success: Reading symbolic link was blocked.");
                } else {
                    System.out.println("   Failed: Code followed a symbolic link.");
                }
            } else {
                 System.out.println("\n5. Reading a symbolic link (symlink.txt): SKIPPED");
            }

        } catch (IOException e) {
            System.err.println("An error occurred during test setup: " + e.getMessage());
        } finally {
            // Cleanup
            try { Files.deleteIfExists(safeDirPath.resolve("good.txt")); } catch (IOException e) { e.printStackTrace(); }
            try { Files.deleteIfExists(safeDirPath.resolve("large.txt")); } catch (IOException e) { e.printStackTrace(); }
            try { Files.deleteIfExists(safeDirPath.resolve("symlink.txt")); } catch (IOException e) { e.printStackTrace(); }
            try { Files.deleteIfExists(safeDirPath); } catch (IOException e) { e.printStackTrace(); }
            try { Files.deleteIfExists(secretFilePath); } catch (IOException e) { e.printStackTrace(); }
        }
    }
}