
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.util.Base64;

public class Lse95 {
    private static final int SALT_LENGTH = 16; // 128 bits
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final String DB_URL = "jdbc:sqlite:users.db";
    
    // Generate cryptographically secure random salt
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }
    
    // Hash password with PBKDF2-HMAC-SHA256
    private static byte[] hashPassword(String password, byte[] salt) {
        try {
            javax.crypto.spec.PBEKeySpec spec = new javax.crypto.spec.PBEKeySpec(
                password.toCharArray(), salt, PBKDF2_ITERATIONS, 256);
            javax.crypto.SecretKeyFactory factory = javax.crypto.SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return factory.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    // Register user with validated input
    public static boolean registerUser(String username, String password) {
        // Input validation - reject empty or excessively long inputs
        if (username == null || username.trim().isEmpty() || username.length() > 100) {
            return false;
        }
        if (password == null || password.length() < 8 || password.length() > 128) {
            return false;
        }
        
        // Sanitize username - allow only alphanumeric and underscore
        if (!username.matches("^[a-zA-Z0-9_]+$")) {
            return false;
        }
        
        try (Connection conn = DriverManager.getConnection(DB_URL)) {
            // Create table if not exists
            String createTable = "CREATE TABLE IF NOT EXISTS users (" +
                               "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                               "username TEXT UNIQUE NOT NULL, " +
                               "password_hash TEXT NOT NULL, " +
                               "salt TEXT NOT NULL)";
            try (PreparedStatement stmt = conn.prepareStatement(createTable)) {
                stmt.execute();
            }
            
            // Generate unique salt for this user
            byte[] salt = generateSalt();
            
            // Hash password with salt using PBKDF2
            byte[] hash = hashPassword(password, salt);
            
            // Encode to Base64 for storage
            String saltBase64 = Base64.getEncoder().encodeToString(salt);
            String hashBase64 = Base64.getEncoder().encodeToString(hash);
            
            // Store in database using parameterized query to prevent SQL injection
            String insertSQL = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
            try (PreparedStatement pstmt = conn.prepareStatement(insertSQL)) {
                pstmt.setString(1, username);
                pstmt.setString(2, hashBase64);
                pstmt.setString(3, saltBase64);
                pstmt.executeUpdate();
            }
            
            return true;
        } catch (Exception e) {
            // Log internally but return generic error to user
            System.err.println("Registration failed");
            return false;
        }
    }
    
    // Verify user credentials during login
    public static boolean verifyUser(String username, String password) {
        // Input validation
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            return false;
        }
        
        try (Connection conn = DriverManager.getConnection(DB_URL)) {
            // Retrieve stored hash and salt using parameterized query
            String selectSQL = "SELECT password_hash, salt FROM users WHERE username = ?";
            try (PreparedStatement pstmt = conn.prepareStatement(selectSQL)) {
                pstmt.setString(1, username);
                try (ResultSet rs = pstmt.executeQuery()) {
                    if (rs.next()) {
                        String storedHashBase64 = rs.getString("password_hash");
                        String storedSaltBase64 = rs.getString("salt");
                        
                        // Decode from Base64
                        byte[] storedHash = Base64.getDecoder().decode(storedHashBase64);
                        byte[] storedSalt = Base64.getDecoder().decode(storedSaltBase64);
                        
                        // Hash provided password with stored salt
                        byte[] computedHash = hashPassword(password, storedSalt);
                        
                        // Constant-time comparison to prevent timing attacks
                        return MessageDigest.isEqual(storedHash, computedHash);
                    }
                }
            }
        } catch (Exception e) {
            // Log internally but return generic error
            System.err.println("Login failed");
        }
        
        return false;
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid registration
        System.out.println("Test 1 - Valid registration: " + 
            registerUser("user1", "StrongP@ss123"));
        
        // Test case 2: Valid login
        System.out.println("Test 2 - Valid login: " + 
            verifyUser("user1", "StrongP@ss123"));
        
        // Test case 3: Invalid login (wrong password)
        System.out.println("Test 3 - Invalid password: " + 
            verifyUser("user1", "WrongPassword"));
        
        // Test case 4: Invalid username (SQL injection attempt)
        System.out.println("Test 4 - SQL injection attempt: " + 
            registerUser("admin' OR '1'='1", "password123"));
        
        // Test case 5: Weak password (too short)
        System.out.println("Test 5 - Weak password: " + 
            registerUser("user2", "weak"));
    }
}
