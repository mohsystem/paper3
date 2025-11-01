import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.util.regex.Pattern;

public class Lse108 {

    // A regex pattern for a typical and safe Linux username.
    // It starts with a letter or underscore, followed by letters, numbers, underscores, or hyphens.
    // This strict validation is the primary defense against command injection.
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-z_][a-z0-9_-]{0,31}$");

    /**
     * Executes the 'getent passwd <username>' command securely.
     * It validates the username against a strict pattern and uses ProcessBuilder
     * to prevent shell command injection vulnerabilities.
     *
     * @param username The username to look up. It must conform to the validation pattern.
     * @return The output of the command, or an error message if the username is invalid or an execution error occurs.
     */
    public static String executeGetent(String username) {
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            return "Error: Invalid username format.";
        }

        try {
            // Use ProcessBuilder to execute the command without a shell.
            // Arguments are passed separately, which prevents them from being interpreted by a shell.
            ProcessBuilder pb = new ProcessBuilder("getent", "passwd", username);
            Process process = pb.start();

            StringBuilder output = new StringBuilder();
            // Read the standard output of the command.
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    output.append(line).append(System.lineSeparator());
                }
            }

            int exitCode = process.waitFor();
            
            // 'getent' exits with 2 if the user is not found. An empty output is a good indicator.
            if (exitCode != 0 && output.length() == 0) {
                 return "User not found or an error occurred.";
            }

            return output.toString();

        } catch (IOException | InterruptedException e) {
            // Restore the interrupted status if an InterruptedException occurs.
            if (e instanceof InterruptedException) {
                Thread.currentThread().interrupt();
            }
            return "Error executing command: " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        String[] testUsernames = {
            "root",                                 // Test Case 1: Valid and existing user
            "nobody",                               // Test Case 2: Another valid user
            "thisuserdoesnotexist123",              // Test Case 3: Valid format, but likely non-existent
            "user;whoami",                          // Test Case 4: Command injection attempt (should be rejected)
            "us$er"                                 // Test Case 5: Invalid character (should be rejected)
        };

        for (String user : testUsernames) {
            System.out.println("--- Testing with username: '" + user + "' ---");
            String result = executeGetent(user);
            System.out.println("Output:");
            System.out.print(result);
            System.out.println("----------------------------------------\n");
        }
    }
}