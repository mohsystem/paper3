import java.io.IOException;
import java.util.regex.Pattern;

public class Lse114 {

    // A pattern to validate arguments to prevent command injection.
    // This is a whitelist of allowed characters.
    private static final Pattern SAFE_ARG_PATTERN = Pattern.compile("^[a-zA-Z0-9/._-]+$");

    /**
     * Validates an argument string against a whitelist of safe characters.
     * @param arg The string to validate.
     * @return true if the string is safe, false otherwise.
     */
    public static boolean isArgumentSafe(String arg) {
        // An empty string is a valid (no-op) argument.
        if (arg == null || arg.isEmpty()) {
            return true;
        }
        return SAFE_ARG_PATTERN.matcher(arg).matches();
    }

    /**
     * Constructs and executes an 'ls' command using a shell.
     *
     * @param path The directory path to list.
     * @param flags The flags to pass to the 'ls' command (e.g., "-l").
     */
    public static void executeLsCommand(String path, String flags) {
        // Rule #1: Ensure all input is validated according to expected formats.
        if (!isArgumentSafe(path) || !isArgumentSafe(flags)) {
            System.err.println("Error: Invalid characters in arguments. Aborting to prevent command injection.");
            return;
        }

        // The "print function" (String.format) is used to create the string for the system function.
        String command = String.format("ls %s %s", flags, path);

        System.out.println("Executing command: " + command);
        System.out.println("---");

        try {
            // Rule #2: Use potentially dangerous functions with caution.
            // Using a shell ("/bin/sh -c") is equivalent to the C system() call.
            // This is inherently risky and is only done here after strict input validation.
            // The recommended, safer alternative in Java is to use ProcessBuilder without a shell,
            // e.g., new ProcessBuilder("ls", flags, path).
            String[] shellCommand = {"/bin/sh", "-c", command};
            ProcessBuilder pb = new ProcessBuilder(shellCommand);
            
            // Redirects the subprocess's standard output and error to the current process.
            pb.inheritIO();
            
            Process process = pb.start();
            
            int exitCode = process.waitFor();
            
            System.out.println("---");
            System.out.println("Command finished with exit code: " + exitCode);

        } catch (IOException | InterruptedException e) {
            System.err.println("Failed to execute command: " + e.getMessage());
            // In a real application, log this error securely.
        }
    }

    /**
     * Main method with test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        System.out.println("--- Test Case 1: List current directory ---");
        executeLsCommand(".", "");

        System.out.println("\n--- Test Case 2: List /tmp with long format ---");
        executeLsCommand("/tmp", "-l");

        System.out.println("\n--- Test Case 3: Non-existent directory ---");
        executeLsCommand("/non_existent_dir_12345", "-a");

        System.out.println("\n--- Test Case 4: Invalid path (command injection attempt) ---");
        executeLsCommand(".; ls", "-l");

        System.out.println("\n--- Test Case 5: Invalid flag (command injection attempt) ---");
        executeLsCommand(".", "-l; echo vulnerable");
    }
}