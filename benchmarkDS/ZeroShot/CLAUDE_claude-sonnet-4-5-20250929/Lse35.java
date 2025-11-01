
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse35 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/userdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static class LoginResult {
        public boolean success;
        public String redirectUrl;
        
        public LoginResult(boolean success, String redirectUrl) {
            this.success = success;
            this.redirectUrl = redirectUrl;
        }
    }
    
    private static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static LoginResult login(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            return new LoginResult(false, "login.html");
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
                    return new LoginResult(true, "home.html");
                }
            }
            return new LoginResult(false, "login.html");
            
        } catch (SQLException e) {
            e.printStackTrace();
            return new LoginResult(false, "login.html");
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
        // Test cases
        System.out.println("Test 1 - Valid user:");
        LoginResult result1 = login("john_doe", "SecurePass123");
        System.out.println("Success: " + result1.success + ", Redirect: " + result1.redirectUrl);
        
        System.out.println("\\nTest 2 - Invalid password:");
        LoginResult result2 = login("john_doe", "WrongPass");
        System.out.println("Success: " + result2.success + ", Redirect: " + result2.redirectUrl);
        
        System.out.println("\\nTest 3 - Non-existent user:");
        LoginResult result3 = login("fake_user", "password");
        System.out.println("Success: " + result3.success + ", Redirect: " + result3.redirectUrl);
        
        System.out.println("\\nTest 4 - Empty username:");
        LoginResult result4 = login("", "password");
        System.out.println("Success: " + result4.success + ", Redirect: " + result4.redirectUrl);
        
        System.out.println("\\nTest 5 - Null inputs:");
        LoginResult result5 = login(null, null);
        System.out.println("Success: " + result5.success + ", Redirect: " + result5.redirectUrl);
    }
}
