
import java.util.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Lse39 {
    private static Map<String, String> userDatabase = new HashMap<>();
    
    // Initialize with some test users (password stored as hash)
    static {
        // Storing hashed passwords (SHA-256)
        userDatabase.put("admin", hashPassword("admin123"));
        userDatabase.put("user1", hashPassword("password1"));
        userDatabase.put("user2", hashPassword("password2"));
        userDatabase.put("testuser", hashPassword("test123"));
        userDatabase.put("john", hashPassword("john456"));
    }
    
    // Hash password using SHA-256
    private static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hash = md.digest(password.getBytes());
            StringBuilder hexString = new StringBuilder();
            for (byte b : hash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) hexString.append('0');
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
    }
    
    // Secure login function
    public static String login(String username, String password) {
        // Input validation
        if (username == null || password == null || username.trim().isEmpty() || password.trim().isEmpty()) {
            return "Error: Username or password cannot be empty";
        }
        
        // Check if user exists and password matches
        String storedHash = userDatabase.get(username);
        if (storedHash != null && storedHash.equals(hashPassword(password))) {
            return "Success: Redirecting to session page for user: " + username;
        } else {
            return "Error: Incorrect username or password";
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Login System Test Cases ===\\n");
        
        // Test Case 1: Valid credentials
        System.out.println("Test Case 1: Valid login (admin/admin123)");
        System.out.println(login("admin", "admin123"));
        System.out.println();
        
        // Test Case 2: Invalid password
        System.out.println("Test Case 2: Invalid password (admin/wrongpass)");
        System.out.println(login("admin", "wrongpass"));
        System.out.println();
        
        // Test Case 3: Non-existent user
        System.out.println("Test Case 3: Non-existent user (hacker/hack123)");
        System.out.println(login("hacker", "hack123"));
        System.out.println();
        
        // Test Case 4: Empty credentials
        System.out.println("Test Case 4: Empty username");
        System.out.println(login("", "password"));
        System.out.println();
        
        // Test Case 5: Valid user2 login
        System.out.println("Test Case 5: Valid login (user2/password2)");
        System.out.println(login("user2", "password2"));
    }
}
