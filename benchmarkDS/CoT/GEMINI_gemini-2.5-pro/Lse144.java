import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class Lse144 {

    // Simulates a database of subscribed emails for demonstration purposes.
    private static final Set<String> emailDatabase = new HashSet<>(Arrays.asList(
            "user.success@example.com",
            "user.fail@example.com",
            "another.success@example.com",
            "test.user@domain.com"
    ));

    /**
     * Simulates an external script that unsubscribes an email.
     * In a real-world scenario, this would be a call to an external service or script.
     * Calling a function is much safer than using Runtime.exec() or ProcessBuilder,
     * which can be vulnerable to command injection if not handled properly.
     *
     * @param email The email to unsubscribe.
     * @return 0 for success, 1 for failure.
     */
    private static int mockUnsubscribeScript(String email) {
        // This is a mock implementation.
        // It succeeds if the email contains "success" or "test", and fails otherwise.
        if (email != null && (email.contains("success") || email.contains("test"))) {
            System.out.println("-> Mock script: Successfully unsubscribed " + email);
            return 0; // Success
        } else {
            System.out.println("-> Mock script: Failed to unsubscribe " + email);
            return 1; // Failure
        }
    }

    /**
     * Checks if an email exists and attempts to unsubscribe it.
     *
     * @param email The email address to check and unsubscribe.
     * @return 1 if the email exists and the unsubscribe script succeeds, otherwise 0.
     */
    public static int unsubscribeUser(String email) {
        // Input validation: ensure email is not null or empty.
        if (email == null || email.trim().isEmpty()) {
            System.out.println("Processing '" + email + "': Invalid input.");
            return 0;
        }

        System.out.println("Processing '" + email + "':");

        // Step 1: Check if the email exists in the database.
        if (emailDatabase.contains(email)) {
            System.out.println("-> Email found in the database.");
            // Step 2: If it exists, call the unsubscribe script.
            int scriptResult = mockUnsubscribeScript(email);

            // Step 3: Check the script's return value.
            if (scriptResult == 0) {
                // Script succeeded, return 1.
                System.out.println("-> Result: 1 (Success)");
                return 1;
            } else {
                // Script failed, return 0.
                System.out.println("-> Result: 0 (Script Failed)");
                return 0;
            }
        } else {
            // Email does not exist, return 0.
            System.out.println("-> Email not found in the database.");
            System.out.println("-> Result: 0");
            return 0;
        }
    }

    public static void main(String[] args) {
        String[] testEmails = {
            "user.success@example.com",  // Test case 1: Email exists, script succeeds
            "user.fail@example.com",     // Test case 2: Email exists, script fails
            "not.found@example.com",     // Test case 3: Email does not exist
            "test.user@domain.com",      // Test case 4: Another success case
            null                         // Test case 5: Invalid null input
        };

        for (int i = 0; i < testEmails.length; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            int result = unsubscribeUser(testEmails[i]);
            System.out.println("Final return value: " + result);
        }
    }
}