
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
    
    // Hash password with SHA-256
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
    
    // Generate salt for new users
    private static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    // Main login function with prepared statements to prevent SQL injection
    public static LoginResult login(String username, String password) {
        // Input validation
        if (username == null || username.trim().isEmpty() || 
            password == null || password.trim().isEmpty()) {
            return new LoginResult(false, "login.html?error=empty");
        }
        
        // Sanitize username (alphanumeric only)
        if (!username.matches("^[a-zA-Z0-9_]{3,20}$")) {
            return new LoginResult(false, "login.html?error=invalid");
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Establish secure database connection
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Use prepared statement to prevent SQL injection
            String query = "SELECT password_hash, salt FROM users WHERE username = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setString(1, username);
            
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                String storedHash = rs.getString("password_hash");
                String salt = rs.getString("salt");
                
                // Hash the input password with stored salt
                String inputHash = hashPassword(password, salt);
                
                // Constant-time comparison to prevent timing attacks
                if (MessageDigest.isEqual(inputHash.getBytes(), storedHash.getBytes())) {
                    return new LoginResult(true, "home.html");
                }
            }
            
            // Invalid credentials
            return new LoginResult(false, "login.html?error=invalid");
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return new LoginResult(false, "login.html?error=system");
        } finally {
            // Clean up resources
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources: " + e.getMessage());
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1 - Valid user:");
        LoginResult result1 = login("validuser", "correctpassword");
        System.out.println("Success: " + result1.success + ", Redirect: " + result1.redirectUrl);
        
        System.out.println("\\nTest Case 2 - Invalid password:");
        LoginResult result2 = login("validuser", "wrongpassword");
        System.out.println("Success: " + result2.success + ", Redirect: " + result2.redirectUrl);
        
        System.out.println("\\nTest Case 3 - Non-existent user:");
        LoginResult result3 = login("nonexistent", "anypassword");
        System.out.println("Success: " + result3.success + ", Redirect: " + result3.redirectUrl);
        
        System.out.println("\\nTest Case 4 - Empty username:");
        LoginResult result4 = login("", "password");
        System.out.println("Success: " + result4.success + ", Redirect: " + result4.redirectUrl);
        
        System.out.println("\\nTest Case 5 - SQL injection attempt:");
        LoginResult result5 = login("admin' OR '1'='1", "password");
        System.out.println("Success: " + result5.success + ", Redirect: " + result5.redirectUrl);
    }
}
