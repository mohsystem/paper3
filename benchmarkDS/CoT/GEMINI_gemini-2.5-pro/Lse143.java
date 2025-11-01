import java.util.HashSet;
import java.util.Set;

public class Lse143 {

    // Simulating a database of subscribed users with a static set.
    private static Set<String> database = new HashSet<>();

    /**
     * Unsubscribes a user by removing their email from the database.
     *
     * @param email The email address of the user to unsubscribe.
     * @return A message indicating the result of the operation.
     */
    public static String unsubscribe(String email) {
        // Security: Validate input to prevent processing null or empty data.
        if (email == null || email.trim().isEmpty()) {
            return "Invalid email address provided.";
        }

        // Check if the email is in the database.
        if (database.contains(email)) {
            // If it exists, remove it (unsubscribe).
            database.remove(email);
            return "User with email " + email + " has been successfully unsubscribed.";
        } else {
            // If it does not exist, return a message.
            return "User with email " + email + " is not subscribed.";
        }
    }

    public static void main(String[] args) {
        // Pre-populate the simulated database for testing.
        database.add("user1@example.com");
        database.add("user2@example.com");
        database.add("user3@example.com");

        System.out.println("Initial database state: " + database);
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Unsubscribe an existing user.
        String testEmail1 = "user1@example.com";
        System.out.println("Test 1: Unsubscribing '" + testEmail1 + "'");
        String result1 = unsubscribe(testEmail1);
        System.out.println("Result: " + result1);
        System.out.println("Database state: " + database + "\n");

        // Test Case 2: Try to unsubscribe the same user again.
        String testEmail2 = "user1@example.com";
        System.out.println("Test 2: Unsubscribing '" + testEmail2 + "' again");
        String result2 = unsubscribe(testEmail2);
        System.out.println("Result: " + result2);
        System.out.println("Database state: " + database + "\n");

        // Test Case 3: Unsubscribe a user that is not in the database.
        String testEmail3 = "nonexistent@example.com";
        System.out.println("Test 3: Unsubscribing '" + testEmail3 + "'");
        String result3 = unsubscribe(testEmail3);
        System.out.println("Result: " + result3);
        System.out.println("Database state: " + database + "\n");

        // Test Case 4: Unsubscribe with a null email.
        System.out.println("Test 4: Unsubscribing with a null email");
        String result4 = unsubscribe(null);
        System.out.println("Result: " + result4);
        System.out.println("Database state: " + database + "\n");

        // Test Case 5: Unsubscribe with an empty email.
        System.out.println("Test 5: Unsubscribing with an empty email");
        String result5 = unsubscribe("   ");
        System.out.println("Result: " + result5);
        System.out.println("Database state: " + database + "\n");
    }
}