import static spark.Spark.*;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

public class Lse149 {
    // In-memory "database" to store messages
    private static final List<Map<String, String>> messages = new ArrayList<>();

    // Simple HTML escaping to prevent XSS
    public static String escapeHTML(String s) {
        if (s == null) {
            return "";
        }
        return s.replace("&", "&amp;")
                .replace("<", "&lt;")
                .replace(">", "&gt;")
                .replace("\"", "&quot;")
                .replace("'", "&#39;");
    }

    public static String getMessagesAsHtml() {
        if (messages.isEmpty()) {
            return "<p>No messages yet.</p>";
        }
        return messages.stream()
            .map(msg -> "<p><strong>" + escapeHTML(msg.get("username")) + ":</strong> " 
                       + escapeHTML(msg.get("message")) + "</p>")
            .collect(Collectors.joining(""));
    }

    public static String getMainPageHtml() {
        return "<html>" +
               "<head><title>Message Board</title></head>" +
               "<body>" +
               "<h1>Post a Message</h1>" +
               "<form action='/post' method='post'>" +
               "    <label for='username'>Username:</label><br>" +
               "    <input type='text' id='username' name='username' required><br>" +
               "    <label for='message'>Message:</label><br>" +
               "    <textarea id='message' name='message' required></textarea><br><br>" +
               "    <input type='submit' value='Post'>" +
               "</form>" +
               "<h2>Messages</h2>" +
               "<div id='messages'>" + getMessagesAsHtml() + "</div>" +
               "</body>" +
               "</html>";
    }

    public static void insert_user_message_in_db(String username, String message) {
        Map<String, String> entry = new HashMap<>();
        if (username != null && !username.trim().isEmpty() && message != null && !message.trim().isEmpty()) {
            entry.put("username", username);
            entry.put("message", message);
            messages.add(entry);
        }
    }

    public static void main(String[] args) {
        port(8080);

        // GET route to display the form and messages
        get("/", (req, res) -> {
            res.type("text/html");
            return getMainPageHtml();
        });

        // POST route to handle form submission
        post("/post", (req, res) -> {
            String username = req.queryParams("username");
            String message = req.queryParams("message");

            insert_user_message_in_db(username, message);
            
            res.redirect("/");
            return "";
        });

        System.out.println("Server started at http://localhost:8080");
        System.out.println("Manual Test Cases to run in your browser:");
        System.out.println("1. Go to http://localhost:8080. You should see a form and 'No messages yet'.");
        System.out.println("2. Submit a message with username 'Alice' and message 'Hello World'. You should be redirected and see Alice's message.");
        System.out.println("3. Submit another message with username 'Bob' and message 'Hi Alice!'. You should see both messages displayed.");
        System.out.println("4. Submit a message with username 'Eve' and message '<b>bold</b>'. The output should show the literal tags '<b>bold</b>' and not bolded text, preventing XSS.");
        System.out.println("5. Submit a message with username 'Mallory' and message '<script>alert(\"xss\")</script>'. No alert box should appear, and the script tag should be displayed as text.");
    }
}
// To run this code:
// 1. You need to have a Java JDK installed.
// 2. You need the SparkJava dependency. A simple way is to use a build tool like Maven or Gradle.
//    For Maven, add this to your pom.xml:
//    <dependencies>
//        <dependency>
//            <groupId>com.sparkjava</groupId>
//            <artifactId>spark-core</artifactId>
//            <version>2.9.4</version>
//        </dependency>
//    </dependencies>
// 3. Compile and run the Lse149 class.