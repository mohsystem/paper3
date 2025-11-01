
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.sql.*;
import java.util.Scanner;

public class Lse99 {
    
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
        String url = "jdbc:sqlite:users.db";
        
        try {
            Connection conn = DriverManager.getConnection(url);
            String createTableSQL = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password TEXT)";
            Statement stmt = conn.createStatement();
            stmt.execute(createTableSQL);
            
            String insertSQL = "INSERT INTO users (username, password) VALUES (?, ?)";
            PreparedStatement pstmt = conn.prepareStatement(insertSQL);
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            pstmt.executeUpdate();
            
            pstmt.close();
            stmt.close();
            conn.close();
            return true;
        } catch (SQLException e) {
            System.out.println("Error: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        System.out.println(insertUser("user1", "password123") ? "Success" : "Failed");
        
        System.out.println("\\nTest Case 2:");
        System.out.println(insertUser("user2", "securePass456") ? "Success" : "Failed");
        
        System.out.println("\\nTest Case 3:");
        System.out.println(insertUser("admin", "admin@2024") ? "Success" : "Failed");
        
        System.out.println("\\nTest Case 4:");
        System.out.println(insertUser("testuser", "test1234") ? "Success" : "Failed");
        
        System.out.println("\\nTest Case 5:");
        System.out.println(insertUser("john_doe", "mySecret99") ? "Success" : "Failed");
    }
}
