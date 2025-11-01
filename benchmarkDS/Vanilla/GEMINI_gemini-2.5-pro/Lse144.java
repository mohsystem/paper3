import java.util.HashSet;
import java.util.Set;

public class Lse144 {

    /**
     * Simulates a script that unsubscribes an email.
     * @param email The email to unsubscribe.
     * @return 0 for success, non-zero for failure.
     */
    private static int unsubscribeScript(String email) {
        System.out.println("-> Calling unsubscribe script for: " + email);
        // Simulate success for emails containing "success", failure otherwise.
        if (email.contains("success")) {
            System.out.println("-> Script returned 0 (success).");
            return 0;
        } else {
            System.out.println("-> Script returned 1 (failure).");
            return 1;
        }
    }

    /**
     * Checks if an email exists and attempts to unsubscribe if it does.
     * @param email The email to check.
     * @param database A set representing the database of emails.
     * @return 1 if email exists and unsubscribe is successful, 0 otherwise.
     */
    public static int checkAndUnsubscribe(String email, Set<String> database) {
        System.out.println("Checking for email: " + email);
        if (!database.contains(email)) {
            System.out.println("Email not found in database.");
            return 0; // Email doesn't exist
        }

        System.out.println("Email found in database.");
        if (unsubscribeScript(email) == 0) {
            return 1; // Unsubscribe script was successful
        } else {
            return 0; // Unsubscribe script failed
        }
    }

    public static void main(String[] args) {
        // Mock database
        Set<String> emailDatabase = new HashSet<>();
        emailDatabase.add("user.success@example.com");
        emailDatabase.add("user.fail@example.com");
        emailDatabase.add("another.success@example.com");

        // Test cases
        String[] testEmails = {
            "not.in.db@example.com",        // Test Case 1: Email not in DB
            "user.success@example.com",     // Test Case 2: Email in DB, script succeeds
            "user.fail@example.com",        // Test Case 3: Email in DB, script fails
            "another.success@example.com",  // Test Case 4: Another email in DB, script succeeds
            "another.not.in.db@email.com"   // Test Case 5: Another email not in DB
        };

        for (int i = 0; i < testEmails.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            int result = checkAndUnsubscribe(testEmails[i], emailDatabase);
            System.out.println("Final Result: " + result);
            System.out.println();
        }
    }
}