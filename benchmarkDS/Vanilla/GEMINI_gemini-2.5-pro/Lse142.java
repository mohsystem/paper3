import java.util.HashSet;
import java.util.Set;

public class Lse142 {

    /**
     * Checks if an email is in the database and removes it if found.
     *
     * @param database A Set of strings representing the email database.
     * @param email The email address to unsubscribe.
     * @return A string indicating the result of the operation.
     */
    public static String unsubscribe(Set<String> database, String email) {
        if (database.contains(email)) {
            database.remove(email);
            return email + " has been unsubscribed successfully.";
        } else {
            return email + " is not in the database.";
        }
    }

    public static void main(String[] args) {
        // Simulate a database with a HashSet
        Set<String> emailDatabase = new HashSet<>();
        emailDatabase.add("user1@example.com");
        emailDatabase.add("user2@example.com");
        emailDatabase.add("user3@example.com");

        System.out.println("Initial database: " + emailDatabase);

        // --- 5 Test Cases ---
        String[] testEmails = {
            "user2@example.com",     // Case 1: Unsubscribe an existing email.
            "user4@example.com",     // Case 2: Try to unsubscribe an email that does not exist.
            "user1@example.com",     // Case 3: Unsubscribe another existing email.
            "user2@example.com",     // Case 4: Try to unsubscribe the first email again.
            "user3@example.com"      // Case 5: Unsubscribe the last remaining email.
        };

        for (int i = 0; i < testEmails.length; i++) {
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            String currentEmail = testEmails[i];
            System.out.println("Attempting to unsubscribe: " + currentEmail);
            String result = unsubscribe(emailDatabase, currentEmail);
            System.out.println("Result: " + result);
            System.out.println("Current database: " + emailDatabase);
        }
    }
}