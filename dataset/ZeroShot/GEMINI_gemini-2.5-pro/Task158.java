import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class Task158 {

    /**
     * Executes an OS command securely using user-provided data.
     * This method avoids command injection by passing the command and its arguments
     * as a list of strings to ProcessBuilder. The shell is not invoked, and the
     * userInput is treated as a single, literal argument.
     *
     * @param userInput The data provided by the user, to be used as an argument.
     */
    public static void secureExecuteCommand(String userInput) {
        System.out.println("--- Executing command with input: \"" + userInput + "\" ---");

        // Basic validation: prevent empty or null input from being processed.
        if (userInput == null || userInput.trim().isEmpty()) {
            System.out.println("Error: Input cannot be null or empty.");
            System.out.println("--------------------------------------------------\n");
            return;
        }

        // Determine the ping command based on the operating system.
        String os = System.getProperty("os.name").toLowerCase();
        List<String> command = new ArrayList<>();
        if (os.contains("win")) {
            command.add("ping");
            command.add("-n"); // Count option for Windows
            command.add("1");
        } else {
            command.add("ping");
            command.add("-c"); // Count option for Linux/macOS
            command.add("1");
        }
        // The user input is added as the final argument. It will be treated as a
        // single, literal string and not be interpreted by a shell.
        command.add(userInput);

        try {
            ProcessBuilder pb = new ProcessBuilder(command);
            // Redirect error stream to standard output stream to see all output
            pb.redirectErrorStream(true); 
            Process process = pb.start();

            // Read the output from the command
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    System.out.println(line);
                }
            }

            int exitCode = process.waitFor();
            System.out.println("Exit Code: " + exitCode);

        } catch (IOException | InterruptedException e) {
            System.err.println("Error executing command: " + e.getMessage());
            e.printStackTrace();
        }
        System.out.println("--------------------------------------------------\n");
    }

    public static void main(String[] args) {
        // Test Case 1: Benign, valid input
        secureExecuteCommand("8.8.8.8");

        // Test Case 2: Another benign, valid input
        secureExecuteCommand("localhost");

        // Test Case 3: Malicious input attempt (Unix style)
        // This will fail to ping a host named "127.0.0.1; ls" but will NOT execute "ls"
        secureExecuteCommand("127.0.0.1; ls");

        // Test Case 4: Malicious input attempt (Windows style)
        // This will fail to ping a host named "127.0.0.1 && dir" but will NOT execute "dir"
        secureExecuteCommand("127.0.0.1 && dir");

        // Test Case 5: Input with spaces that should be treated as a single argument
        // This will likely fail, but demonstrates it's not splitting the argument.
        secureExecuteCommand("not a valid hostname");
    }
}