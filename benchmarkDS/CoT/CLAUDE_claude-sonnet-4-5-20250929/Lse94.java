
import java.security.SecureRandom;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.util.Base64;

public class Lse94 {
    private static final int SALT_LENGTH = 16;
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
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
    
    public static boolean insertUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || 
            password == null || password.length() < 8) {
            return false;
        }
        
        String salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        String combinedHash = salt + ":" + hashedPassword;
        
        String url = "jdbc:sqlite:users.db";
        String sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
        
        try (Connection conn = DriverManager.getConnection(url);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, username);
            pstmt.setString(2, combinedHash);
            pstmt.executeUpdate();
            return true;
        } catch (Exception e) {
            System.err.println("Error inserting user: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + insertUser("user1", "password123"));
        System.out.println("Test 2: " + insertUser("user2", "securePass456"));
        System.out.println("Test 3: " + insertUser("user3", "myP@ssw0rd"));
        System.out.println("Test 4: " + insertUser("", "password123"));
        System.out.println("Test 5: " + insertUser("user5", "short"));
    }
}
