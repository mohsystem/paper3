import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

public class Lse143 {

    // A secure regex for email validation to prevent malformed input.
    // This pattern is a common and reasonably comprehensive one.
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[a-zA-Z0-9_+&*-]+(?:\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,7}$"
    );

    /**
     * Checks if the given email has a valid format.
     * @param email The email to validate.
     * @return true if the email is valid, false otherwise.
     */
    private static boolean isValidEmail(String email) {
        if (email == null || email.trim().isEmpty()) {
            return false;
        }
        return EMAIL_PATTERN.matcher(email).matches();
    }

    /**
     * Unsubscribes a user by removing their email from a simulated database.
     * In a real application, this would interact with a persistent database
     * and should use parameterized queries (PreparedStatements in Java) to prevent SQL injection.
     * @param database A Set representing the list of subscribed users. A Set is used for
     *                 efficient lookups (contains) and removals.
     * @param email The email of the user to unsubscribe.
     * @return A status message indicating success, failure, or error.
     */
    public static String unsubscribe(Set<String> database, String email) {
        if (!isValidEmail(email)) {
            return "Error: Invalid email format provided.";
        }

        // The contains() and remove() operations are efficient (average O(1)) on a HashSet.
        if (database.contains(email)) {
            database.remove(email);
            // In a real scenario, you would also trigger other actions,
            // e.g., removing them from a mailing list service via an API call.
            return "User with email '" + email + "' has been successfully unsubscribed.";
        } else {
            return "User with email '" + email + "' is not subscribed.";
        }
    }

    public static void main(String[] args) {
        // 1. Initialize a simulated database of subscribed emails
        Set<String> subscribedUsers = new HashSet<>();
        subscribedUsers.add("test.user@example.com");
        subscribedUsers.add("jane.doe@example.org");
        subscribedUsers.add("john.smith@email.net");

        System.out.println("Initial database: " + subscribedUsers);
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Unsubscribe an existing user
        System.out.println("\nTest 1: Unsubscribing an existing user...");
        String result1 = unsubscribe(subscribedUsers, "jane.doe@example.org");
        System.out.println("Result: " + result1);
        System.out.println("Database state: " + subscribedUsers);

        // Test Case 2: Try to unsubscribe the same user again
        System.out.println("\nTest 2: Unsubscribing the same user again...");
        String result2 = unsubscribe(subscribedUsers, "jane.doe@example.org");
        System.out.println("Result: " + result2);
        System.out.println("Database state: " + subscribedUsers);

        // Test Case 3: Try to unsubscribe a user not in the database
        System.out.println("\nTest 3: Unsubscribing a non-existent user...");
        String result3 = unsubscribe(subscribedUsers, "not.found@example.com");
        System.out.println("Result: " + result3);
        System.out.println("Database state: " + subscribedUsers);

        // Test Case 4: Try to unsubscribe with an invalid email format
        System.out.println("\nTest 4: Unsubscribing with an invalid email...");
        String result4 = unsubscribe(subscribedUsers, "invalid-email");
        System.out.println("Result: " + result4);
        System.out.println("Database state: " + subscribedUsers);
        
        // Test Case 5: Try to unsubscribe with a null email
        System.out.println("\nTest 5: Unsubscribing with a null email...");
        String result5 = unsubscribe(subscribedUsers, null);
        System.out.println("Result: " + result5);
        System.out.println("Database state: " + subscribedUsers);
    }
}