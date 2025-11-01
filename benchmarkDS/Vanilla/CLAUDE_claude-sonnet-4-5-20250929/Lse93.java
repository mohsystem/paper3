
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.sql.*;
import java.util.Scanner;

public class Lse93 {
    
    public static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hashedBytes = md.digest(password.getBytes());
            StringBuilder sb = new StringBuilder();
            for (byte b : hashedBytes) {
                sb.append(String.format("%02x", b));
            }
            return sb.toString();
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static boolean insertUser(String username, String password) {
        String hashedPassword = hashPassword(password);
        
        // Using SQLite for demonstration
        String url = "jdbc:sqlite:users.db";
        String createTableSQL = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT)";
        String insertSQL = "INSERT INTO users (username, password) VALUES (?, ?)";
        
        try (Connection conn = DriverManager.getConnection(url);
             Statement stmt = conn.createStatement()) {
            
            stmt.execute(createTableSQL);
            
            try (PreparedStatement pstmt = conn.prepareStatement(insertSQL)) {
                pstmt.setString(1, username);
                pstmt.setString(2, hashedPassword);
                pstmt.executeUpdate();
                return true;
            }
        } catch (SQLException e) {
            System.out.println("Error inserting user: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        boolean result1 = insertUser("user1", "password123");
        System.out.println("User1 inserted: " + result1);
        
        System.out.println("\\nTest Case 2:");
        boolean result2 = insertUser("user2", "securePass456");
        System.out.println("User2 inserted: " + result2);
        
        System.out.println("\\nTest Case 3:");
        boolean result3 = insertUser("user3", "mySecret789");
        System.out.println("User3 inserted: " + result3);
        
        System.out.println("\\nTest Case 4:");
        boolean result4 = insertUser("user4", "testPass000");
        System.out.println("User4 inserted: " + result4);
        
        System.out.println("\\nTest Case 5:");
        boolean result5 = insertUser("user5", "finalTest111");
        System.out.println("User5 inserted: " + result5);
    }
}
