import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.LinkOption;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class Task39 {

    private static final Pattern FILENAME_PATTERN = Pattern.compile("^[a-zA-Z0-9._-]+$");

    /**
     * Displays the content of a file using a system command ('cat' or 'type').
     * The filename is validated to prevent command injection and path traversal.
     *
     * @param filename The name of the file to display. It must be in the current directory.
     */
    public static void displayFileContent(String filename) {
        if (filename == null || filename.isEmpty()) {
            System.err.println("Error: Filename cannot be null or empty.");
            return;
        }

        // Rule #1 & #5: Validate filename format and prevent path traversal.
        // The regex ensures no special shell characters or path separators.
        if (!FILENAME_PATTERN.matcher(filename).matches()) {
            System.err.println("Error: Invalid filename format. Only alphanumeric characters, dots, underscores, and hyphens are allowed.");
            return;
        }

        Path path = Paths.get(filename);

        // Rule #5: Further check to ensure the path has no directory components.
        if (path.getNameCount() > 1) {
            System.err.println("Error: Path traversal is not allowed. Please provide a simple filename.");
            return;
        }

        // Rule #5 & #6: Check if it's a regular file and not a directory or symlink before use (TOCTOU mitigation).
        if (!Files.isRegularFile(path, LinkOption.NOFOLLOW_LINKS)) {
            System.err.println("Error: File does not exist, is not a regular file, or is a symbolic link.");
            return;
        }

        try {
            // Rule #2: Use ProcessBuilder to pass arguments securely, avoiding shell interpretation.
            List<String> command = new ArrayList<>();
            String os = System.getProperty("os.name").toLowerCase();
            if (os.contains("win")) {
                command.add("cmd.exe");
                command.add("/c");
                command.add("type");
            } else {
                command.add("cat");
            }
            command.add(filename);

            ProcessBuilder pb = new ProcessBuilder(command);
            pb.inheritIO(); // Redirects subprocess's I/O to the current Java process.
            Process process = pb.start();
            int exitCode = process.waitFor();
            System.out.println("\n--- Command finished with exit code: " + exitCode + " ---");

        } catch (IOException | InterruptedException e) {
            System.err.println("Error executing command: " + e.getMessage());
            // In a real application, log this exception.
            if (e instanceof InterruptedException) {
               Thread.currentThread().interrupt(); // Restore interrupted status
            }
        }
    }
    
    // Main method with test cases
    public static void main(String[] args) {
        // Setup test files and directories
        try {
            File testDir = new File("test_dir");
            if (!testDir.exists()) {
                testDir.mkdir();
            }
            File validFile = new File("test_valid.txt");
            try (FileWriter writer = new FileWriter(validFile)) {
                writer.write("This is a valid test file.");
            }
             if (!System.getProperty("os.name").toLowerCase().contains("win")) {
                try {
                    Files.createSymbolicLink(Paths.get("test_symlink.txt"), validFile.toPath());
                } catch (UnsupportedOperationException | SecurityException e) {
                    System.out.println("Could not create symlink, skipping test case.");
                }
            }
        } catch (IOException e) {
            System.err.println("Failed to set up test environment: " + e.getMessage());
            return;
        }

        System.out.println("--- Test Case 1: Valid file ---");
        displayFileContent("test_valid.txt");
        System.out.println();

        System.out.println("--- Test Case 2: Non-existent file ---");
        displayFileContent("non_existent_file.txt");
        System.out.println();

        System.out.println("--- Test Case 3: Invalid filename (command injection attempt) ---");
        displayFileContent("test_valid.txt; ls");
        System.out.println();

        System.out.println("--- Test Case 4: Invalid filename (path traversal attempt) ---");
        displayFileContent("../another_file.txt");
        System.out.println();
        
        System.out.println("--- Test Case 5: Directory instead of file ---");
        displayFileContent("test_dir");
        System.out.println();
        
        if (new File("test_symlink.txt").exists()) {
            System.out.println("--- Test Case 6: Symbolic link ---");
            displayFileContent("test_symlink.txt");
            System.out.println();
        }

        // Cleanup
        new File("test_valid.txt").delete();
        new File("test_symlink.txt").delete();
        new File("test_dir").delete();
    }
}