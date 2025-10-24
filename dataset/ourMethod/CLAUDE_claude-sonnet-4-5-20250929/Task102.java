
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.sql.*;
import java.util.Base64;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task102 {
    private static final int SALT_LENGTH = 16;
    private static final int ITERATIONS = 210000;
    private static final int KEY_LENGTH = 256;
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";
    
    public static String hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException, InvalidKeySpecException {
        if (password == null || password.isEmpty()) {
            throw new IllegalArgumentException("Password cannot be null or empty");
        }
        if (salt == null || salt.length != SALT_LENGTH) {
            throw new IllegalArgumentException("Invalid salt");
        }
        
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
        byte[] hash = factory.generateSecret(spec).getEncoded();
        
        byte[] combined = new byte[SALT_LENGTH + hash.length];
        System.arraycopy(salt, 0, combined, 0, SALT_LENGTH);
        System.arraycopy(hash, 0, combined, SALT_LENGTH, hash.length);
        
        return Base64.getEncoder().encodeToString(combined);
    }
    
    public static boolean updateUserPassword(String username, String newPassword, Connection conn) {
        if (username == null || username.isEmpty() || username.length() > 255) {
            throw new IllegalArgumentException("Invalid username");
        }
        if (newPassword == null || newPassword.length() < 8 || newPassword.length() > 128) {
            throw new IllegalArgumentException("Password must be between 8 and 128 characters");
        }
        
        String sqlCheck = "SELECT COUNT(*) FROM users WHERE username = ?";
        String sqlUpdate = "UPDATE users SET password_hash = ? WHERE username = ?";
        
        try {
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[SALT_LENGTH];
            random.nextBytes(salt);
            
            String hashedPassword = hashPassword(newPassword, salt);
            
            try (PreparedStatement checkStmt = conn.prepareStatement(sqlCheck)) {
                checkStmt.setString(1, username);
                try (ResultSet rs = checkStmt.executeQuery()) {
                    if (rs.next() && rs.getInt(1) == 0) {
                        return false;
                    }
                }
            }
            
            try (PreparedStatement updateStmt = conn.prepareStatement(sqlUpdate)) {
                updateStmt.setString(1, hashedPassword);
                updateStmt.setString(2, username);
                int rowsAffected = updateStmt.executeUpdate();
                return rowsAffected > 0;
            }
        } catch (SQLException | NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Error updating password: " + e.getMessage());
            return false;
        }
    }
    
    public static void main(String[] args) {
        String url = "jdbc:h2:mem:testdb";
        
        try (Connection conn = DriverManager.getConnection(url)) {
            try (Statement stmt = conn.createStatement()) {
                stmt.execute("CREATE TABLE users (username VARCHAR(255) PRIMARY KEY, password_hash VARCHAR(500))");
                stmt.execute("INSERT INTO users (username, password_hash) VALUES ('user1', 'old_hash')");
                stmt.execute("INSERT INTO users (username, password_hash) VALUES ('user2', 'old_hash')");
                stmt.execute("INSERT INTO users (username, password_hash) VALUES ('user3', 'old_hash')");
                stmt.execute("INSERT INTO users (username, password_hash) VALUES ('user4', 'old_hash')");
                stmt.execute("INSERT INTO users (username, password_hash) VALUES ('user5', 'old_hash')");
            }
            
            System.out.println("Test 1: " + updateUserPassword("user1", "NewPass123!", conn));
            System.out.println("Test 2: " + updateUserPassword("user2", "SecureP@ssw0rd", conn));
            System.out.println("Test 3: " + updateUserPassword("user3", "Str0ng!Pass", conn));
            System.out.println("Test 4: " + updateUserPassword("nonexistent", "Password123", conn));
            System.out.println("Test 5: " + updateUserPassword("user5", "MyS3cur3P@ss", conn));
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        }
    }
}
