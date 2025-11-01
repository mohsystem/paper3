import static spark.Spark.*;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Lse149 {

    // A simple class to hold message data
    private static class Message {
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

    // In-memory "database" using a thread-safe list
    private static final List<Message> messages = Collections.synchronizedList(new ArrayList<>());
    
    // Max lengths for input validation
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_MESSAGE_LENGTH = 280;

    /**
     * A utility function to escape HTML characters to prevent XSS.
     * @param input The string to escape.
     * @return The escaped string.
     */
    private static String escapeHtml(String input) {
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
     * Inserts a user message into the in-memory database.
     * This is thread-safe because we use a synchronized list.
     * @param username The username of the poster.
     * @param message The message content.
     */
    public static void insert_user_message_in_db(String username, String message) {
        messages.add(new Message(username, message));
    }

    public static void main(String[] args) {
        // Set the port the server will run on
        port(8080);

        // Add 5 test cases before starting the server
        insert_user_message_in_db("Alice", "Hello from Java Spark!");
        insert_user_message_in_db("Bob", "This is a test message.");
        insert_user_message_in_db("Charlie", "Web development in Java.");
        insert_user_message_in_db("David", "Sanitizing output is important.");
        insert_user_message_in_db("Eve", "<img src=x onerror=alert('XSS')>");
        
        // Route to display the form and messages
        get("/", (req, res) -> {
            StringBuilder htmlBuilder = new StringBuilder();
            htmlBuilder.append("<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>Message Board</title>");
            htmlBuilder.append("<style>body { font-family: sans-serif; margin: 2em; } .message { border: 1px solid #ccc; padding: 10px; margin-bottom: 10px; border-radius: 5px; } .message-user { font-weight: bold; } .message-text { margin-top: 5px; }</style>");
            htmlBuilder.append("</head><body>");
            htmlBuilder.append("<h1>Post a Message</h1>");
            htmlBuilder.append("<form action=\"/post\" method=\"post\">");
            htmlBuilder.append("<label for=\"username\">Username:</label><br>");
            htmlBuilder.append("<input type=\"text\" id=\"username\" name=\"username\" required maxlength=\"").append(MAX_USERNAME_LENGTH).append("\"><br><br>");
            htmlBuilder.append("<label for=\"message\">Message:</label><br>");
            htmlBuilder.append("<textarea id=\"message\" name=\"message\" rows=\"4\" cols=\"50\" required maxlength=\"").append(MAX_MESSAGE_LENGTH).append("\"></textarea><br><br>");
            htmlBuilder.append("<input type=\"submit\" value=\"Submit\">");
            htmlBuilder.append("</form>");
            htmlBuilder.append("<h2>Messages</h2>");

            // To prevent ConcurrentModificationException, we synchronize on the list
            // while creating a copy for iteration.
            List<Message> messagesCopy;
            synchronized (messages) {
                messagesCopy = new ArrayList<>(messages);
            }

            // Iterate backwards to show newest first
            for (int i = messagesCopy.size() - 1; i >= 0; i--) {
                Message msg = messagesCopy.get(i);
                htmlBuilder.append("<div class='message'>");
                htmlBuilder.append("<div class='message-user'>").append(escapeHtml(msg.getUsername())).append("</div>");
                htmlBuilder.append("<div class='message-text'>").append(escapeHtml(msg.getText())).append("</div>");
                htmlBuilder.append("</div>");
            }
            
            htmlBuilder.append("</body></html>");
            return htmlBuilder.toString();
        });

        // Route to handle the form submission
        post("/post", (req, res) -> {
            String username = req.queryParams("username");
            String message = req.queryParams("message");

            // Input validation
            if (username == null || username.trim().isEmpty() || message == null || message.trim().isEmpty()) {
                res.status(400);
                return "Username and message cannot be empty.";
            }
            
            username = username.trim();
            message = message.trim();
            
            if (username.length() > MAX_USERNAME_LENGTH) {
                res.status(400);
                return "Username exceeds maximum length of " + MAX_USERNAME_LENGTH + " characters.";
            }
            
            if (message.length() > MAX_MESSAGE_LENGTH) {
                res.status(400);
                return "Message exceeds maximum length of " + MAX_MESSAGE_LENGTH + " characters.";
            }

            insert_user_message_in_db(username, message);

            res.redirect("/");
            return ""; // The return value is ignored on redirect
        });

        System.out.println("Server started on http://localhost:8080");
        System.out.println("To run this, you need the SparkJava dependency.");
        System.out.println("Maven: <dependency><groupId>com.sparkjava</groupId><artifactId>spark-core</artifactId><version>2.9.4</version></dependency>");
    }
}