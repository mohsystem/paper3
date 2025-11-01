import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public class Lse150 {

    // In a real web application, this would be a database connection.
    // Here, we use a static list to simulate a database for demonstration.
    private static final List<Message> messageDatabase = new ArrayList<>();

    // A simple record to hold message data.
    private record Message(String username, String message) {}

    /**
     * Sanitizes user input to prevent Cross-Site Scripting (XSS) by escaping HTML characters.
     * In a real application, a library like OWASP Java Encoder would be preferred.
     * @param input The raw string from the user.
     * @return A sanitized string safe for HTML rendering.
     */
    public static String sanitize(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\"", "&quot;")
                    .replace("'", "&#x27;");
    }

    /**
     * Simulates inserting a user's message into the database.
     * This method is safe from SQL Injection because it's not constructing a raw SQL query.
     * In a real application, this would use PreparedStatement.
     * @param username The user's name.
     * @param message The user's message.
     */
    public static void insert_user_message_in_db(String username, String message) {
        System.out.println("[DB] Inserting message for user: " + username);
        messageDatabase.add(new Message(username, message));
    }

    /**
     * Simulates handling a POST request from a user submitting a message.
     * @param username The username from the request.
     * @param message The message from the request.
     */
    public static void post(String username, String message) {
        System.out.println("\n--- Handling POST Request ---");

        // Security: Validate input to prevent empty submissions.
        if (username == null || username.trim().isEmpty() || message == null || message.trim().isEmpty()) {
            System.out.println("Error: Username and message cannot be empty.");
            System.out.println("--- End POST Request ---");
            return;
        }

        // The data is stored raw and sanitized only upon display.
        insert_user_message_in_db(username.trim(), message.trim());

        // Simulate redirecting the user back to the main page.
        System.out.println("Action: Message posted successfully.");
        System.out.println("Action: Redirecting user to main page...");
        System.out.println("--- End POST Request ---");
    }

    /**
     * Simulates rendering the main page with a message form and existing messages.
     */
    public static void showMainPage() {
        System.out.println("\n======================================");
        System.out.println("         Welcome to the Main Page");
        System.out.println("======================================");

        System.out.println("\n--- Message Board ---");
        if (messageDatabase.isEmpty()) {
            System.out.println("(No messages yet)");
        } else {
            for (Message msg : messageDatabase) {
                // Security: Sanitize output to prevent XSS.
                String safeUsername = sanitize(msg.username());
                String safeMessage = sanitize(msg.message());
                System.out.println(safeUsername + ": " + safeMessage);
            }
        }
        System.out.println("--- End of Messages ---");

        // Simulate the submission form.
        System.out.println("\n--- Submit a New Message (Form) ---");
        System.out.println("[ Username: ___________ ]");
        System.out.println("[ Message:  ___________ ]");
        System.out.println("[ [SUBMIT] ]");
        System.out.println("======================================\n");
    }

    public static void main(String[] args) {
        // Initially, show the empty main page.
        showMainPage();

        // --- Test Cases ---

        // Test Case 1: A standard, valid post.
        System.out.println(">>> TEST CASE 1: Standard Post");
        post("Alice", "Hello everyone!");
        showMainPage();

        // Test Case 2: Another standard post.
        System.out.println(">>> TEST CASE 2: Another User Post");
        post("Bob", "Hi Alice, how are you?");
        showMainPage();

        // Test Case 3: A post with special HTML characters (XSS attempt).
        System.out.println(">>> TEST CASE 3: XSS Attempt");
        post("Mallory", "<script>alert('pwned')</script>");
        showMainPage();

        // Test Case 4: A post with empty message (should be rejected).
        System.out.println(">>> TEST CASE 4: Empty Message");
        post("Charlie", "      ");
        showMainPage();

        // Test Case 5: A post with a mix of content.
        System.out.println(">>> TEST CASE 5: Mixed Content");
        post("David", "I think 5 > 3 & 2 < 4. What do you think?");
        showMainPage();
    }
}