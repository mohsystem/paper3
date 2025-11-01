import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

public class Lse143 {

    // In a real application, this would be a persistent database.
    // Using a synchronized Set for thread-safety in a web server context.
    private static final Set<String> database = Collections.synchronizedSet(new HashSet<>());

    // A simple regex for email validation.
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
            "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,6}$");
    private static final int MAX_EMAIL_LENGTH = 254;

    /**
     * Unsubscribes a user by removing their email from the database.
     *
     * @param email The email address of the user to unsubscribe.
     * @return A string message indicating the result of the operation.
     */
    public static String unsubscribe(String email) {
        // Rule #1: Ensure all input is validated.
        if (email == null || email.trim().isEmpty()) {
            return "Error: Email parameter cannot be empty.";
        }
        if (email.length() > MAX_EMAIL_LENGTH) {
            return "Error: Email address is too long.";
        }
        if (!EMAIL_PATTERN.matcher(email).matches()) {
            return "Error: Invalid email format.";
        }

        // The synchronizedSet handles concurrent access.
        if (database.contains(email)) {
            database.remove(email);
            // In a real application, you would also perform other actions,
            // e.g., unsubscribing from a third-party mailing list service.
            return "User unsubscribed successfully.";
        } else {
            return "User is not subscribed.";
        }
    }

    public static void main(String[] args) {
        // Populate the database for testing
        database.add("test@example.com");
        database.add("user1@domain.com");
        database.add("another.user@service.net");

        System.out.println("Initial database state: " + database);
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: A valid email that exists in the database.
        String email1 = "test@example.com";
        System.out.println("\n1. Unsubscribing '" + email1 + "':");
        String result1 = unsubscribe(email1);
        System.out.println("   Result: " + result1);
        System.out.println("   Database state: " + database);

        // Test Case 2: The same email, which should now be gone.
        System.out.println("\n2. Unsubscribing '" + email1 + "' again:");
        String result2 = unsubscribe(email1);
        System.out.println("   Result: " + result2);

        // Test Case 3: A valid email that is not in the database.
        String email3 = "notfound@example.com";
        System.out.println("\n3. Unsubscribing '" + email3 + "':");
        String result3 = unsubscribe(email3);
        System.out.println("   Result: " + result3);

        // Test Case 4: An invalid email format.
        String email4 = "invalid-email";
        System.out.println("\n4. Unsubscribing '" + email4 + "':");
        String result4 = unsubscribe(email4);
        System.out.println("   Result: " + result4);

        // Test Case 5: A null email input.
        String email5 = null;
        System.out.println("\n5. Unsubscribing a null email:");
        String result5 = unsubscribe(email5);
        System.out.println("   Result: " + result5);
        
        System.out.println("\n--- Test Cases Complete ---");
        System.out.println("Final database state: " + database);
    }
}