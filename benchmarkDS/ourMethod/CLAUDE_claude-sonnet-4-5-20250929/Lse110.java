
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Pattern;

public class Lse110 {
    // Whitelist pattern: alphanumeric, underscore, hyphen only
    // This prevents command injection through special shell characters
    private static final Pattern SAFE_USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_-]+$");
    private static final int MAX_USERNAME_LENGTH = 32;

    /**
     * Executes the id command for a given username.
     * Returns the exit status of the command.
     * 
     * @param username The username to query (must match safe pattern)
     * @return Exit status: 0 for success, non-zero for failure, -1 for invalid input
     */
    public static int executeIdCommand(String username) {
        // Input validation: check for null or empty
        if (username == null || username.isEmpty()) {
            System.err.println("Error: Username cannot be null or empty");
            return -1;
        }

        // Input validation: check length to prevent buffer issues
        if (username.length() > MAX_USERNAME_LENGTH) {
            System.err.println("Error: Username exceeds maximum length");
            return -1;
        }

        // Input validation: sanitize username to prevent command injection
        // Only allow alphanumeric characters, underscore, and hyphen
        if (!SAFE_USERNAME_PATTERN.matcher(username).matches()) {
            System.err.println("Error: Username contains invalid characters");
            return -1;
        }

        ProcessBuilder processBuilder = new ProcessBuilder();
        
        // Use ProcessBuilder with argument list instead of shell execution
        // This prevents command injection by treating username as a single argument
        List<String> command = new ArrayList<>();
        command.add("id");
        command.add(username);
        processBuilder.command(command);

        // Disable shell interpretation to prevent injection attacks
        processBuilder.redirectErrorStream(true);

        try {
            Process process = processBuilder.start();
            
            // Read output (though we mainly care about exit status here)
            BufferedReader reader = new BufferedReader(
                new InputStreamReader(process.getInputStream())
            );
            
            String line;
            while ((line = reader.readLine()) != null) {
                // Output is displayed but not logged to avoid potential information leakage
                System.out.println(line);
            }

            // Wait for process to complete and get exit status
            int exitStatus = process.waitFor();
            return exitStatus;

        } catch (IOException e) {
            // Generic error message to avoid information leakage
            System.err.println("Error: Failed to execute command");
            return -1;
        } catch (InterruptedException e) {
            // Restore interrupted status
            Thread.currentThread().interrupt();
            System.err.println("Error: Command execution interrupted");
            return -1;
        }
    }

    public static void main(String[] args) {
        // Test cases
        String[] testUsers = {"root", "nobody", "testuser123", "user-name", "invalid@user"};
        
        if (args.length > 0) {
            // Use command line argument if provided
            String username = args[0];
            int exitStatus = executeIdCommand(username);
            System.out.println("Exit status: " + exitStatus);
        } else {
            // Run test cases
            System.out.println("Running test cases:");
            for (String user : testUsers) {
                System.out.println("\\n--- Testing with username: " + user + " ---");
                int exitStatus = executeIdCommand(user);
                System.out.println("Exit status: " + exitStatus);
            }
        }
    }
}
