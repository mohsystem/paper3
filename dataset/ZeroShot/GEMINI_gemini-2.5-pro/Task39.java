import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class Task39 {

    /**
     * Securely displays a file's content using a system command.
     * Note: On Windows, the command "cat" might need to be changed to "type".
     *
     * @param filename The name of the file to display.
     */
    public static void displayFileContent(String filename) {
        System.out.println("--- Attempting to display: '" + filename + "' ---");

        // Using ProcessBuilder is secure because it treats the command and its arguments
        // as separate entities. The filename is passed as a single, uninterpreted
        // argument, preventing the shell from executing any embedded commands.
        ProcessBuilder pb = new ProcessBuilder("cat", filename);

        try {
            Process process = pb.start();

            // Read standard output from the command
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    System.out.println(line);
                }
            }

            // Read standard error from the command
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getErrorStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    System.err.println(line);
                }
            }

            int exitCode = process.waitFor();
            System.out.println("-> Command finished with exit code: " + exitCode);

        } catch (IOException e) {
            System.err.println("Error executing command: " + e.getMessage());
        } catch (InterruptedException e) {
            System.err.println("Process was interrupted: " + e.getMessage());
            Thread.currentThread().interrupt();
        }
        System.out.println("--- End of attempt ---\n");
    }

    public static void main(String[] args) {
        // 1. Setup: Create dummy files for testing
        Path file1 = Paths.get("test_file_1.txt");
        Path file2 = Paths.get("file with spaces.txt");
        try {
            Files.writeString(file1, "Hello from test file 1.");
            Files.writeString(file2, "This file has spaces in its name.");
            System.out.println("Created test files.\n");
        } catch (IOException e) {
            System.err.println("Failed to create test files: " + e.getMessage());
            return;
        }

        // 2. Test cases
        String[] testCases = {
            file1.toString(),                     // 1. Valid file
            file2.toString(),                     // 2. Valid file with spaces
            "non_existent_file.txt",              // 3. Non-existent file
            "test_file_1.txt; ls -l",             // 4. Command injection attempt (benign)
            "| whoami"                            // 5. Command injection attempt (malicious)
        };

        for (String testCase : testCases) {
            displayFileContent(testCase);
        }

        // 3. Cleanup: Delete the dummy files
        try {
            Files.deleteIfExists(file1);
            Files.deleteIfExists(file2);
            System.out.println("Cleaned up test files.");
        } catch (IOException e) {
            System.err.println("Failed to clean up test files: " + e.getMessage());
        }
    }
}