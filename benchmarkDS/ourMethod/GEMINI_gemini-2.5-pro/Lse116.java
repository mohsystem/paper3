import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class Lse116 {

    // A pattern to validate arguments, following a whitelist approach.
    // Allows:
    // - options starting with '-' (e.g., -l, -a)
    // - file/directory paths (alphanumeric, /, ., _, -)
    // - wildcards like '*' or '?' for shell globbing.
    // Disallows shell metacharacters (`&`, `|`, `;`, `$`, etc.) that could lead to command injection.
    private static final Pattern VALID_ARG_PATTERN = Pattern.compile("^[a-zA-Z0-9_\\.\\/\\*\\?\\-]+$");

    /**
     * Executes the 'ls' command with the given arguments in a shell.
     * This method validates each argument to prevent command injection vulnerabilities.
     *
     * Note: Using ProcessBuilder is generally safer as it can avoid invoking a shell.
     * This implementation uses a shell to match the prompt's requirements of
     * emulating a system() call.
     *
     * @param args A variable number of string arguments for the 'ls' command.
     */
    public static void executeLsCommand(String... args) {
        List<String> commandParts = new ArrayList<>();
        commandParts.add("ls");

        for (String arg : args) {
            if (arg == null || arg.isEmpty()) {
                continue;
            }
            // SECURITY: Validate each argument against a whitelist pattern to prevent command injection.
            if (!VALID_ARG_PATTERN.matcher(arg).matches()) {
                System.err.println("Error: Invalid or potentially malicious argument detected: " + arg);
                return;
            }
            commandParts.add(arg);
        }
        
        // The 'print function' (in this case, String.join) is used to create the string
        // that is passed to the system shell.
        String command = String.join(" ", commandParts);
        System.out.println("Executing command: " + command);

        Process process = null;
        try {
            // Using "/bin/sh -c" emulates the behavior of the C system() call,
            // which executes the command within a shell.
            String[] shellCommand = {"/bin/sh", "-c", command};
            process = Runtime.getRuntime().exec(shellCommand);

            // Capture and print standard output
            try (BufferedReader stdInput = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String s;
                System.out.println("Output:");
                while ((s = stdInput.readLine()) != null) {
                    System.out.println(s);
                }
            }

            // Capture and print standard error
            try (BufferedReader stdError = new BufferedReader(new InputStreamReader(process.getErrorStream()))) {
                String s;
                System.out.println("Error (if any):");
                while ((s = stdError.readLine()) != null) {
                    System.err.println(s);
                }
            }

            int exitCode = process.waitFor();
            System.out.println("Exit Code: " + exitCode);

        } catch (IOException | InterruptedException e) {
            System.err.println("An error occurred while executing the command.");
            System.err.println(e.getMessage());
            // In a real application, this would be logged more robustly.
            Thread.currentThread().interrupt(); // Restore interrupted status
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Test Case 1: ls current directory ---");
        executeLsCommand(); // No arguments

        System.out.println("\n--- Test Case 2: ls -l -a ---");
        executeLsCommand("-l", "-a");

        System.out.println("\n--- Test Case 3: ls specific path ---");
        executeLsCommand("."); // Use a safe, known path

        System.out.println("\n--- Test Case 4: ls non-existent file (shows error stream) ---");
        executeLsCommand("/non_existent_dir_12345");
        
        System.out.println("\n--- Test Case 5: Attempted command injection (should be rejected) ---");
        executeLsCommand("-l;id");
    }
}