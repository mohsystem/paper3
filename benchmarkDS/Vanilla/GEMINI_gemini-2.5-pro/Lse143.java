import java.util.HashSet;
import java.util.Set;

public class Lse143 {
    // Simulating a database of subscribed emails
    private static Set<String> emailDatabase = new HashSet<>();

    /**
     * Unsubscribes a user by removing their email from the database.
     *
     * @param email The email address to unsubscribe.
     * @return A message indicating the result of the operation.
     */
    public static String unsubscribe(String email) {
        if (emailDatabase.contains(email)) {
            emailDatabase.remove(email);
            return "User with email '" + email + "' unsubscribed successfully.";
        } else {
            return "User with email '" + email + "' is not subscribed.";
        }
    }

    public static void main(String[] args) {
        // Pre-populate the database
        emailDatabase.add("test1@example.com");
        emailDatabase.add("user.two@domain.org");
        emailDatabase.add("another@email.net");

        System.out.println("Initial database: " + emailDatabase);
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Unsubscribe an existing user
        String email1 = "test1@example.com";
        System.out.println("Attempting to unsubscribe: " + email1);
        System.out.println("Result: " + unsubscribe(email1));
        System.out.println("Current database: " + emailDatabase + "\n");

        // Test Case 2: Unsubscribe a non-existent user
        String email2 = "notfound@example.com";
        System.out.println("Attempting to unsubscribe: " + email2);
        System.out.println("Result: " + unsubscribe(email2));
        System.out.println("Current database: " + emailDatabase + "\n");

        // Test Case 3: Unsubscribe another existing user
        String email3 = "another@email.net";
        System.out.println("Attempting to unsubscribe: " + email3);
        System.out.println("Result: " + unsubscribe(email3));
        System.out.println("Current database: " + emailDatabase + "\n");

        // Test Case 4: Try to unsubscribe the first user again
        System.out.println("Attempting to unsubscribe: " + email1);
        System.out.println("Result: " + unsubscribe(email1));
        System.out.println("Current database: " + emailDatabase + "\n");
        
        // Test Case 5: Unsubscribe the last user
        String email5 = "user.two@domain.org";
        System.out.println("Attempting to unsubscribe: " + email5);
        System.out.println("Result: " + unsubscribe(email5));
        System.out.println("Current database: " + emailDatabase + "\n");
    }
}