import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

public class Lse144 {

    // A mock database of email addresses. In a real application, this would be a connection to a database.
    private static final Set<String> emailDatabase = new HashSet<>(Arrays.asList(
            "user1@example.com",
            "success@example.com",
            "fail@example.com"
    ));

    // A simple regex for email validation based on common patterns.
    // This is a basic check and may not cover all edge cases of RFC 5322.
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
            "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,6}$"
    );
    private static final int MAX_EMAIL_LENGTH = 254;

    /**
     * Validates an email address format and length.
     * @param email The email string to validate.
     * @return true if the email is valid, false otherwise.
     */
    private boolean isValidEmail(String email) {
        if (email == null || email.trim().isEmpty() || email.length() > MAX_EMAIL_LENGTH) {
            return false;
        }
        return EMAIL_PATTERN.matcher(email).matches();
    }

    /**
     * Checks if an email exists in the mock database.
     * @param email The email to check.
     * @return true if the email exists, false otherwise.
     */
    private boolean emailExistsInDB(String email) {
        // In a real application, this would query a database.
        return emailDatabase.contains(email);
    }

    /**
     * Simulates calling an external script to unsubscribe an email.
     * @param email The email to unsubscribe.
     * @return 0 for success, non-zero for failure.
     */
    private int callUnsubscribeScript(String email) {
        System.out.println("Simulating call to unsubscribe script for: " + email);
        // Security Note: In a real-world application, never build a command string by concatenating user input.
        // Use ProcessBuilder to pass arguments safely, which prevents command injection.
        // try {
        //     ProcessBuilder pb = new ProcessBuilder("/path/to/unsubscribe.sh", email);
        //     Process p = pb.start();
        //     int exitCode = p.waitFor();
        //     return exitCode;
        // } catch (java.io.IOException | InterruptedException e) {
        //     // Log the exception
        //     return 1; // Indicate failure
        // }

        // Mock logic: script fails for a specific email for testing purposes.
        if ("fail@example.com".equals(email)) {
            System.out.println("Mock script simulation: FAILED.");
            return 1; // Simulate failure
        }

        System.out.println("Mock script simulation: SUCCESS.");
        return 0; // Simulate success
    }

    /**
     * Processes an unsubscribe request based on the specified logic.
     * @param email The email address to process.
     * @return 1 if the email exists and is successfully unsubscribed, 0 otherwise.
     */
    public int processUnsubscribeRequest(String email) {
        // 1. Rule#1: Ensure all input is validated.
        if (!isValidEmail(email)) {
            // Invalid input, so we return 0 as per the logic for non-existent emails.
            return 0;
        }

        // 2. Check if email exists in the database.
        if (emailExistsInDB(email)) {
            // 3. If it exists, call the unsubscribe script.
            int scriptResult = callUnsubscribeScript(email);

            // 4. If the script returns 0, it was a success.
            if (scriptResult == 0) {
                return 1; // Success
            } else {
                return 0; // Script failed
            }
        } else {
            // Email does not exist.
            return 0;
        }
    }

    public static void main(String[] args) {
        Lse144 processor = new Lse144();
        String[] testEmails = {
            "success@example.com", // Case 1: Exists, script succeeds -> should return 1
            "fail@example.com",    // Case 2: Exists, script fails -> should return 0
            "notfound@example.com",// Case 3: Does not exist -> should return 0
            "invalid-email",       // Case 4: Invalid format -> should return 0
            ""                     // Case 5: Empty string -> should return 0
        };

        for (String email : testEmails) {
            System.out.println("\n--- Processing test case for: '" + email + "' ---");
            int result = processor.processUnsubscribeRequest(email);
            System.out.println("Result for '" + email + "': " + result);
            System.out.println("----------------------------------------");
        }
    }
}