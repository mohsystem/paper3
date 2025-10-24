import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.stream.Collectors;

public class Task158 {

    // A whitelist of allowed directory names. This is the most secure way to validate
    // input that will be used in OS commands.
    private static final Set<String> ALLOWED_DIRECTORIES = new HashSet<>(Arrays.asList("safe_dir_1", "safe_dir_2", "."));

    /**
     * Executes the 'ls -l' command on an allowed directory in a secure manner.
     *
     * @param directory The directory name to list. Must be in the allowed list.
     * @return The output of the command or an error message if the input is invalid or an error occurs.
     */
    public static String listDirectoryContents(String directory) {
        if (directory == null || !ALLOWED_DIRECTORIES.contains(directory)) {
            return "Error: Invalid or disallowed directory specified.";
        }

        // Using ProcessBuilder is the secure way to execute commands in Java.
        // It treats the command and its arguments as separate strings,
        // preventing shell metacharacters in the input from being interpreted,
        // which mitigates OS command injection vulnerabilities.
        ProcessBuilder processBuilder = new ProcessBuilder("ls", "-l", directory);
        processBuilder.redirectErrorStream(true); // Combine stdout and stderr for simplicity

        try {
            Process process = processBuilder.start();

            String output;
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8))) {
                output = reader.lines().collect(Collectors.joining(System.lineSeparator()));
            }

            int exitCode = process.waitFor();
            if (exitCode != 0) {
                // In a real application, you might want to handle the non-zero exit code more gracefully.
                return "Error: Command exited with code " + exitCode + "\nOutput:\n" + output;
            }

            return output;

        } catch (IOException | InterruptedException e) {
            // Restore the interrupted status
            if (e instanceof InterruptedException) {
                Thread.currentThread().interrupt();
            }
            // In a real application, log this exception properly.
            return "Error: Failed to execute command. " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        // Note: For testing purposes, we create dummy directories. In a secure application,
        // file system write operations should be carefully controlled and restricted.
        new java.io.File("safe_dir_1").mkdir();
        new java.io.File("safe_dir_2").mkdir();

        String[] testCases = {
            ".",                  // A valid case (current directory)
            "safe_dir_1",         // A valid case from the whitelist
            "safe_dir_2",         // Another valid case from the whitelist
            "non_existent_dir",   // An invalid case (not in whitelist)
            "safe_dir_1; whoami", // A malicious attempt (will be rejected by the whitelist)
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + ": Input='" + testCases[i] + "' ---");
            String result = listDirectoryContents(testCases[i]);
            System.out.println(result);
            System.out.println("--- End Test Case " + (i + 1) + " ---\n");
        }
    }
}