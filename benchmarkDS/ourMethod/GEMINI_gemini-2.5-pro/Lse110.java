import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;

public class Lse110 {

    // Regex for a safe, typical POSIX username.
    // Starts with a letter or underscore, followed by letters, digits, underscores, hyphens, or dots.
    // Length is between 1 and 32 characters.
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z_][a-zA-Z0-9_.-]{0,31}$");

    /**
     * Executes the 'id' command for a given username in a secure way.
     *
     * @param username The username to look up.
     * @return The exit code of the 'id' command, or a negative value on error.
     *         -1: Invalid username format.
     *         -2: Command timed out.
     *         -3: Execution error (e.g., IOException, InterruptedException).
     */
    public static int executeIdCommand(String username) {
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            System.err.println("Error: Invalid username format for '" + username + "'.");
            return -1; // Indicate invalid input
        }

        try {
            // Using ProcessBuilder with a list of arguments prevents shell injection.
            ProcessBuilder pb = new ProcessBuilder("id", username);
            
            // It's good practice to handle the output streams to prevent the process buffer from filling up.
            // InheritIO redirects the subprocess's standard output and error to the current process's streams.
            pb.inheritIO();
            
            Process process = pb.start();

            // Wait for the process to complete, with a timeout for robustness.
            if (!process.waitFor(10, TimeUnit.SECONDS)) {
                System.err.println("Error: Command timed out for username '" + username + "'.");
                process.destroyForcibly();
                return -2; // Indicate timeout
            }
            
            return process.exitValue();
        } catch (Exception e) {
            // Catches InterruptedException, IOException, etc.
            System.err.println("An error occurred while executing the command: " + e.getMessage());
            return -3; // Indicate execution error
        }
    }

    public static void main(String[] args) {
        // This part handles a username passed as a command-line argument.
        if (args.length > 0) {
            System.out.println("--- Running with command-line argument ---");
            String username = args[0];
            System.out.println("Running command for username: " + username);
            int exitStatus = executeIdCommand(username);
            System.out.println("Exit Status: " + exitStatus);
            System.out.println("----------------------------------------\n");
        }
        
        // This part runs 5 hardcoded test cases as requested.
        System.out.println("--- Running 5 Hardcoded Test Cases ---");
        String[] testUsernames = {
            "root",                 // Test case 1: Valid, existing user
            "daemon",               // Test case 2: Another valid, existing user
            "nonexistentuser12345", // Test case 3: Valid format, but non-existent user
            "invalid;user",         // Test case 4: Invalid format (command injection attempt)
            " a bad user"           // Test case 5: Invalid format (contains spaces)
        };

        for (int i = 0; i < testUsernames.length; i++) {
            System.out.println("Test Case " + (i + 1) + ": Running command for username '" + testUsernames[i] + "'");
            int exitStatus = executeIdCommand(testUsernames[i]);
            System.out.println("Exit Status: " + exitStatus);
            System.out.println("----------------------------------------");
        }
    }
}