import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class Task158 {

    /**
     * Validates that the input contains only characters that are safe for a hostname or IP address.
     * This is a critical defense-in-depth measure that uses an "allow-list" approach
     * to prevent characters that could be used for command injection (e.g., ';', '|', '&', '$').
     *
     * @param input The string to validate.
     * @return true if the input is valid, false otherwise.
     */
    private static boolean isValidInput(String input) {
        if (input == null || input.trim().isEmpty()) {
            return false;
        }
        // Allow alphanumeric characters, dots, and hyphens.
        return Pattern.matches("^[a-zA-Z0-9.-]+$", input);
    }

    /**
     * Executes an OS command (ping) securely using user-provided input.
     * It uses ProcessBuilder to pass arguments safely, which prevents command injection.
     *
     * @param userInput The destination (hostname or IP) to be pinged.
     */
    public static void secureOsCommand(String userInput) {
        System.out.println("--- Executing command for input: '" + userInput + "' ---");

        if (!isValidInput(userInput)) {
            System.out.println("Invalid input format. Aborting command execution.");
            System.out.println("-------------------------------------------------");
            return;
        }

        try {
            List<String> command = new ArrayList<>();
            String os = System.getProperty("os.name").toLowerCase();

            command.add("ping");
            // Platform-specific arguments for ping count to prevent flooding
            if (os.contains("win")) {
                command.add("-n");
            } else {
                command.add("-c");
            }
            command.add("4");
            // The user input is added as a separate, single argument.
            // ProcessBuilder ensures it is treated as a literal string and not interpreted by a shell.
            command.add(userInput);

            ProcessBuilder pb = new ProcessBuilder(command);
            pb.redirectErrorStream(true); // Combine stdout and stderr for easier reading
            Process process = pb.start();

            // Read the output from the command
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    System.out.println(line);
                }
            }

            int exitCode = process.waitFor();
            System.out.println("\nExited with code: " + exitCode);

        } catch (IOException | InterruptedException e) {
            System.err.println("An error occurred during command execution: " + e.getMessage());
        } finally {
            System.out.println("-------------------------------------------------");
        }
    }

    public static void main(String[] args) {
        String[] testCases = {
            "127.0.0.1",              // Valid IP address
            "google.com",             // Valid hostname
            "invalid-hostname!",      // Invalid character, should be rejected by validation
            "127.0.0.1; ls",          // Classic command injection, should be rejected by validation
            "$(reboot)"               // Command substitution, should be rejected by validation
        };

        for (String testCase : testCases) {
            secureOsCommand(testCase);
        }
    }
}