import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

public class Task66 {

    private static final Set<String> ALLOWED_COMMANDS = new HashSet<>(Arrays.asList(
            "ls", "echo", "date", "whoami"
    ));
    
    // Disallow shell metacharacters to prevent complex command injection scenarios
    private static final String SHELL_META_CHARS_PATTERN = "[;&|<>`$()]";


    /**
     * Validates and executes a shell command securely.
     *
     * @param command The command string to be executed.
     * @return The standard output and standard error of the command.
     */
    public static String executeCommand(String command) {
        if (command == null || command.trim().isEmpty()) {
            return "Error: Command cannot be null or empty.";
        }
        
        if (command.matches(".*" + SHELL_META_CHARS_PATTERN + ".*")) {
            return "Error: Command contains forbidden shell metacharacters.";
        }

        // Split the command into the program and its arguments.
        // This is a simple split by whitespace. For commands with quoted arguments,
        // a more sophisticated parser would be needed.
        List<String> commandParts = Arrays.asList(command.trim().split("\\s+"));
        if (commandParts.isEmpty()) {
            return "Error: Command is empty after trimming.";
        }

        String program = commandParts.get(0);
        if (!ALLOWED_COMMANDS.contains(program)) {
            return "Error: Command '" + program + "' is not allowed.";
        }
        
        try {
            ProcessBuilder processBuilder = new ProcessBuilder(commandParts);
            // Redirect error stream to the output stream to capture both
            processBuilder.redirectErrorStream(true);
            
            Process process = processBuilder.start();

            StringBuilder output = new StringBuilder();
            // Use try-with-resources to ensure the reader is closed
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append(System.lineSeparator());
                }
            }

            // Wait for the process to complete and check the exit code
            int exitCode = process.waitFor();
            output.append("Exited with code: ").append(exitCode);

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
            "ls -l",
            "echo hello from java",
            "grep root /etc/passwd", // Not in allowlist
            "ls; whoami",            // Contains metacharacter
            ""                       // Empty command
        };

        for (String cmd : testCommands) {
            System.out.println("Executing command: \"" + cmd + "\"");
            String result = executeCommand(cmd);
            System.out.println("Result:\n" + result);
            System.out.println("------------------------------------");
        }
    }
}