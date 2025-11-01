import java.util.ArrayList;
import java.util.List;
import java.util.Collections;

public class Lse149 {
    // In-memory list to simulate a database.
    // Using a synchronized list for basic thread safety in a web-like context.
    private static final List<Message> database = Collections.synchronizedList(new ArrayList<>());

    // Data class to hold message information
    static class Message {
        String username;
        String message;

        Message(String username, String message) {
            this.username = username;
            this.message = message;
        }
    }

    /**
     * Escapes HTML special characters in a string to prevent Cross-Site Scripting (XSS).
     *
     * @param input The string to escape.
     * @return The escaped string.
     */
    public static String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\"", "&quot;")
                    .replace("'", "&#x27;");
    }

    /**
     * Simulates inserting a user message into the database.
     * In a real application, this function must use a PreparedStatement to prevent SQL injection.
     *
     * @param username The username of the poster.
     * @param message  The message content.
     */
    public static void insert_user_message_in_db(String username, String message) {
        // SQL INJECTION PREVENTION NOTE:
        // If this were a real SQL database, the code would use parameterized queries:
        // String sql = "INSERT INTO messages (username, message) VALUES (?, ?)";
        // try (PreparedStatement pstmt = connection.prepareStatement(sql)) {
        //     pstmt.setString(1, username);
        //     pstmt.setString(2, message);
        //     pstmt.executeUpdate();
        // }
        
        // Basic validation: do not insert empty or null data.
        if (username != null && !username.trim().isEmpty() && message != null && !message.trim().isEmpty()) {
            database.add(new Message(username, message));
        }
    }
    
    /**
     * Simulates fetching all messages from the database and rendering them as an HTML string.
     * Calls escapeHtml on all user-provided data to prevent XSS.
     *
     * @return A string containing the HTML representation of messages.
     */
    public static String display_messages() {
        StringBuilder html = new StringBuilder("<h1>Messages</h1>\n<ul>\n");
        // Create a defensive copy to iterate over to avoid concurrency issues
        List<Message> messages_copy = new ArrayList<>(database);
        for (Message msg : messages_copy) {
            String safeUsername = escapeHtml(msg.username);
            String safeMessage = escapeHtml(msg.message);
            html.append("<li><b>").append(safeUsername).append(":</b> ")
                .append(safeMessage).append("</li>\n");
        }
        html.append("</ul>");
        return html.toString();
    }
    
    /**
     * Simulates a request handler for the "/post" route.
     *
     * @param username The username from the request.
     * @param message  The message from the request.
     */
    public static void routePost(String username, String message) {
        System.out.println("--- Handling POST /post ---");
        System.out.println("Received: username=" + username + ", message=" + message);
        insert_user_message_in_db(username, message);
        System.out.println("-> Inserted data into DB.");
        System.out.println("-> Redirecting to /");
        System.out.println("-------------------------");
    }

    /**
     * Simulates a request handler for the "/" route.
     */
    public static void routeGet() {
        System.out.println("--- Handling GET / ---");
        System.out.println("-> Displaying messages:");
        System.out.println(display_messages());
        System.out.println("----------------------");
    }

    public static void main(String[] args) {
        // --- 5 Test Cases ---

        System.out.println(">>> Test Case 1: Post a regular message");
        routePost("Alice", "Hello World!");
        routeGet();

        System.out.println("\n>>> Test Case 2: Post another message");
        routePost("Bob", "This is a test.");
        routeGet();

        System.out.println("\n>>> Test Case 3: Post a message with HTML characters (XSS attempt)");
        routePost("Eve", "<script>alert('xss')</script>");
        routeGet(); // Output should show escaped HTML: &lt;script&gt;...

        System.out.println("\n>>> Test Case 4: Post a message with other special characters");
        routePost("Charlie", "Me & You > Them");
        routeGet(); // Output should show escaped characters: Me &amp; You &gt; Them

        System.out.println("\n>>> Test Case 5: Post empty/null data (should be handled gracefully)");
        routePost("", "An empty username");
        routePost("David", null);
        routePost(null, "A null username");
        System.out.println("-> View after invalid posts (should be unchanged):");
        routeGet();
    }
}