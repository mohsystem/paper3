import java.util.HashSet;
import java.util.Set;
import java.util.Arrays;

public class Lse142 {

    /**
     * Simulates unsubscribing an email address from a database.
     * The database is represented by a Set of strings.
     *
     * @param database The set of subscribed email addresses. This set will be modified.
     * @param email The email address to unsubscribe.
     * @return A status message indicating success or failure.
     */
    public static String unsubscribe(Set<String> database, String email) {
        // Basic input validation
        if (email == null || email.trim().isEmpty()) {
            return "Invalid email address provided.";
        }
        
        // Normalize email to lowercase for case-insensitive comparison
        String normalizedEmail = email.toLowerCase();
        
        // Check if the email is in the database and remove it
        if (database.contains(normalizedEmail)) {
            database.remove(normalizedEmail);
            return "Successfully unsubscribed " + email + ".";
        } else {
            return "Email address " + email + " not found in the database.";
        }
    }

    public static void main(String[] args) {
        // 1. Initialize the simulated database with lowercase emails
        Set<String> emailDatabase = new HashSet<>(Arrays.asList(
            "user1@example.com",
            "user2@example.com",
            "subscriber@test.com",
            "another.user@domain.org"
        ));
        
        System.out.println("Initial database: " + emailDatabase);

        // 2. Define test cases
        String[] testEmails = {
            "user1@example.com",      // Case 1: Email exists and should be removed.
            "user99@example.com",     // Case 2: Email does not exist.
            "user1@example.com",      // Case 3: Email should now be gone after the first test.
            "SUBSCRIBER@TEST.COM",  // Case 4: Test case-insensitivity.
            "invalid-email"           // Case 5: An invalid/non-existent email format.
        };

        // 3. Run test cases
        for (String email : testEmails) {
            System.out.println("\nAttempting to unsubscribe: " + email);
            String result = unsubscribe(emailDatabase, email);
            System.out.println("Result: " + result);
            System.out.println("Current database: " + emailDatabase);
        }
    }
}