import java.util.ArrayList;
import java.util.List;

public class Lse148 {

    // In-memory list to act as a mock database
    private final List<Message> messageDatabase;

    public Lse148() {
        this.messageDatabase = new ArrayList<>();
    }

    // A simple data class to hold message information
    private static class Message {
        String username;
        String content;

        Message(String username, String content) {
            this.username = username;
            this.content = content;
        }
    }

    /**
     * Sanitizes user input to prevent Cross-Site Scripting (XSS) attacks.
     * This is done by escaping HTML special characters.
     *
     * @param input The raw string from the user.
     * @return A sanitized string safe for HTML rendering.
     */
    private String htmlEscape(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder escaped = new StringBuilder();
        for (char c : input.toCharArray()) {
            switch (c) {
                case '&':
                    escaped.append("&amp;");
                    break;
                case '<':
                    escaped.append("&lt;");
                    break;
                case '>':
                    escaped.append("&gt;");
                    break;
                case '"':
                    escaped.append("&quot;");
                    break;
                case '\'':
                    escaped.append("&#39;");
                    break;
                default:
                    escaped.append(c);
                    break;
            }
        }
        return escaped.toString();
    }


    /**
     * Inserts a user's message into our mock database.
     * Input is stored raw. Sanitization happens on display.
     *
     * @param username The name of the user posting the message.
     * @param message  The content of the message.
     */
    public void insert_user_message_in_db(String username, String message) {
        // Basic input validation: prevent nulls or overly long messages
        if (username == null || message == null || username.isEmpty() || message.isEmpty()) {
            System.out.println("Error: Username and message cannot be empty.");
            return;
        }
        if (username.length() > 50 || message.length() > 280) {
             System.out.println("Error: Username or message is too long.");
             return;
        }
        messageDatabase.add(new Message(username, message));
        System.out.println("Message from '" + username + "' stored successfully.");
    }

    /**
     * Simulates a user submitting a post.
     *
     * @param username The name of the user from the request.
     * @param message  The message content from the request.
     */
    public void post(String username, String message) {
        System.out.println("\n--- Received new post from user: " + username + " ---");
        insert_user_message_in_db(username, message);
        System.out.println("...Redirecting user to the main page.");
    }

    /**
     * Displays all messages from the mock database in a secure manner.
     */
    public void display_all_messages() {
        System.out.println("\n--- Main Page: All Messages ---");
        if (messageDatabase.isEmpty()) {
            System.out.println("No messages to display.");
        } else {
            for (Message msg : messageDatabase) {
                // Sanitize output just before displaying it to prevent stored XSS.
                String safeUsername = htmlEscape(msg.username);
                String safeContent = htmlEscape(msg.content);
                System.out.println("User: " + safeUsername);
                System.out.println("Message: " + safeContent);
                System.out.println("---------------------------------");
            }
        }
    }


    public static void main(String[] args) {
        Lse148 app = new Lse148();

        // --- Test Cases ---

        // Test Case 1: Normal post
        app.post("Alice", "Hello, world! This is my first message.");

        // Test Case 2: Another normal post
        app.post("Bob", "This is a great platform!");

        // Test Case 3: Post with special HTML characters
        app.post("Charlie", "My thoughts on C++: it's great for when you need <performance> & control.");

        // Test Case 4: A malicious post attempting an XSS attack
        app.post("Eve", "<script>alert('You have been hacked!');</script>");

        // Test Case 5: Post with quotes
        app.post("David", "He said, \"It's a 'nice' day.\"");
        
        // Display all the messages at the end
        app.display_all_messages();
    }
}