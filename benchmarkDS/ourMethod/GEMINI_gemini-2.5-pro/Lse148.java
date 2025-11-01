import java.util.ArrayList;
import java.util.List;
import java.util.Collections;

// The main class name must be Lse148
public class Lse148 {

    // Simulating a database table with an in-memory list
    private static final List<Message> messageDatabase = new ArrayList<>();
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_MESSAGE_LENGTH = 280;

    // A simple class to represent a message
    static class Message {
        private final String username;
        private final String text;

        public Message(String username, String text) {
            this.username = username;
            this.text = text;
        }

        public String getUsername() {
            return username;
        }

        public String getText() {
            return text;
        }
    }

    /**
     * Sanitizes input to prevent basic injection attacks for display.
     * In a real web app, use a library like OWASP Java HTML Sanitizer.
     * @param input The string to sanitize.
     * @return The sanitized string.
     */
    public static String sanitize(String input) {
        if (input == null) {
            return "";
        }
        // Basic sanitization to prevent console manipulation or simple HTML injection if rendered in a web view.
        return input.replaceAll("<", "&lt;").replaceAll(">", "&gt;");
    }

    /**
     * Inserts a user message into our simulated database.
     * @param username The username of the poster.
     * @param messageText The message content.
     * @return true if insertion was successful, false otherwise.
     */
    public static boolean insert_user_message_in_db(String username, String messageText) {
        // Rule #1: Validate input
        if (username == null || username.trim().isEmpty() || username.length() > MAX_USERNAME_LENGTH) {
            System.err.println("Error: Invalid username.");
            return false;
        }
        if (messageText == null || messageText.trim().isEmpty() || messageText.length() > MAX_MESSAGE_LENGTH) {
            System.err.println("Error: Invalid message.");
            return false;
        }

        // Sanitize input before storing to ensure data integrity
        String sanitizedUsername = sanitize(username.trim());
        String sanitizedMessage = sanitize(messageText.trim());

        Message newMessage = new Message(sanitizedUsername, sanitizedMessage);
        messageDatabase.add(newMessage);
        System.out.println("Message from '" + sanitizedUsername + "' stored successfully.");
        return true;
    }

    /**
     * Called when a user submits a message.
     * @param username The username from the request.
     * @param message The message from the request.
     */
    public static void post(String username, String message) {
        System.out.println("\n--- Handling post request ---");
        // It calls the insert_user_message_in_db() function
        boolean success = insert_user_message_in_db(username, message);

        if (success) {
            // It redirects the user to the main page (simulated)
            System.out.println("Redirecting user to the main page...");
        } else {
            System.out.println("Post failed. Not redirecting.");
        }
    }

    /**
     * Retrieves all messages from the database for display.
     * @return An unmodifiable list of messages.
     */
    public static List<Message> getMessagesFromDb() {
        return Collections.unmodifiableList(messageDatabase);
    }

    /**
     * Displays all messages currently in the database.
     */
    public static void displayMessages() {
        System.out.println("\n--- Displaying All Messages ---");
        List<Message> messages = getMessagesFromDb();
        if (messages.isEmpty()) {
            System.out.println("No messages to display.");
        } else {
            for (Message msg : messages) {
                // Output is already sanitized from when it was inserted
                System.out.println(msg.getUsername() + ": " + msg.getText());
            }
        }
        System.out.println("-----------------------------\n");
    }

    public static void main(String[] args) {
        // --- 5 Test Cases ---

        // Test Case 1: Valid post
        post("Alice", "Hello, world!");
        displayMessages();

        // Test Case 2: Another valid post
        post("Bob", "This is a test message.");
        displayMessages();

        // Test Case 3: Invalid post (empty message)
        post("Charlie", " ");
        displayMessages();

        // Test Case 4: Invalid post (username too long)
        String longUsername = "user".repeat(20);
        post(longUsername, "My username is too long.");
        displayMessages();
        
        // Test Case 5: Post with characters that should be sanitized
        post("Eve", "<script>alert('XSS')</script>");
        displayMessages();
    }
}