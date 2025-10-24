
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;

public class Task102 {
    
    // Method to generate a random salt
    private static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    // Method to hash password with salt using SHA-256
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
    
    // Method to update user password in database
    public static boolean updateUserPassword(String username, String newPassword, Connection conn) {
        if (username == null || username.trim().isEmpty() || 
            newPassword == null || newPassword.trim().isEmpty()) {
            return false;
        }
        
        String salt = generateSalt();
        String hashedPassword = hashPassword(newPassword, salt);
        
        String updateQuery = "UPDATE users SET password_hash = ?, salt = ? WHERE username = ?";
        
        try (PreparedStatement pstmt = conn.prepareStatement(updateQuery)) {
            pstmt.setString(1, hashedPassword);
            pstmt.setString(2, salt);
            pstmt.setString(3, username);
            
            int rowsAffected = pstmt.executeUpdate();
            return rowsAffected > 0;
        } catch (SQLException e) {
            System.err.println("Error updating password: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Database connection setup (using in-memory H2 database for testing)
        String jdbcUrl = "jdbc:h2:mem:testdb";
        
        try (Connection conn = DriverManager.getConnection(jdbcUrl, "sa", "")) {
            // Create test table
            String createTable = "CREATE TABLE users (" +
                                "id INT AUTO_INCREMENT PRIMARY KEY, " +
                                "username VARCHAR(255) UNIQUE NOT NULL, " +
                                "password_hash VARCHAR(255), " +
                                "salt VARCHAR(255))";
            conn.createStatement().execute(createTable);
            
            // Insert test users
            String insertUser = "INSERT INTO users (username) VALUES (?)";
            PreparedStatement pstmt = conn.prepareStatement(insertUser);
            String[] testUsers = {"user1", "user2", "user3", "user4", "user5"};
            for (String user : testUsers) {
                pstmt.setString(1, user);
                pstmt.executeUpdate();
            }
            
            // Test case 1: Valid username and password
            System.out.println("Test 1: " + updateUserPassword("user1", "SecurePass123!", conn));
            
            // Test case 2: Another valid update
            System.out.println("Test 2: " + updateUserPassword("user2", "MyP@ssw0rd", conn));
            
            // Test case 3: Non-existent user
            System.out.println("Test 3: " + updateUserPassword("nonexistent", "password", conn));
            
            // Test case 4: Empty password
            System.out.println("Test 4: " + updateUserPassword("user3", "", conn));
            
            // Test case 5: Null username
            System.out.println("Test 5: " + updateUserPassword(null, "password", conn));
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        }
    }
}
