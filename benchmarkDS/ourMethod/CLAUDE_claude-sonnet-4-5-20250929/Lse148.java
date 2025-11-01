
import java.io.*;
import java.sql.*;
import java.util.*;
import javax.servlet.http.*;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;

public class Lse148 {
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_MESSAGE_LENGTH = 500;
    private static final String DB_URL_ENV = "DB_URL"; // Read from environment
    
    // Simulated servlet-style post handler
    public static String post(Map<String, String> request, Map<String, String> session) {
        try {
            // Rule#2: Input validation - check all parameters exist
            String username = request.get("username");
            String message = request.get("message");
            String csrfToken = request.get("csrf_token");
            String sessionToken = session.get("csrf_token");
            
            // Validate CSRF token to prevent cross-site request forgery
            if (csrfToken == null || sessionToken == null || !csrfToken.equals(sessionToken)) {
                return "error"; // Rule#11: Generic error message
            }
            
            // Rule#2: Validate inputs are not null and within bounds
            if (username == null || message == null) {
                return "error";
            }
            
            // Rule#2: Validate length constraints
            if (username.length() == 0 || username.length() > MAX_USERNAME_LENGTH) {
                return "error";
            }
            if (message.length() == 0 || message.length() > MAX_MESSAGE_LENGTH) {
                return "error";
            }
            
            // Rule#2: Validate encoding and reject control characters
            if (!isValidInput(username) || !isValidInput(message)) {
                return "error";
            }
            
            // Insert into database using safe parameterized query
            insertUserMessageInDb(username, message);
            
            // Rule#11: Redirect to main page (return redirect target)
            return "redirect:/main";
            
        } catch (Exception e) {
            // Rule#11: Do not leak internal details
            // Rule#10: Log error securely without sensitive data
            System.err.println("Error processing post request");
            return "error";
        }
    }
    
    // Rule#2: Validate input contains only safe characters
    private static boolean isValidInput(String input) {
        // Check for valid UTF-8 and no control characters except newline/tab
        for (char c : input.toCharArray()) {
            if (Character.isISOControl(c) && c != '\\n' && c != '\\t') {
                return false;
            }
        }
        return true;
    }
    
    // Rule#32: Use parameterized queries to prevent SQL injection
    private static void insertUserMessageInDb(String username, String message) throws SQLException {
        // Rule#3: Read connection string from environment, not hardcoded
        String dbUrl = System.getenv(DB_URL_ENV);
        if (dbUrl == null) {
            dbUrl = "jdbc:sqlite:messages.db"; // Fallback for demo
        }
        
        // Rule#32: Parameterized query prevents SQL injection
        String sql = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, ?)";
        
        try (Connection conn = DriverManager.getConnection(dbUrl);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            // Rule#32: Set parameters safely
            pstmt.setString(1, username);
            pstmt.setString(2, message);
            pstmt.setTimestamp(3, new Timestamp(System.currentTimeMillis()));
            
            pstmt.executeUpdate();
        }
    }
    
    // Display messages with XSS protection
    public static List<Map<String, String>> getMessages() {
        List<Map<String, String>> messages = new ArrayList<>();
        
        try {
            String dbUrl = System.getenv(DB_URL_ENV);
            if (dbUrl == null) {
                dbUrl = "jdbc:sqlite:messages.db";
            }
            
            // Rule#32: Safe parameterized query
            String sql = "SELECT username, message, created_at FROM messages ORDER BY created_at DESC LIMIT 100";
            
            try (Connection conn = DriverManager.getConnection(dbUrl);
                 PreparedStatement pstmt = conn.prepareStatement(sql);
                 ResultSet rs = pstmt.executeQuery()) {
                
                while (rs.next()) {
                    Map<String, String> msg = new HashMap<>();
                    // Rule#2: Retrieve and validate data
                    String username = rs.getString("username");
                    String message = rs.getString("message");
                    String timestamp = rs.getString("created_at");
                    
                    // Rule#2: Escape for HTML display to prevent XSS
                    msg.put("username", escapeHtml(username));
                    msg.put("message", escapeHtml(message));
                    msg.put("timestamp", escapeHtml(timestamp));
                    messages.add(msg);
                }
            }
        } catch (Exception e) {
            // Rule#11: Generic error handling
            System.err.println("Error retrieving messages");
        }
        
        return messages;
    }
    
    // Rule#2: Escape HTML to prevent XSS attacks
    private static String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    // Generate CSRF token using secure random
    private static String generateCsrfToken() {
        SecureRandom random = new SecureRandom();
        byte[] bytes = new byte[32];
        random.nextBytes(bytes);
        return Base64.getEncoder().encodeToString(bytes);
    }
    
    public static void main(String[] args) {
        // Test case setup: Create in-memory database
        try {
            String dbUrl = "jdbc:sqlite::memory:";
            Connection conn = DriverManager.getConnection(dbUrl);
            Statement stmt = conn.createStatement();
            stmt.execute("CREATE TABLE messages (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT, message TEXT, created_at TIMESTAMP)");
            conn.close();
            System.setProperty(DB_URL_ENV, dbUrl);
        } catch (Exception e) {
            System.err.println("Setup failed");
        }
        
        // Test case 1: Valid post
        Map<String, String> req1 = new HashMap<>();
        Map<String, String> session1 = new HashMap<>();
        String token1 = generateCsrfToken();
        req1.put("username", "alice");
        req1.put("message", "Hello world!");
        req1.put("csrf_token", token1);
        session1.put("csrf_token", token1);
        System.out.println("Test 1: " + post(req1, session1));
        
        // Test case 2: Another valid post
        Map<String, String> req2 = new HashMap<>();
        Map<String, String> session2 = new HashMap<>();
        String token2 = generateCsrfToken();
        req2.put("username", "bob");
        req2.put("message", "Test message");
        req2.put("csrf_token", token2);
        session2.put("csrf_token", token2);
        System.out.println("Test 2: " + post(req2, session2));
        
        // Test case 3: Invalid CSRF token
        Map<String, String> req3 = new HashMap<>();
        Map<String, String> session3 = new HashMap<>();
        req3.put("username", "eve");
        req3.put("message", "Attack");
        req3.put("csrf_token", "wrong_token");
        session3.put("csrf_token", "correct_token");
        System.out.println("Test 3 (should error): " + post(req3, session3));
        
        // Test case 4: Oversized input
        Map<String, String> req4 = new HashMap<>();
        Map<String, String> session4 = new HashMap<>();
        String token4 = generateCsrfToken();
        req4.put("username", "x".repeat(100));
        req4.put("message", "msg");
        req4.put("csrf_token", token4);
        session4.put("csrf_token", token4);
        System.out.println("Test 4 (should error): " + post(req4, session4));
        
        // Test case 5: XSS attempt
        Map<String, String> req5 = new HashMap<>();
        Map<String, String> session5 = new HashMap<>();
        String token5 = generateCsrfToken();
        req5.put("username", "hacker");
        req5.put("message", "<script>alert('xss')</script>");
        req5.put("csrf_token", token5);
        session5.put("csrf_token", token5);
        System.out.println("Test 5: " + post(req5, session5));
        
        // Display all messages
        System.out.println("\\nMessages:");
        List<Map<String, String>> messages = getMessages();
        for (Map<String, String> msg : messages) {
            System.out.println(msg.get("username") + ": " + msg.get("message"));
        }
    }
}
