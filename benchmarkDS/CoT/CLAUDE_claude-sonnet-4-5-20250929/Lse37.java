
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.sql.*;
import java.util.HashMap;
import java.util.Map;

public class Lse37 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    // Simulated database for demonstration
    private static Map<String, String> userDatabase = new HashMap<>();
    
    static {
        // Initialize with hashed passwords (SHA-256 for demo, use BCrypt in production)
        try {
            userDatabase.put("admin", hashPassword("admin123"));
            userDatabase.put("user1", hashPassword("password1"));
            userDatabase.put("john", hashPassword("securePass"));
            userDatabase.put("alice", hashPassword("alice2023"));
            userDatabase.put("bob", hashPassword("bobSecret"));
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
    }
    
    public static String hashPassword(String password) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        byte[] hash = md.digest(password.getBytes());
        StringBuilder hexString = new StringBuilder();
        for (byte b : hash) {
            String hex = Integer.toHexString(0xff & b);
            if (hex.length() == 1) hexString.append('0');
            hexString.append(hex);
        }
        return hexString.toString();
    }
    
    public static String login(String username, String password) {
        try {
            // Input validation
            if (username == null || password == null || 
                username.trim().isEmpty() || password.trim().isEmpty()) {
                return "cant_login";
            }
            
            // Sanitize input
            username = username.trim();
            
            // Hash the provided password
            String hashedPassword = hashPassword(password);
            
            // Check credentials (simulated database query)
            String storedHash = userDatabase.get(username);
            
            if (storedHash != null && storedHash.equals(hashedPassword)) {
                return "success";
            } else {
                return "cant_login";
            }
            
        } catch (Exception e) {
            // Log error securely (don't expose details to user)
            System.err.println("Login error: " + e.getClass().getName());
            return "internal_server_error";
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Login System Test Cases ===\\n");
        
        // Test Case 1: Valid credentials
        String result1 = login("admin", "admin123");
        System.out.println("Test 1 - Valid login (admin): " + result1);
        
        // Test Case 2: Invalid password
        String result2 = login("admin", "wrongpass");
        System.out.println("Test 2 - Invalid password: " + result2);
        
        // Test Case 3: Non-existent user
        String result3 = login("hacker", "password");
        System.out.println("Test 3 - Non-existent user: " + result3);
        
        // Test Case 4: Empty credentials
        String result4 = login("", "");
        System.out.println("Test 4 - Empty credentials: " + result4);
        
        // Test Case 5: Null inputs
        String result5 = login(null, null);
        System.out.println("Test 5 - Null inputs: " + result5);
    }
}
