import java.io.IOException;
import java.util.Arrays;
import java.util.List;

public class Lse110 {

    /**
     * Runs the 'id <username>' command securely and returns its exit status.
     *
     * @param username The username to look up.
     * @return The exit status of the 'id' command. 0 for success, non-zero for failure.
     *         Returns -1 if an internal error like IOException occurs.
     */
    public static int runIdCommand(String username) {
        // Validate input to prevent command injection with empty or null strings
        if (username == null || username.trim().isEmpty()) {
            System.err.println("Error: Username cannot be null or empty.");
            return -1;
        }

        // Using ProcessBuilder is the secure way to run external commands.
        // It treats the command and its arguments separately, preventing shell injection.
        ProcessBuilder pb = new ProcessBuilder("id", username);

        try {
            // Start the process
            Process process = pb.start();

            // Wait for the process to complete and get its exit code
            int exitCode = process.waitFor();
            return exitCode;

        } catch (IOException | InterruptedException e) {
            System.err.println("Error executing command: " + e.getMessage());
            // Restore the interrupted status
            if (e instanceof InterruptedException) {
                Thread.currentThread().interrupt();
            }
            return -1; // Indicate an exception occurred
        }
    }

    public static void main(String[] args) {
        // 5 test cases
        List<String> testUsernames = Arrays.asList(
            "root",                 // A user that usually exists
            "nobody",               // Another user that often exists
            "nonexistentuser12345", // A user that should not exist
            "; whoami",             // A potential command injection attempt
            "test user"             // A username with a space
        );

        System.out.println("--- Running 5 Test Cases ---");
        for (String user : testUsernames) {
            System.out.println("Testing username: '" + user + "'");
            int exitStatus = runIdCommand(user);
            System.out.println("Exit Status: " + exitStatus + "\n");
        }

        // Additionally, handle command line argument as per prompt description
        if (args.length > 0) {
            System.out.println("--- Running with command line argument ---");
            String userFromArg = args[0];
            System.out.println("Testing username: '" + userFromArg + "'");
            int exitStatus = runIdCommand(userFromArg);
            System.out.println("Exit Status: " + exitStatus);
        }
    }
}