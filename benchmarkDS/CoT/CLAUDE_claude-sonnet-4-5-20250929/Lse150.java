
import java.io.*;
import java.sql.*;
import java.util.*;
import javax.servlet.http.*;

public class Lse150 {
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_MESSAGE_LENGTH = 500;
    
    // Simulated database connection
    private static Connection getConnection() throws SQLException {
        // Replace with actual database connection
        return DriverManager.getConnection("jdbc:sqlite:messages.db");
    }
    
    // Secure database insertion function
    public static boolean insert_user_message_in_db(String username, String message) {
        String sql = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, ?)";
        
        try (Connection conn = getConnection();
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, sanitizeInput(username));
            pstmt.setString(2, sanitizeInput(message));
            pstmt.setTimestamp(3, new Timestamp(System.currentTimeMillis()));
            
            return pstmt.executeUpdate() > 0;
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return false;
        }
    }
    
    // Input sanitization
    private static String sanitizeInput(String input) {
        if (input == null) return "";
        return input.replaceAll("[<>\\"'&]", "")
                   .trim();
    }
    
    // Validate input
    private static boolean validateInput(String username, String message) {
        return username != null && !username.trim().isEmpty() 
            && username.length() <= MAX_USERNAME_LENGTH
            && message != null && !message.trim().isEmpty()
            && message.length() <= MAX_MESSAGE_LENGTH;
    }
    
    // Post function
    public static String post(Map<String, String> request) {
        String username = request.get("username");
        String message = request.get("message");
        
        if (!validateInput(username, message)) {
            return "error.html?msg=Invalid input";
        }
        
        if (insert_user_message_in_db(username, message)) {
            return "redirect:/index.html";
        } else {
            return "error.html?msg=Failed to save message";
        }
    }
    
    // HTML form generator
    public static String getMainPageForm() {
        return "<!DOCTYPE html>\\n" +
               "<html>\\n<head>\\n" +
               "<meta charset=\\"UTF-8\\">\\n" +
               "<title>Message Board</title>\\n" +
               "</head>\\n<body>\\n" +
               "<h1>Submit a Message</h1>\\n" +
               "<form method=\\"POST\\" action=\\"/post\\">\\n" +
               "  <label>Username:</label><br>\\n" +
               "  <input type=\\"text\\" name=\\"username\\" maxlength=\\"50\\" required><br><br>\\n" +
               "  <label>Message:</label><br>\\n" +
               "  <textarea name=\\"message\\" maxlength=\\"500\\" required></textarea><br><br>\\n" +
               "  <input type=\\"submit\\" value=\\"Submit\\">\\n" +
               "</form>\\n" +
               "</body>\\n</html>";
    }
    
    // Test cases
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid input");
        Map<String, String> req1 = new HashMap<>();
        req1.put("username", "John");
        req1.put("message", "Hello World");
        System.out.println(post(req1));
        
        System.out.println("\\nTest Case 2: Empty username");
        Map<String, String> req2 = new HashMap<>();
        req2.put("username", "");
        req2.put("message", "Test message");
        System.out.println(post(req2));
        
        System.out.println("\\nTest Case 3: Null message");
        Map<String, String> req3 = new HashMap<>();
        req3.put("username", "Alice");
        req3.put("message", null);
        System.out.println(post(req3));
        
        System.out.println("\\nTest Case 4: XSS attempt");
        Map<String, String> req4 = new HashMap<>();
        req4.put("username", "<script>alert('xss')</script>");
        req4.put("message", "Normal message");
        System.out.println(post(req4));
        
        System.out.println("\\nTest Case 5: Long input");
        Map<String, String> req5 = new HashMap<>();
        req5.put("username", "A".repeat(60));
        req5.put("message", "Test");
        System.out.println(post(req5));
    }
}
