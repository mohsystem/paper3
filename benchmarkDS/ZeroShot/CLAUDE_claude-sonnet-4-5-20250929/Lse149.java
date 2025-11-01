
import java.io.*;
import java.sql.*;
import java.util.*;
import javax.servlet.*;
import javax.servlet.http.*;

public class Lse149 {
    private static final String DB_URL = "jdbc:sqlite:messages.db";
    
    // Database initialization
    public static void initializeDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS messages (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "username TEXT NOT NULL," +
                        "message TEXT NOT NULL," +
                        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)";
            stmt.execute(sql);
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    // Insert user message in database
    public static boolean insertUserMessageInDb(String username, String message) {
        if (username == null || message == null || username.trim().isEmpty() || message.trim().isEmpty()) {
            return false;
        }
        
        // Sanitize inputs to prevent SQL injection
        String sanitizedUsername = sanitizeInput(username);
        String sanitizedMessage = sanitizeInput(message);
        
        String sql = "INSERT INTO messages (username, message) VALUES (?, ?)";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, sanitizedUsername);
            pstmt.setString(2, sanitizedMessage);
            pstmt.executeUpdate();
            return true;
        } catch (SQLException e) {
            e.printStackTrace();
            return false;
        }
    }
    
    // Get all messages from database
    public static List<Map<String, String>> getAllMessages() {
        List<Map<String, String>> messages = new ArrayList<>();
        String sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            
            while (rs.next()) {
                Map<String, String> message = new HashMap<>();
                message.put("username", rs.getString("username"));
                message.put("message", rs.getString("message"));
                message.put("timestamp", rs.getString("timestamp"));
                messages.add(message);
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        
        return messages;
    }
    
    // Sanitize input to prevent XSS and SQL injection
    public static String sanitizeInput(String input) {
        if (input == null) return "";
        return input.replaceAll("[<>\\"'&]", "")
                   .replaceAll("(?i)(script|javascript|onerror|onload)", "")
                   .trim();
    }
    
    // Simulated POST route handler
    public static String handlePostRoute(String username, String message) {
        if (username == null || message == null || username.trim().isEmpty() || message.trim().isEmpty()) {
            return "redirect:/error";
        }
        
        insertUserMessageInDb(username, message);
        return "redirect:/";
    }
    
    // Simulated root route handler
    public static String handleRootRoute() {
        List<Map<String, String>> messages = getAllMessages();
        StringBuilder html = new StringBuilder("<html><body><h1>Messages</h1>");
        
        for (Map<String, String> msg : messages) {
            html.append("<div>")
                .append("<strong>").append(msg.get("username")).append("</strong>: ")
                .append(msg.get("message"))
                .append(" <em>(").append(msg.get("timestamp")).append(")</em>")
                .append("</div><br>");
        }
        
        html.append("</body></html>");
        return html.toString();
    }
    
    public static void main(String[] args) {
        // Initialize database
        initializeDatabase();
        
        // Test case 1: Valid message
        System.out.println("Test 1: Valid message");
        System.out.println(handlePostRoute("Alice", "Hello World!"));
        System.out.println(handleRootRoute());
        System.out.println();
        
        // Test case 2: Another valid message
        System.out.println("Test 2: Another valid message");
        System.out.println(handlePostRoute("Bob", "Testing the system"));
        System.out.println();
        
        // Test case 3: Empty username
        System.out.println("Test 3: Empty username");
        System.out.println(handlePostRoute("", "This should fail"));
        System.out.println();
        
        // Test case 4: Null message
        System.out.println("Test 4: Null message");
        System.out.println(handlePostRoute("Charlie", null));
        System.out.println();
        
        // Test case 5: XSS attempt (should be sanitized)
        System.out.println("Test 5: XSS attempt");
        System.out.println(handlePostRoute("Dave", "<script>alert('XSS')</script>Safe message"));
        System.out.println(handleRootRoute());
    }
}
