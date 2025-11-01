
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

public class Lse97 {
    private static Connection conn;
    
    public static String hashPassword(String password) {
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
    
    public static String registerUser(String username, String password, String firstName, String lastName, String email) {
        try {
            // Initialize SQLite connection if not exists
            if (conn == null || conn.isClosed()) {
                conn = DriverManager.getConnection("jdbc:sqlite:users.db");
                Statement stmt = conn.createStatement();
                stmt.execute("CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT, firstName TEXT, lastName TEXT, email TEXT)");
            }
            
            // Check if username exists
            PreparedStatement checkStmt = conn.prepareStatement("SELECT username FROM users WHERE username = ?");
            checkStmt.setString(1, username);
            ResultSet rs = checkStmt.executeQuery();
            
            if (rs.next()) {
                return "Username already exists";
            }
            
            // Hash password
            String hashedPassword = hashPassword(password);
            
            // Insert user
            PreparedStatement insertStmt = conn.prepareStatement("INSERT INTO users (username, password, firstName, lastName, email) VALUES (?, ?, ?, ?, ?)");
            insertStmt.setString(1, username);
            insertStmt.setString(2, hashedPassword);
            insertStmt.setString(3, firstName);
            insertStmt.setString(4, lastName);
            insertStmt.setString(5, email);
            insertStmt.executeUpdate();
            
            return "Registration succeeded";
        } catch (SQLException e) {
            return "Registration failed: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println(registerUser("john_doe", "password123", "John", "Doe", "john@example.com"));
        System.out.println(registerUser("jane_smith", "securePass456", "Jane", "Smith", "jane@example.com"));
        System.out.println(registerUser("john_doe", "anotherPass", "John", "Doe", "john2@example.com"));
        System.out.println(registerUser("bob_wilson", "bobPass789", "Bob", "Wilson", "bob@example.com"));
        System.out.println(registerUser("alice_brown", "alicePass321", "Alice", "Brown", "alice@example.com"));
        
        try {
            if (conn != null) conn.close();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }
}
