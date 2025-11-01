import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

public class Lse142 {

    // Simulating a database table of subscribers using a HashSet for efficient lookups
    private static Set<String> subscriberDatabase = new HashSet<>();

    // A common regex for email validation to prevent malformed inputs.
    private static final String EMAIL_REGEX = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,6}$";
    private static final Pattern EMAIL_PATTERN = Pattern.compile(EMAIL_REGEX);

    /**
     * Unsubscribes an email address after validating its format.
     * In a real-world application, this function would interact with a database.
     *
     * @param email The email address to unsubscribe.
     * @param database The subscriber database (for simulation purposes).
     * @return A status message indicating success, failure, or info.
     */
    public static String unsubscribe(String email, Set<String> database) {
        // 1. Input Validation: Check for null or empty string to prevent errors.
        if (email == null || email.trim().isEmpty()) {
            return "Error: Email address cannot be empty.";
        }

        // 2. Input Validation: Check for valid email format to reject malformed data.
        if (!EMAIL_PATTERN.matcher(email).matches()) {
            return "Error: Invalid email address format.";
        }

        // --- Simulated Database Interaction ---
        // SECURITY NOTE: In a real application, use PreparedStatement (parameterized queries)
        // to interact with the database. This is the single most important defense
        // against SQL injection attacks.
        // Example:
        // PreparedStatement stmt = connection.prepareStatement("DELETE FROM subscribers WHERE email = ?");
        // stmt.setString(1, email);
        // int rowsAffected = stmt.executeUpdate();

        // 3. Business Logic: Check if email exists and remove it.
        if (database.contains(email)) {
            database.remove(email);
            return "Success: " + email + " has been unsubscribed.";
        } else {
            return "Info: " + email + " was not found in our database.";
        }
    }

    public static void main(String[] args) {
        // Initialize the simulated database with some sample emails
        subscriberDatabase.add("test@example.com");
        subscriberDatabase.add("user1@domain.com");
        subscriberDatabase.add("another.user@service.net");

        System.out.println("Initial database: " + subscriberDatabase);
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Unsubscribe an existing email
        System.out.println("1. " + unsubscribe("test@example.com", subscriberDatabase));
        System.out.println("   Database after: " + subscriberDatabase);

        // Test Case 2: Attempt to unsubscribe an email that is not in the database
        System.out.println("2. " + unsubscribe("nonexistent@example.com", subscriberDatabase));
        System.out.println("   Database after: " + subscriberDatabase);

        // Test Case 3: Attempt to unsubscribe with an invalid email format
        System.out.println("3. " + unsubscribe("invalid-email", subscriberDatabase));
        System.out.println("   Database after: " + subscriberDatabase);
        
        // Test Case 4: Attempt to unsubscribe another existing email
        System.out.println("4. " + unsubscribe("user1@domain.com", subscriberDatabase));
        System.out.println("   Database after: " + subscriberDatabase);

        // Test Case 5: Attempt to unsubscribe with a null input
        System.out.println("5. " + unsubscribe(null, subscriberDatabase));
        System.out.println("   Database after: " + subscriberDatabase);
    }
}