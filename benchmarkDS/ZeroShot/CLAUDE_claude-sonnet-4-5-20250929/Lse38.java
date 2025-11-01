
import java.sql.*;
import java.util.Scanner;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Lse38 {
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
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
    
    // Initialize database with test users
    private static void initDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            stmt.execute("CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT)");
            stmt.execute("DELETE FROM users");
            
            // Insert test users with hashed passwords
            String[] testUsers = {"admin", "user1", "user2", "user3", "user4"};
            String[] testPasswords = {"admin123", "pass123", "password", "test123", "user123"};
            
            for (int i = 0; i < testUsers.length; i++) {
                String hashedPass = hashPassword(testPasswords[i]);
                PreparedStatement pstmt = conn.prepareStatement("INSERT INTO users VALUES (?, ?)");
                pstmt.setString(1, testUsers[i]);
                pstmt.setString(2, hashedPass);
                pstmt.executeUpdate();
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
    
    // Secure login function using prepared statements
    public static boolean login(String username, String password) {
        String hashedPassword = hashPassword(password);
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(
                 "SELECT * FROM users WHERE username = ? AND password = ?")) {
            
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            
            ResultSet rs = pstmt.executeQuery();
            return rs.next();
            
        } catch (SQLException e) {
            e.printStackTrace();
            return false;
        }
    }
    
    // Redirect simulation
    public static String authenticate(String username, String password) {
        if (login(username, password)) {
            return "REDIRECT: Dashboard - Welcome " + username + "!";
        } else {
            return "REDIRECT: Login Page - Invalid credentials";
        }
    }
    
    public static void main(String[] args) {
        // Initialize database with test data
        initDatabase();
        
        System.out.println("=== Secure Login System Test Cases ===\\n");
        
        // Test case 1: Valid admin login
        System.out.println("Test 1 - Valid admin:");
        System.out.println(authenticate("admin", "admin123"));
        
        // Test case 2: Valid user login
        System.out.println("\\nTest 2 - Valid user1:");
        System.out.println(authenticate("user1", "pass123"));
        
        // Test case 3: Invalid password
        System.out.println("\\nTest 3 - Invalid password:");
        System.out.println(authenticate("admin", "wrongpass"));
        
        // Test case 4: Non-existent user
        System.out.println("\\nTest 4 - Non-existent user:");
        System.out.println(authenticate("hacker", "hack123"));
        
        // Test case 5: SQL injection attempt (protected)
        System.out.println("\\nTest 5 - SQL injection attempt:");
        System.out.println(authenticate("admin' OR '1'='1", "anything"));
    }
}
