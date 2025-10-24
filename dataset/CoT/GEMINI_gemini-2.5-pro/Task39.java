import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.regex.Pattern;

public class Task39 {

    /**
     * Displays the contents of a file using a system command.
     * This implementation uses ProcessBuilder to prevent command injection vulnerabilities.
     *
     * @param filename The name of the file to display.
     */
    public static void displayFileContents(String filename) {
        System.out.println("--- Attempting to display file: " + filename + " ---");

        // Security Check 1: Validate filename format to prevent path traversal.
        // This regex allows for alphanumeric characters, dots, underscores, and hyphens.
        // It disallows directory separators like '/' or '\' and sequences like '..'.
        String safePattern = "^[a-zA-Z0-9._-]+$";
        if (filename == null || !Pattern.matches(safePattern, filename) || filename.contains("..")) {
            System.out.println("Error: Invalid or unsafe filename provided.");
            System.out.println("--------------------------------------------------\n");
            return;
        }

        // Security Check 2: Verify that the file exists and is a file.
        File file = new File(filename);
        if (!file.exists() || !file.isFile()) {
            System.out.println("Error: File does not exist or is not a regular file.");
            System.out.println("--------------------------------------------------\n");
            return;
        }

        try {
            String os = System.getProperty("os.name").toLowerCase();
            String command = os.contains("win") ? "type" : "cat";

            // Security Core: Use ProcessBuilder to pass the filename as a single,
            // uninterpreted argument to the command, preventing shell injection.
            ProcessBuilder pb = new ProcessBuilder(command, filename);
            pb.inheritIO(); // Redirects the subprocess's output and error streams to the current process.

            Process process = pb.start();
            int exitCode = process.waitFor();
            System.out.println("\n> Command finished with exit code: " + exitCode);

        } catch (IOException | InterruptedException e) {
            System.err.println("Error executing command: " + e.getMessage());
        }
        System.out.println("--------------------------------------------------\n");
    }

    public static void main(String[] args) {
        // Setup: Create some test files
        try {
            FileWriter writer1 = new FileWriter("test1.txt");
            writer1.write("This is a safe test file.");
            writer1.close();

            FileWriter writer2 = new FileWriter("safe-file.log");
            writer2.write("This is another safe log file.");
            writer2.close();
        } catch (IOException e) {
            System.err.println("Failed to create test files.");
            e.printStackTrace();
            return;
        }

        // Test Case 1: A valid and safe filename
        displayFileContents("test1.txt");

        // Test Case 2: Another valid filename with a hyphen
        displayFileContents("safe-file.log");

        // Test Case 3: A file that does not exist
        displayFileContents("nonexistent.txt");

        // Test Case 4: A malicious input attempting command injection
        // This will be caught by the validation regex.
        displayFileContents("test1.txt; ls");

        // Test Case 5: A malicious input attempting path traversal
        // This will be caught by the validation regex.
        displayFileContents("../some_other_file");

        // Cleanup: Delete the test files
        new File("test1.txt").delete();
        new File("safe-file.log").delete();
    }
}