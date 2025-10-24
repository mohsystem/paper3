import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.io.File;

public class Task70 {

    /**
     * Reads the content of a file, ensuring the path is safe.
     *
     * @param filePath The path to the file.
     * @return The content of the file as a string, or an error message if reading fails.
     */
    public static String readFileContent(String filePath) {
        if (filePath == null || filePath.isEmpty()) {
            return "Error: File path is empty.";
        }
        
        try {
            Path path = Paths.get(filePath);

            // Rule #7: Refuse symlinks. Check before resolving the path.
            if (Files.isSymbolicLink(path)) {
                return "Error: Symbolic links are not allowed.";
            }

            // Rule #7: Ensure path resolves within the expected directory.
            // Using toRealPath with NOFOLLOW_LINKS to get the canonical path without following the final link.
            // This also checks for file existence.
            Path canonicalPath = path.toRealPath(LinkOption.NOFOLLOW_LINKS);
            Path baseDir = Paths.get(".").toRealPath();

            if (!canonicalPath.startsWith(baseDir)) {
                return "Error: Path traversal attempt detected.";
            }

            // Rule #7: Ensure we are reading a regular file, not a directory.
            if (!Files.isRegularFile(canonicalPath)) {
                return "Error: Path does not point to a regular file.";
            }

            // Rule #1, #8: Read file content safely using try-with-resources semantics.
            return Files.readString(canonicalPath, StandardCharsets.UTF_8);

        } catch (NoSuchFileException e) {
            return "Error: File does not exist at path: " + filePath;
        } catch (IOException | SecurityException e) {
            return "Error reading file: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            // Process file path from command line argument
            String content = readFileContent(args[0]);
            System.out.println(content);
        } else {
            // Run test cases if no arguments are provided
            runTests();
        }
    }

    private static void runTests() {
        System.out.println("Running test cases...");
        Path testDir = Paths.get("java_test_files");
        Path safeFile = testDir.resolve("safe_file.txt");
        Path subDir = testDir.resolve("sub");
        Path nestedFile = subDir.resolve("nested.txt");
        Path symlinkFile = testDir.resolve("slink.txt");
        
        try {
            // Setup test environment
            Files.createDirectories(subDir);
            Files.writeString(safeFile, "This is a safe file.");
            Files.writeString(nestedFile, "This is a nested file.");
            Files.createSymbolicLink(symlinkFile, safeFile.getFileName());

            // Test Case 1: Read a valid file
            System.out.println("\n--- Test Case 1: Valid file ---");
            System.out.println("Reading: " + safeFile);
            System.out.println("Content: " + readFileContent(safeFile.toString()));

            // Test Case 2: Read a non-existent file
            System.out.println("\n--- Test Case 2: Non-existent file ---");
            String nonExistentPath = testDir.resolve("nonexistent.txt").toString();
            System.out.println("Reading: " + nonExistentPath);
            System.out.println("Content: " + readFileContent(nonExistentPath));

            // Test Case 3: Attempt to read a directory
            System.out.println("\n--- Test Case 3: Directory path ---");
            System.out.println("Reading: " + subDir);
            System.out.println("Content: " + readFileContent(subDir.toString()));

            // Test Case 4: Attempt to read a symbolic link
            System.out.println("\n--- Test Case 4: Symbolic link ---");
            System.out.println("Reading: " + symlinkFile);
            System.out.println("Content: " + readFileContent(symlinkFile.toString()));

            // Test Case 5: Path traversal attempt
            System.out.println("\n--- Test Case 5: Path traversal ---");
            // Assuming current directory is project root, and tests run from there.
            // This path tries to go up one level from the current directory.
            String traversalPath = ".." + File.separator + "pom.xml"; // A common file outside CWD
            System.out.println("Reading: " + traversalPath);
            System.out.println("Content: " + readFileContent(traversalPath));

        } catch (IOException e) {
            System.err.println("Test setup failed: " + e.getMessage());
        } finally {
            // Cleanup
            try {
                if(Files.exists(symlinkFile)) Files.delete(symlinkFile);
                if(Files.exists(nestedFile)) Files.delete(nestedFile);
                if(Files.exists(subDir)) Files.delete(subDir);
                if(Files.exists(safeFile)) Files.delete(safeFile);
                if(Files.exists(testDir)) Files.delete(testDir);
                System.out.println("\nTest cleanup complete.");
            } catch (IOException e) {
                System.err.println("Test cleanup failed: " + e.getMessage());
            }
        }
    }
}