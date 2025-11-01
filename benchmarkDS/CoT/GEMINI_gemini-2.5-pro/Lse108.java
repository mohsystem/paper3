import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.regex.Pattern;

public class Lse108 {

    /**
     * Validates a username to ensure it contains only safe characters
     * and is not empty. This prevents command injection.
     * Allowed characters are: a-z, A-Z, 0-9, underscore, and hyphen.
     * Usernames must be between 1 and 32 characters long.
     *
     * @param username The username string to validate.
     * @return true if the username is valid, false otherwise.
     */
    private static boolean isValidUsername(String username) {
        if (username == null || username.trim().isEmpty()) {
            return false;
        }
        // A regex for typical Linux usernames.
        // It must not contain shell metacharacters.
        String regex = "^[a-zA-Z0-9_-]{1,32}$";
        return Pattern.matches(regex, username);
    }

    /**
     * Executes the command 'getent passwd <username>' and returns its output.
     *
     * @param username The username to look up.
     * @return The output of the command, or an error message if the input is invalid or an error occurs.
     */
    public static String getUserInfo(String username) {
        if (!isValidUsername(username)) {
            return "Error: Invalid username format.";
        }

        // Using ProcessBuilder is the recommended and secure way to execute external commands.
        // It treats arguments separately, preventing shell injection.
        ProcessBuilder processBuilder = new ProcessBuilder("getent", "passwd", username);
        processBuilder.redirectErrorStream(true); // Combine stdout and stderr

        try {
            Process process = processBuilder.start();
            StringBuilder output = new StringBuilder();

            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append("\n");
                }
            }

            int exitCode = process.waitFor();
            // Even with exitCode != 0, getent might print to stdout (e.g., nothing), so we return the captured output.
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
        String[] testUsernames = {
            "root",                 // Test Case 1: A valid, existing user
            "nonexistentuser12345", // Test Case 2: A valid but likely non-existing user
            "user name",            // Test Case 3: Invalid format (space)
            "root;whoami",          // Test Case 4: Command injection attempt
            ""                      // Test Case 5: Empty username
        };

        for (String user : testUsernames) {
            System.out.println("---- Testing for username: '" + user + "' ----");
            String result = getUserInfo(user);
            if (result.isEmpty()) {
                System.out.println("No output received (user may not exist or command failed silently).");
            } else {
                System.out.print(result);
            }
            System.out.println("----------------------------------------\n");
        }
    }
}