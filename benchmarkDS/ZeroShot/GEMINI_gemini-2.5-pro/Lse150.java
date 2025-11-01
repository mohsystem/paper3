public class Lse150 {
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_MESSAGE_LENGTH = 280;

    // Simulated in-memory database using a thread-safe map for potential concurrency
    private static final java.util.Map<String, java.util.List<String>> userMessagesDb = new java.util.concurrent.ConcurrentHashMap<>();

    /**
     * Inserts a user's message into the simulated database.
     * In a real application, this is where secure database interaction would happen.
     * @param username The validated user's name.
     * @param message The validated message content.
     */
    public static void insert_user_message_in_db(String username, String message) {
        System.out.println("DB: Inserting message for user '" + username + "'.");
        // SECURITY: In a real application, use parameterized queries (PreparedStatements)
        // to prevent SQL injection. NEVER build a query by concatenating user input.
        // Example (JDBC):
        // PreparedStatement stmt = conn.prepareStatement("INSERT INTO messages(user, msg) VALUES (?, ?)");
        // stmt.setString(1, username);
        // stmt.setString(2, message);
        // stmt.executeUpdate();
        
        userMessagesDb.computeIfAbsent(username, k -> new java.util.ArrayList<>()).add(message);
    }

    /**
     * Simulates handling a POST request to submit a message.
     * It gets the username and message from the request, performs validation,
     * calls the database insertion function, and simulates a redirect.
     * @param username The username from the request.
     * @param message The message from the request.
     * @return A string indicating the result, simulating a redirect URL.
     */
    public static String post(String username, String message) {
        // SECURITY: Validate all user input before processing.
        // 1. Check for null or effectively empty strings.
        if (username == null || username.trim().isEmpty()) {
            System.err.println("Error: Username cannot be empty.");
            return "redirect:/main?error=invalid_username";
        }
        if (message == null || message.trim().isEmpty()) {
            System.err.println("Error: Message cannot be empty.");
            return "redirect:/main?error=invalid_message";
        }

        // 2. Enforce length limits to prevent resource exhaustion or oversized data issues.
        if (username.length() > MAX_USERNAME_LENGTH) {
            System.err.println("Error: Username is too long.");
            return "redirect:/main?error=username_too_long";
        }
        if (message.length() > MAX_MESSAGE_LENGTH) {
            System.err.println("Error: Message is too long.");
            return "redirect:/main?error=message_too_long";
        }
        
        // SECURITY: To prevent Cross-Site Scripting (XSS), user-provided content
        // should be properly HTML-escaped before being rendered on a web page.
        // This function only handles storing data, not rendering it.

        insert_user_message_in_db(username, message);
        
        System.out.println("Success! Redirecting to the main page.");
        return "redirect:/main";
    }

    /**
     * The main method simulates a user interface (like a form on a main page)
     * and runs several test cases.
     */
    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Valid submission
        System.out.println("\n--- Test Case 1: Valid submission ---");
        post("alice", "Hello, world!");

        // Test Case 2: Another valid submission from a different user
        System.out.println("\n--- Test Case 2: Another valid user ---");
        post("bob", "This is a test message.");

        // Test Case 3: Message is too long
        System.out.println("\n--- Test Case 3: Message too long ---");
        String longMessage = new String(new char[300]).replace('\0', 'a');
        post("alice", longMessage);

        // Test Case 4: Empty username (whitespace only)
        System.out.println("\n--- Test Case 4: Empty username ---");
        post(" ", "This should fail.");

        // Test Case 5: Potentially malicious input (would be escaped on display)
        System.out.println("\n--- Test Case 5: Input with special characters ---");
        post("charlie", "<script>alert('xss')</script>");

        System.out.println("\n--- Final Database State ---");
        userMessagesDb.forEach((user, messages) -> {
            System.out.println("User: " + user);
            messages.forEach(msg -> System.out.println("  - " + msg));
        });
    }
}