
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse36 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/userdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static String login(String username, String password) {
        if (username == null || password == null || username.trim().isEmpty() || password.trim().isEmpty()) {
            return "Invalid credentials";
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            String query = "SELECT password_hash, salt FROM users WHERE username = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setString(1, username);
            
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                String storedHash = rs.getString("password_hash");
                String salt = rs.getString("salt");
                String inputHash = hashPassword(password, salt);
                
                if (storedHash.equals(inputHash)) {
                    return "Redirect to home page";
                } else {
                    return "Incorrect password";
                }
            } else {
                return "User not found";
            }
        } catch (SQLException e) {
            return "Database error: " + e.getMessage();
        } finally {
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid credentials");
        System.out.println(login("user1", "password123"));
        
        System.out.println("\\nTest Case 2: Invalid password");
        System.out.println(login("user1", "wrongpassword"));
        
        System.out.println("\\nTest Case 3: Non-existent user");
        System.out.println(login("nonexistent", "password123"));
        
        System.out.println("\\nTest Case 4: Empty username");
        System.out.println(login("", "password123"));
        
        System.out.println("\\nTest Case 5: Null values");
        System.out.println(login(null, null));
    }
}
