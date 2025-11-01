
import java.io.*;
import java.util.*;
import javax.servlet.*;
import javax.servlet.http.*;

public class Lse148 extends HttpServlet {
    
    // In-memory storage for messages (simulating database)
    private static List<Message> messages = new ArrayList<>();
    
    static class Message {
        String username;
        String message;
        Date timestamp;
        
        public Message(String username, String message) {
            this.username = username;
            this.message = message;
            this.timestamp = new Date();
        }
    }
    
    // Function to insert user message in database
    public static void insert_user_message_in_db(String username, String message) {
        messages.add(new Message(username, message));
        System.out.println("Message inserted: " + username + " - " + message);
    }
    
    // Post method to handle form submission
    public void post(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        String username = request.getParameter("username");
        String message = request.getParameter("message");
        
        if (username != null && message != null && !username.isEmpty() && !message.isEmpty()) {
            insert_user_message_in_db(username, message);
        }
        
        response.sendRedirect("/");
    }
    
    // Get method to display messages
    public void get(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        response.setContentType("text/html");
        PrintWriter out = response.getWriter();
        
        out.println("<html><body>");
        out.println("<h2>Messages</h2>");
        
        for (Message msg : messages) {
            out.println("<p><strong>" + msg.username + ":</strong> " + msg.message + "</p>");
        }
        
        out.println("<hr>");
        out.println("<form method='post' action='/post'>");
        out.println("Username: <input type='text' name='username'><br>");
        out.println("Message: <input type='text' name='message'><br>");
        out.println("<input type='submit' value='Send'>");
        out.println("</form>");
        out.println("</body></html>");
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1: Adding first message");
        insert_user_message_in_db("Alice", "Hello World!");
        
        System.out.println("\\nTest Case 2: Adding second message");
        insert_user_message_in_db("Bob", "Hi there!");
        
        System.out.println("\\nTest Case 3: Adding message with special characters");
        insert_user_message_in_db("Charlie", "Test @#$ message!");
        
        System.out.println("\\nTest Case 4: Adding empty username");
        insert_user_message_in_db("", "Anonymous message");
        
        System.out.println("\\nTest Case 5: Adding long message");
        insert_user_message_in_db("David", "This is a very long message to test the system's ability to handle lengthy text content.");
        
        System.out.println("\\nAll messages:");
        for (Message msg : messages) {
            System.out.println(msg.username + ": " + msg.message);
        }
    }
}
