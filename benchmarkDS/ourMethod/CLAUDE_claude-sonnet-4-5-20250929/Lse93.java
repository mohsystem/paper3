
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.Base64;

public class Lse93 {
    private static final int SALT_LENGTH = 16; // 128-bit salt
    private static final String DB_URL = "jdbc:sqlite::memory:"; // In-memory DB for testing
    
    /**
     * Generates a cryptographically secure random salt.
     * Uses SecureRandom to ensure unpredictability and uniqueness.
     * @return Base64-encoded salt string
     */
    private static String generateSalt() {
        SecureRandom secureRandom = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        secureRandom.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    /**
     * Hashes password with salt using SHA-256.
     * Note: For production, use bcrypt, scrypt, or Argon2id instead of SHA-256.
     * This implementation demonstrates the salting concept.
     * @param password Plain text password
     * @param salt Unique salt for this password
     * @return Base64-encoded hash
     */
    private static String hashPassword(String password, String salt) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            // Combine password and salt before hashing
            String saltedPassword = password + salt;
            byte[] hash = digest.digest(saltedPassword.getBytes(StandardCharsets.UTF_8));
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 algorithm not available", e);
        }
    }
    
    /**
     * Validates username input to prevent SQL injection and ensure format.
     * Accepts only alphanumeric characters, underscore, and hyphen.
     * Length between 3 and 50 characters.
     * @param username Input username
     * @return true if valid, false otherwise
     */
    private static boolean isValidUsername(String username) {
        if (username == null || username.length() < 3 || username.length() > 50) {
            return false;
        }
        return username.matches("^[a-zA-Z0-9_-]+$");
    }
    
    /**
     * Validates password input to ensure it meets security requirements.
     * Minimum 8 characters, maximum 128 characters.
     * @param password Input password
     * @return true if valid, false otherwise
     */
    private static boolean isValidPassword(String password) {
        if (password == null || password.length() < 8 || password.length() > 128) {
            return false;
        }
        return true;
    }
    
    /**
     * Inserts username and hashed password into database.
     * Uses parameterized queries to prevent SQL injection.
     * Stores both the hash and salt for future verification.
     * @param username Validated username
     * @param password Plain text password (will be hashed)
     * @return true if insertion successful, false otherwise
     */
    public static boolean insertUser(String username, String password) {
        // Validate inputs before processing
        if (!isValidUsername(username)) {
            System.err.println("Invalid username format");
            return false;
        }
        
        if (!isValidPassword(password)) {
            System.err.println("Invalid password format");
            return false;
        }
        
        // Generate unique salt for this password
        String salt = generateSalt();
        
        // Hash password with salt
        String hashedPassword = hashPassword(password, salt);
        
        // Use try-with-resources to ensure proper resource cleanup
        try (Connection conn = DriverManager.getConnection(DB_URL)) {
            // Create table if not exists
            try (PreparedStatement createTable = conn.prepareStatement(
                "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password_hash TEXT NOT NULL, salt TEXT NOT NULL)")) {
                createTable.execute();
            }
            
            // Use parameterized query to prevent SQL injection
            String sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
            try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                pstmt.setString(1, username);
                pstmt.setString(2, hashedPassword);
                pstmt.setString(3, salt);
                
                int rowsAffected = pstmt.executeUpdate();
                return rowsAffected > 0;
            }
        } catch (SQLException e) {
            System.err.println("Database error occurred");
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid username and password
        System.out.println("Test 1: " + insertUser("john_doe", "SecurePass123!"));
        
        // Test case 2: Another valid user
        System.out.println("Test 2: " + insertUser("jane-smith", "MyP@ssw0rd2024"));
        
        // Test case 3: Invalid username (too short)
        System.out.println("Test 3: " + insertUser("ab", "ValidPassword1"));
        
        // Test case 4: Invalid password (too short)
        System.out.println("Test 4: " + insertUser("validuser", "short"));
        
        // Test case 5: Valid edge case username and password
        System.out.println("Test 5: " + insertUser("user123", "LongPassword123456"));
    }
}
