import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

public class Lse108 {

    // A pre-compiled pattern for valid usernames.
    // Allows alphanumeric characters, dot, underscore, and hyphen.
    // This strict validation is critical to prevent command injection.
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9._-]+$");

    /**
     * Executes the command 'getent passwd <username>' and returns its output.
     * The username is strictly validated to prevent command injection.
     * The prompt requires using a function like C's system(), so this implementation
     * invokes a shell using ProcessBuilder("sh", "-c", ...), which has similar risks
     * if input is not validated.
     *
     * @param username The username to look up.
     * @return The output of the command, or an error message if the input is invalid or an error occurs.
     */
    public static String getUserInfo(String username) {
        // Rule #1: Ensure all input is validated.
        if (username == null || username.isEmpty() || username.length() > 32) {
            return "Error: Username is invalid (null, empty, or too long).";
        }
        if (!USERNAME_PATTERN.matcher(username).matches()) {
            return "Error: Username contains invalid characters.";
        }

        // Rule #2: Use potentially dangerous functions with caution.
        // Invoking a shell is dangerous. Security relies on the strict validation above.
        String command = "getent passwd " + username;
        ProcessBuilder processBuilder = new ProcessBuilder("sh", "-c", command);
        
        try {
            Process process = processBuilder.start();

            // Capture standard output
            String output = new BufferedReader(
                new InputStreamReader(process.getInputStream(), StandardCharsets.UTF_8))
                .lines()
                .collect(Collectors.joining("\n"));

            // Capture standard error
            String errorOutput = new BufferedReader(
                new InputStreamReader(process.getErrorStream(), StandardCharsets.UTF_8))
                .lines()
                .collect(Collectors.joining("\n"));

            int exitCode = process.waitFor();

            if (exitCode == 0) {
                return output;
            } else {
                if (output.isEmpty()) {
                    // getent typically returns exit code 2 if the user is not found.
                    if (!errorOutput.isEmpty()) {
                        return "Error: " + errorOutput;
                    }
                    return "User not found or an error occurred (exit code: " + exitCode + ").";
                }
                // Return output even on error, as some info might be available.
                return output + "\nError stream: " + errorOutput;
            }

        } catch (IOException e) {
            return "Error: Failed to execute command. " + e.getMessage();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return "Error: Command execution was interrupted. " + e.getMessage();
        }
    }

    public static void main(String[] args) {
        String[] testUsernames = {
            "root",                 // Test case 1: Valid, existing user
            "nonexistentuser12345", // Test case 2: Valid, non-existing user
            "user; whoami",         // Test case 3: Invalid user with command injection attempt
            "",                     // Test case 4: Empty string
            "daemon"                // Test case 5: Another valid, existing user
        };

        for (String user : testUsernames) {
            System.out.println("--- Testing for username: '" + user + "' ---");
            String result = getUserInfo(user);
            System.out.println("Result:\n" + result);
            System.out.println("----------------------------------------\n");
        }
        
        System.out.println("--- Testing for username: null ---");
        String nullResult = getUserInfo(null);
        System.out.println("Result:\n" + nullResult);
        System.out.println("----------------------------------------\n");
    }
}