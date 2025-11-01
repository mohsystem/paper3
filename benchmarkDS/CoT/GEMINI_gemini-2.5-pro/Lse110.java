import java.io.IOException;

public class Lse110 {

    /**
     * Runs the 'id <username>' command in a separate process.
     * This method is secure against command injection because it passes the command
     * and its arguments separately to the ProcessBuilder, avoiding shell interpretation.
     *
     * @param username The username to look up.
     * @return The exit status of the 'id' command. 0 for success, non-zero for failure.
     */
    public static int runIdCommand(String username) {
        if (username == null || username.isEmpty()) {
            System.err.println("Error: Username cannot be null or empty.");
            return -1; // Indicate an error before even trying to run the command
        }

        // Using ProcessBuilder is the secure way to execute commands with arguments.
        // It avoids command injection by not invoking a shell.
        ProcessBuilder processBuilder = new ProcessBuilder("id", username);
        
        // Redirect the subprocess's output and error streams to the current process's streams.
        // This makes the output of the 'id' command appear in the console.
        processBuilder.inheritIO();

        try {
            Process process = processBuilder.start();
            // Wait for the command to complete and get its exit code.
            return process.waitFor();
        } catch (IOException | InterruptedException e) {
            // Handle exceptions that may occur during process execution.
            System.err.println("Error executing command: " + e.getMessage());
            // It's common to return a specific non-zero value for such errors.
            return -1;
        }
    }

    public static void main(String[] args) {
        if (args.length > 0) {
            // If a command-line argument is provided, use it as the username.
            String username = args[0];
            System.out.println("Running command: id " + username);
            int exitStatus = runIdCommand(username);
            System.out.println("Exit Status: " + exitStatus);
        } else {
            // If no arguments are provided, run a set of test cases.
            System.out.println("No command-line argument provided. Running test cases...");
            String[] testUsernames = {
                "root",                                   // 1. Should succeed (exit 0) on most Unix-like systems.
                "nobody",                                 // 2. Should succeed (exit 0) on most Unix-like systems.
                "non_existent_user_12345",                // 3. Should fail (exit non-zero).
                "root;whoami",                            // 4. Malicious input; should be treated as a single, invalid username and fail.
                ""                                        // 5. Empty string, should be handled and fail.
            };

            for (String user : testUsernames) {
                System.out.println("\n------------------------------------");
                System.out.println("Testing with username: \"" + user + "\"");
                int exitStatus = runIdCommand(user);
                System.out.println("Exit Status: " + exitStatus);
                System.out.println("------------------------------------");
            }
        }
    }
}