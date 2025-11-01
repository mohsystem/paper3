import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.regex.Pattern;

public class Lse115 {

    // A pattern to validate arguments. It allows alphanumeric characters, dashes, dots, and slashes.
    // This is a restrictive whitelist to prevent command injection.
    // It does not prevent path traversal (e.g., "ls ../../"), which could be an information disclosure vulnerability.
    private static final Pattern SAFE_ARG_PATTERN = Pattern.compile("^[a-zA-Z0-9_\\-./]+$");

    /**
     * Creates a command string using a format string and arguments.
     *
     * @param format The format string.
     * @param args   The arguments to be formatted into the string.
     * @return The formatted command string.
     */
    public static String createCommandString(String format, Object... args) {
        // This function simply formats a string and is not inherently insecure,
        // but the string it produces must be handled with care.
        return String.format(format, args);
    }

    /**
     * Executes an 'ls' command provided as a single string.
     * The command string is parsed and validated before execution to prevent
     * command injection vulnerabilities. It specifically avoids using a shell.
     *
     * @param command The command string to execute, e.g., "ls -l /tmp".
     */
    public static void executeLsCommand(String command) {
        if (command == null || command.trim().isEmpty()) {
            System.err.println("Error: Command string is null or empty.");
            return;
        }

        // Simple parsing by splitting on whitespace. This does not handle arguments with spaces.
        String[] parts = command.trim().split("\\s+");
        if (parts.length == 0) {
            System.err.println("Error: Invalid command format.");
            return;
        }

        // Security Check 1: The command must be exactly 'ls'.
        // This prevents executing arbitrary commands.
        if (!parts[0].equals("ls")) {
            System.err.println("Error: Only 'ls' command is allowed. Found: " + parts[0]);
            return;
        }

        // Security Check 2: Validate all arguments against a safe pattern.
        // This prevents shell metacharacters and other injection attacks.
        for (int i = 1; i < parts.length; i++) {
            if (!SAFE_ARG_PATTERN.matcher(parts[i]).matches()) {
                System.err.println("Error: Argument contains invalid characters: " + parts[i]);
                return;
            }
        }
        
        // Use ProcessBuilder with the parsed and validated command parts.
        // This is safer than Runtime.exec(String) because it avoids shell interpretation.
        try {
            ProcessBuilder pb = new ProcessBuilder(parts);
            pb.redirectErrorStream(true);
            Process process = pb.start();

            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    System.out.println(line);
                }
            }

            int exitCode = process.waitFor();
            if (exitCode != 0) {
                // Non-zero exit code is not always an application error (e.g., ls non_existent_dir)
                // but we print it for informative purposes.
                System.err.println("'ls' command exited with code: " + exitCode);
            }

        } catch (IOException | InterruptedException e) {
            System.err.println("Error executing command: " + e.getMessage());
            Thread.currentThread().interrupt(); // Preserve interrupted status
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Test Case 1: Simple 'ls .' ---");
        String cmd1 = createCommandString("%s %s", "ls", ".");
        System.out.println("Executing: " + cmd1);
        executeLsCommand(cmd1);
        System.out.println();

        System.out.println("--- Test Case 2: 'ls -l' with an option ---");
        String cmd2 = createCommandString("%s %s", "ls", "-l");
        System.out.println("Executing: " + cmd2);
        executeLsCommand(cmd2);
        System.out.println();

        System.out.println("--- Test Case 3: 'ls' on a non-existent directory ---");
        String cmd3 = createCommandString("%s %s", "ls", "non_existent_dir");
        System.out.println("Executing: " + cmd3);
        executeLsCommand(cmd3);
        System.out.println();

        System.out.println("--- Test Case 4: Malicious command injection attempt (should be blocked) ---");
        String cmd4 = createCommandString("%s %s", "ls", ".;whoami");
        System.out.println("Executing: " + cmd4);
        executeLsCommand(cmd4);
        System.out.println();

        System.out.println("--- Test Case 5: Different command attempt (should be blocked) ---");
        String cmd5 = createCommandString("%s", "whoami");
        System.out.println("Executing: " + cmd5);
        executeLsCommand(cmd5);
        System.out.println();
    }
}