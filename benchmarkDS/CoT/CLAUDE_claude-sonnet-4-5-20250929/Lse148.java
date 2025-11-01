
import java.sql.*;
import java.util.*;
import java.io.*;

public class Lse148 {
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_MESSAGE_LENGTH = 500;
    
    // Simulated request object
    static class Request {
        private Map<String, String> parameters = new HashMap<>();
        
        public String getParameter(String name) {
            return parameters.get(name);
        }
        
        public void setParameter(String name, String value) {
            parameters.put(name, value);
        }
    }
    
    // Simulated response object
    static class Response {
        public void redirect(String url) {
            System.out.println("Redirecting to: " + url);
        }
    }
    
    // Secure input validation
    private static String validateAndSanitizeInput(String input, int maxLength) {
        if (input == null || input.trim().isEmpty()) {
            return null;
        }
        
        String sanitized = input.trim();
        if (sanitized.length() > maxLength) {
            sanitized = sanitized.substring(0, maxLength);
        }
        
        // Remove potentially dangerous characters
        sanitized = sanitized.replaceAll("[<>\\"'&]", "");
        
        return sanitized;
    }
    
    // Secure database insertion using prepared statements
    private static boolean insert_user_message_in_db(String username, String message) {
        String url = "jdbc:sqlite:messages.db";
        String sql = "INSERT INTO messages(username, message, timestamp) VALUES(?, ?, ?)";
        
        try (Connection conn = DriverManager.getConnection(url);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, username);
            pstmt.setString(2, message);
            pstmt.setTimestamp(3, new Timestamp(System.currentTimeMillis()));
            pstmt.executeUpdate();
            return true;
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return false;
        }
    }
    
    // Main post function with security measures
    public static boolean post(Request request, Response response) {
        try {
            String username = request.getParameter("username");
            String message = request.getParameter("message");
            
            // Validate inputs
            username = validateAndSanitizeInput(username, MAX_USERNAME_LENGTH);
            message = validateAndSanitizeInput(message, MAX_MESSAGE_LENGTH);
            
            if (username == null || message == null) {
                System.err.println("Invalid input: username or message is empty");
                return false;
            }
            
            // Insert into database securely
            if (insert_user_message_in_db(username, message)) {
                response.redirect("/main");
                return true;
            }
            
            return false;
            
        } catch (Exception e) {
            System.err.println("Error in post: " + e.getMessage());
            return false;
        }
    }
    
    // Secure message display with HTML encoding
    private static String htmlEncode(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    public static List<String> displayMessages() {
        List<String> messages = new ArrayList<>();
        String sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC LIMIT 100";
        String url = "jdbc:sqlite:messages.db";
        
        try (Connection conn = DriverManager.getConnection(url);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            
            while (rs.next()) {
                String username = htmlEncode(rs.getString("username"));
                String message = htmlEncode(rs.getString("message"));
                String timestamp = rs.getTimestamp("timestamp").toString();
                
                messages.add(String.format("[%s] %s: %s", timestamp, username, message));
            }
            
        } catch (SQLException e) {
            System.err.println("Error retrieving messages: " + e.getMessage());
        }
        
        return messages;
    }
    
    // Initialize database
    private static void initDatabase() {
        String url = "jdbc:sqlite:messages.db";
        String sql = "CREATE TABLE IF NOT EXISTS messages (" +
                    "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                    "username TEXT NOT NULL," +
                    "message TEXT NOT NULL," +
                    "timestamp DATETIME NOT NULL)";
        
        try (Connection conn = DriverManager.getConnection(url);
             Statement stmt = conn.createStatement()) {
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database init error: " + e.getMessage());
        }
    }
    
    public static void main(String[] args) {
        initDatabase();
        
        // Test cases
        System.out.println("=== Test Cases ===\\n");
        
        // Test 1: Valid message
        Request req1 = new Request();
        req1.setParameter("username", "Alice");
        req1.setParameter("message", "Hello World!");
        Response res1 = new Response();
        System.out.println("Test 1 - Valid message: " + post(req1, res1));
        
        // Test 2: Another valid message
        Request req2 = new Request();
        req2.setParameter("username", "Bob");
        req2.setParameter("message", "This is a test message");
        Response res2 = new Response();
        System.out.println("Test 2 - Valid message: " + post(req2, res2));
        
        // Test 3: Empty username (should fail)
        Request req3 = new Request();
        req3.setParameter("username", "");
        req3.setParameter("message", "Message without user");
        Response res3 = new Response();
        System.out.println("Test 3 - Empty username: " + post(req3, res3));
        
        // Test 4: XSS attempt (should be sanitized)
        Request req4 = new Request();
        req4.setParameter("username", "Hacker<script>");
        req4.setParameter("message", "Alert<script>alert('XSS')</script>");
        Response res4 = new Response();
        System.out.println("Test 4 - XSS attempt: " + post(req4, res4));
        
        // Test 5: Very long message (should be truncated)
        Request req5 = new Request();
        req5.setParameter("username", "Charlie");
        req5.setParameter("message", "A".repeat(1000));
        Response res5 = new Response();
        System.out.println("Test 5 - Long message: " + post(req5, res5));
        
        // Display all messages
        System.out.println("\\n=== Displaying Messages ===");
        List<String> messages = displayMessages();
        messages.forEach(System.out::println);
    }
}
