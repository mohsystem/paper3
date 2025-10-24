
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.nio.charset.StandardCharsets;

public class Task102 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/userdb";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static String hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
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
    
    public static boolean updatePassword(String username, String newPassword) {
        String hashedPassword = hashPassword(newPassword);
        String sql = "UPDATE users SET password = ? WHERE username = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setString(1, hashedPassword);
            pstmt.setString(2, username);
            
            int rowsAffected = pstmt.executeUpdate();
            return rowsAffected > 0;
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        boolean result1 = updatePassword("john_doe", "newPassword123");
        System.out.println("Update result for john_doe: " + result1);
        
        System.out.println("\\nTest Case 2:");
        boolean result2 = updatePassword("jane_smith", "SecurePass456!");
        System.out.println("Update result for jane_smith: " + result2);
        
        System.out.println("\\nTest Case 3:");
        boolean result3 = updatePassword("admin", "Admin@2024");
        System.out.println("Update result for admin: " + result3);
        
        System.out.println("\\nTest Case 4:");
        boolean result4 = updatePassword("test_user", "TestPass789#");
        System.out.println("Update result for test_user: " + result4);
        
        System.out.println("\\nTest Case 5:");
        boolean result5 = updatePassword("guest", "GuestPassword2024");
        System.out.println("Update result for guest: " + result5);
    }
}
