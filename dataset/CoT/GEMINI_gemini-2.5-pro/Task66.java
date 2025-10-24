import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.TimeUnit;

public class Task66 {

    // 1. Validation: Use an allowlist of safe commands.
    private static final Set<String> ALLOWED_COMMANDS = new HashSet<>(Arrays.asList(
            "ls", "echo", "date", "pwd", "whoami"
    ));

    /**
     * Validates, executes a shell command, and returns its output.
     * The command is validated against a strict allowlist.
     * It uses ProcessBuilder with a command array to prevent command injection.
     *
     * @param command The command string to execute (e.g., "ls -l").
     * @return The standard output and standard error from the command, or an error message.
     */
    public static String executeCommand(String command) {
        if (command == null || command.trim().isEmpty()) {
            return "Error: Command cannot be null or empty.";
        }

        // 2. Secure Parsing: Split command into arguments without using a shell.
        String[] commandParts = command.trim().split("\\s+");
        String baseCommand = commandParts[0];

        // 3. Validation against Allowlist
        if (!ALLOWED_COMMANDS.contains(baseCommand)) {
            return "Error: Command '" + baseCommand + "' is not allowed.";
        }

        StringBuilder output = new StringBuilder();
        try {
            // 4. Secure Execution: ProcessBuilder avoids shell interpretation.
            ProcessBuilder builder = new ProcessBuilder(commandParts);
            // Redirect error stream to standard output to capture all output
            builder.redirectErrorStream(true);

            Process process = builder.start();

            // 5. Output Capture
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append("\n");
                }
            }

            // 6. Wait for the process to complete with a timeout
            if (!process.waitFor(10, TimeUnit.SECONDS)) {
                process.destroy();
                return "Error: Command timed out.";
            }

            int exitCode = process.exitValue();
            if (exitCode != 0) {
                output.append("Command exited with non-zero status: ").append(exitCode);
            }

        } catch (IOException e) {
            return "Error executing command: " + e.getMessage();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt(); // Preserve interrupted status
            return "Error: Command execution was interrupted.";
        }

        return output.toString();
    }

    public static void main(String[] args) {
        // --- Test Cases ---

        // Test Case 1: Valid and allowed command
        System.out.println("--- Test Case 1: ls -l ---");
        String test1 = "ls -l";
        System.out.println("Executing: " + test1);
        System.out.println("Result:\n" + executeCommand(test1));

        // Test Case 2: Valid command with arguments
        System.out.println("--- Test Case 2: echo Hello World ---");
        String test2 = "echo Hello Secure World";
        System.out.println("Executing: " + test2);
        System.out.println("Result:\n" + executeCommand(test2));

        // Test Case 3: Disallowed command (potential for harm)
        System.out.println("--- Test Case 3: rm -rf / ---");
        String test3 = "rm -rf /";
        System.out.println("Executing: " + test3);
        System.out.println("Result:\n" + executeCommand(test3));
        
        // Test Case 4: Command with shell metacharacters (attempted injection)
        System.out.println("--- Test Case 4: ls; whoami ---");
        String test4 = "ls; whoami";
        System.out.println("Executing: " + test4);
        System.out.println("Result:\n" + executeCommand(test4));

        // Test Case 5: Empty command
        System.out.println("--- Test Case 5: Empty Command ---");
        String test5 = "";
        System.out.println("Executing: \"\"");
        System.out.println("Result:\n" + executeCommand(test5));
    }
}