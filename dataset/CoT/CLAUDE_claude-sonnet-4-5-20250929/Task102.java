
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.util.Base64;

public class Task102 {
    
    public static String hashPassword(String password) {
        try {
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);
            
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedPassword = md.digest(password.getBytes());
            
            byte[] combined = new byte[salt.length + hashedPassword.length];
            System.arraycopy(salt, 0, combined, 0, salt.length);
            System.arraycopy(hashedPassword, 0, combined, salt.length, hashedPassword.length);
            
            return Base64.getEncoder().encodeToString(combined);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static boolean updateUserPassword(String username, String newPassword) {
        if (username == null || username.trim().isEmpty() || 
            newPassword == null || newPassword.trim().isEmpty()) {
            return false;
        }
        
        if (username.length() > 255 || newPassword.length() > 255) {
            return false;
        }
        
        String hashedPassword = hashPassword(newPassword);
        
        String url = "jdbc:sqlite:users.db";
        String sql = "UPDATE users SET password = ? WHERE username = ?";
        
        try (Connection conn = DriverManager.getConnection(url);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, hashedPassword);
            pstmt.setString(2, username);
            
            int rowsAffected = pstmt.executeUpdate();
            return rowsAffected > 0;
            
        } catch (Exception e) {
            System.err.println("Error updating password");
            return false;
        }
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: " + updateUserPassword("user1", "SecurePass123!"));
        System.out.println("Test Case 2: " + updateUserPassword("user2", "AnotherP@ssw0rd"));
        System.out.println("Test Case 3: " + updateUserPassword("admin", "AdminSecure#456"));
        System.out.println("Test Case 4: " + updateUserPassword("", "password"));
        System.out.println("Test Case 5: " + updateUserPassword("user3", ""));
    }
}
