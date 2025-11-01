import java.util.ArrayList;
import java.util.List;

public class Lse148 {

    // Simulating a database table with a static list
    private static List<Message> messagesDb = new ArrayList<>();

    // A simple class to hold message data
    private static class Message {
        String username;
        String message;

        Message(String username, String message) {
            this.username = username;
            this.message = message;
        }

        @Override
        public String toString() {
            return username + ": " + message;
        }
    }

    /**
     * Inserts a user's message into our simulated database.
     * @param username The name of the user.
     * @param message The content of the message.
     */
    public static void insert_user_message_in_db(String username, String message) {
        messagesDb.add(new Message(username, message));
        System.out.println("DB: Message from '" + username + "' stored successfully.");
    }

    /**
     * Simulates a user submitting a message.
     * It gets the username and message, inserts it into the DB, and redirects.
     * @param username The username from the request.
     * @param message The message from the request.
     */
    public static void post(String username, String message) {
        System.out.println("\nPOST request received for user: " + username);
        insert_user_message_in_db(username, message);
        // Simulate redirecting the user
        System.out.println("Action: Redirecting user to the main page...");
    }

    /**
     * Displays all the messages currently stored in the database.
     */
    public static void displayMessages() {
        System.out.println("\n--- Main Message Board ---");
        if (messagesDb.isEmpty()) {
            System.out.println("No messages to display.");
        } else {
            for (Message msg : messagesDb) {
                System.out.println(msg);
            }
        }
        System.out.println("--------------------------");
    }

    public static void main(String[] args) {
        // --- 5 Test Cases ---

        // Test Case 1
        post("Alice", "Hello, world!");

        // Test Case 2
        post("Bob", "This is my first post.");

        // Test Case 3
        post("Charlie", "Does this thing work?");

        // Test Case 4
        post("Alice", "I'm posting again!");

        // Test Case 5
        post("Dave", "Hi everyone!");

        // Now, display all the messages that have been posted.
        displayMessages();
    }
}