import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

public class Lse142 {

    // Simulates a database table of subscribed email addresses.
    // Using a HashSet for efficient lookups (O(1) average time complexity).
    private final Set<String> emailDatabase;

    public Lse142() {
        emailDatabase = new HashSet<>();
        // Pre-populate the database for demonstration
        emailDatabase.add("test@example.com");
        emailDatabase.add("user1@domain.com");
        emailDatabase.add("another.user@service.net");
    }

    /**
     * Validates an email address against a standard regex and length constraints.
     *
     * @param email The email address to validate.
     * @return true if the email is valid, false otherwise.
     */
    private boolean isValidEmail(String email) {
        if (email == null || email.trim().isEmpty()) {
            return false;
        }
        // Limit email length to a reasonable size to prevent abuse
        if (email.length() > 254) {
            return false;
        }
        // A common regex for email validation
        String emailRegex = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,6}$";
        Pattern pat = Pattern.compile(emailRegex);
        return pat.matcher(email).matches();
    }

    /**
     * Unsubscribes an email address by removing it from the database.
     *
     * @param email The email address to unsubscribe.
     * @return A status message indicating the result of the operation.
     */
    public String unsubscribe(String email) {
        // Rule #1: Ensure all input is validated.
        if (!isValidEmail(email)) {
            return "Invalid email address format.";
        }

        // The core logic of the function
        if (emailDatabase.contains(email)) {
            emailDatabase.remove(email);
            return "Successfully unsubscribed " + email + ".";
        } else {
            return "Email address not found in the database.";
        }
    }

    public static void main(String[] args) {
        Lse142 service = new Lse142();
        
        System.out.println("--- Test Cases ---");

        // Test Case 1: A valid email that exists in the database.
        String email1 = "test@example.com";
        System.out.println("Attempting to unsubscribe: " + email1);
        System.out.println("Result: " + service.unsubscribe(email1));
        System.out.println("Re-checking: " + service.unsubscribe(email1)); // Should now be not found
        System.out.println();

        // Test Case 2: A valid email that does not exist in the database.
        String email2 = "nonexistent@example.com";
        System.out.println("Attempting to unsubscribe: " + email2);
        System.out.println("Result: " + service.unsubscribe(email2));
        System.out.println();

        // Test Case 3: An invalid email address.
        String email3 = "invalid-email";
        System.out.println("Attempting to unsubscribe: " + email3);
        System.out.println("Result: " + service.unsubscribe(email3));
        System.out.println();

        // Test Case 4: A null input.
        String email4 = null;
        System.out.println("Attempting to unsubscribe: null");
        System.out.println("Result: " + service.unsubscribe(email4));
        System.out.println();

        // Test Case 5: An empty string input.
        String email5 = "";
        System.out.println("Attempting to unsubscribe: \"\"");
        System.out.println("Result: " + service.unsubscribe(email5));
        System.out.println();
    }
}