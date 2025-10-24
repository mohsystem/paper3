import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.TimeUnit;

public class Task66 {
    // 1. A whitelist of allowed commands is the most effective security measure.
    private static final Set<String> ALLOWED_COMMANDS = new HashSet<>(Arrays.asList("ls", "pwd", "date", "echo"));

    /**
     * Executes a command securely after validation.
     * @param command The command string to execute.
     * @return The output of the command or an error message.
     */
    public static String executeSecureCommand(String command) {
        if (command == null || command.trim().isEmpty()) {
            return "Error: Command cannot be null or empty.";
        }

        // A simple split on whitespace. This avoids complex parsing that could be exploited.
        // It means arguments with spaces must be handled carefully or disallowed.
        String[] parts = command.trim().split("\\s+");
        String baseCommand = parts[0];

        // 2. Validate command against the allowlist.
        if (!ALLOWED_COMMANDS.contains(baseCommand)) {
            return "Error: Command '" + baseCommand + "' is not allowed.";
        }

        // 3. Validate arguments for insecure patterns.
        for (int i = 1; i < parts.length; i++) {
            String arg = parts[i];
            // Disallow path traversal and common shell metacharacters.
            if (arg.contains("..") || arg.matches(".*[;&|<>`$()\\{\\}].*")) {
                return "Error: Argument '" + arg + "' contains forbidden characters or patterns.";
            }
        }

        try {
            // Using ProcessBuilder is secure as it does not invoke a shell by default.
            ProcessBuilder pb = new ProcessBuilder(parts);
            pb.redirectErrorStream(true); // Combine stdout and stderr for simplicity.

            Process process = pb.start();

            StringBuilder output = new StringBuilder();
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append("\n");
                }
            }

            // 4. Add a timeout to prevent long-running processes (resource exhaustion).
            if (!process.waitFor(5, TimeUnit.SECONDS)) {
                process.destroyForcibly();
                return "Error: Command timed out.";
            }

            int exitCode = process.exitValue();
            if (exitCode != 0) {
                 return "Error: Command exited with code " + exitCode + ".\nOutput:\n" + output.toString();
            }

            return output.toString();

        } catch (IOException | InterruptedException e) {
            // Restore interrupted status
            if (e instanceof InterruptedException) {
                Thread.currentThread().interrupt();
            }
            return "Error executing command: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        String[] testCommands = {
            "ls -l",                // 1. Valid command
            "echo Hello World",     // 2. Valid command with arguments
            "whoami",               // 3. Invalid command (not in allowlist)
            "ls; rm -rf /",         // 4. Injection attempt (';' will be seen as part of 'ls;' argument and rejected)
            "ls ../../"             // 5. Path traversal attempt
        };

        for (String cmd : testCommands) {
            System.out.println("Executing: '" + cmd + "'");
            System.out.println("-------------------------");
            String result = executeSecureCommand(cmd);
            System.out.println("Result:\n" + result);
            System.out.println("=========================");
        }
    }
}