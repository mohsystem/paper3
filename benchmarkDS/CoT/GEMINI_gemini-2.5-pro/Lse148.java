import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;

public class Lse148 {

    // Simulating a database table with a list of maps
    private static List<Map<String, String>> messageDatabase = new ArrayList<>();

    /**
     * Sanitizes user input to prevent basic XSS attacks.
     * Replaces special HTML characters with their corresponding entities.
     * @param input The string to sanitize.
     * @return The sanitized string.
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
     * Inserts a user's message into the simulated database.
     * @param username The sanitized username.
     * @param message The sanitized message.
     */
    public static void insert_user_message_in_db(String username, String message) {
        Map<String, String> entry = new HashMap<>();
        entry.put("username", username);
        entry.put("message", message);
        messageDatabase.add(entry);
        System.out.println("LOG: Message from '" + username + "' stored in DB.");
    }

    /**
     * Simulates handling a POST request to submit a message.
     * It gets username and message, sanitizes them, inserts them into the DB,
     * and simulates a redirect.
     * @param username The raw username from the request.
     * @param message The raw message from the request.
     */
    public static void post(String username, String message) {
        // 1. Input validation
        if (username == null || username.trim().isEmpty() || message == null || message.trim().isEmpty()) {
            System.out.println("ERROR: Username or message cannot be empty.");
            return;
        }

        // 2. Sanitize inputs to prevent XSS
        String sanitizedUsername = sanitize(username);
        String sanitizedMessage = sanitize(message);

        // 3. Insert into database
        insert_user_message_in_db(sanitizedUsername, sanitizedMessage);

        // 4. Redirect
        System.out.println("Redirecting user to the main page...\n");
    }

    /**
     * Displays all the messages currently stored in the database.
     */
    public static void displayMessages() {
        System.out.println("--- Displaying All Messages ---");
        if (messageDatabase.isEmpty()) {
            System.out.println("No messages to display.");
        } else {
            for (Map<String, String> entry : messageDatabase) {
                System.out.println(entry.get("username") + ": " + entry.get("message"));
            }
        }
        System.out.println("-----------------------------\n");
    }

    public static void main(String[] args) {
        System.out.println("Initial State:");
        displayMessages();

        // --- Test Cases ---
        System.out.println(">>> Running Test Cases <<<");

        // Test Case 1: Standard post
        post("Alice", "Hello World!");

        // Test Case 2: Another standard post
        post("Bob", "This is a test message.");

        // Test Case 3: Post with characters that need sanitization
        post("Charlie", "<script>alert('xss')</script>");

        // Test Case 4: Post with an empty message (should be rejected)
        post("David", "  ");

        // Test Case 5: Post from a user who posted before
        post("Alice", "I am posting again!");
        
        System.out.println(">>> Test Cases Finished <<<\n");

        System.out.println("Final State:");
        displayMessages();
    }
}