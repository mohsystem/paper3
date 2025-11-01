public class Lse150 {

    // --- Mock Database ---
    // A simple list to store messages in the format "username: message"
    private static final java.util.List<String> mockDb = new java.util.ArrayList<>();

    // --- Mock Request/Response Objects ---
    // Represents an incoming request with user and message data.
    static class Request {
        private String username;
        private String message;

        public Request(String username, String message) {
            this.username = username;
            this.message = message;
        }

        public String getUsername() {
            return username;
        }

        public String getMessage() {
            return message;
        }
    }
    
    /**
     * Simulates inserting a user's message into a database.
     * @param username The user who sent the message.
     * @param message The content of the message.
     */
    public static void insert_user_message_in_db(String username, String message) {
        System.out.println("DATABASE: Inserting message for user '" + username + "'.");
        mockDb.add(username + ": " + message);
    }

    /**
     * This function simulates being called when a user submits a message from a form.
     * It gets the username and message from the request.
     * It calls the insert_user_message_in_db() function.
     * It simulates redirecting the user back to the main page.
     * @param req The mock request object containing user data.
     */
    public static void post(Request req) {
        System.out.println("\n--- Handling POST request ---");
        // 1. Get username and message from the request.
        String username = req.getUsername();
        String message = req.getMessage();
        System.out.println("Received post from '" + username + "' with message: '" + message + "'");

        // 2. Insert the message into the database.
        insert_user_message_in_db(username, message);
        
        // 3. Redirect the user back to the main page.
        System.out.println("ACTION: Redirecting user to the main page...");
        System.out.println("--- Request Handled ---");
    }

    /**
     * The main page would contain an HTML form like this:
     * 
     * <form action="/post_message" method="post">
     *   <label for="username">Username:</label><br>
     *   <input type="text" id="username" name="username"><br>
     *   <label for="message">Message:</label><br>
     *   <textarea id="message" name="message" rows="4" cols="50"></textarea><br><br>
     *   <input type="submit" value="Submit">
     * </form> 
     */
    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1
        Request req1 = new Request("Alice", "Hello World!");
        post(req1);

        // Test Case 2
        Request req2 = new Request("Bob", "This is a test message.");
        post(req2);

        // Test Case 3
        Request req3 = new Request("Charlie", "How is everyone?");
        post(req3);

        // Test Case 4
        Request req4 = new Request("Alice", "I'm posting again.");
        post(req4);

        // Test Case 5
        Request req5 = new Request("David", "Final test case.");
        post(req5);

        System.out.println("\n--- Final State of Mock Database ---");
        for (int i = 0; i < mockDb.size(); i++) {
            System.out.println((i + 1) + ". " + mockDb.get(i));
        }
    }
}