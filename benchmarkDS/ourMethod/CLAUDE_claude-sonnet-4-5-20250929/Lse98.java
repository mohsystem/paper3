
import java.sql.*;
import java.security.SecureRandom;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.nio.charset.StandardCharsets;
import java.util.Base64;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.spec.KeySpec;

public class Lse98 {
    private static final int SALT_LENGTH = 16; // 128 bits
    private static final int ITERATIONS = 210000; // OWASP recommendation for PBKDF2
    private static final int KEY_LENGTH = 256; // 256 bits
    private static final SecureRandom secureRandom = new SecureRandom();
    
    // Generate cryptographically secure random salt - CWE-759, CWE-330
    public static byte[] generateSalt() {
        byte[] salt = new byte[SALT_LENGTH];
        secureRandom.nextBytes(salt);
        return salt;
    }
    
    // Hash password using PBKDF2-HMAC-SHA256 - secure KDF
    public static String hashPassword(String password, byte[] salt) throws Exception {
        // Use PBKDF2 with SHA-256, high iteration count per OWASP guidelines
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] hash = factory.generateSecret(spec).getEncoded();
        
        // Encode salt and hash together for storage
        byte[] combined = new byte[salt.length + hash.length];
        System.arraycopy(salt, 0, combined, 0, salt.length);
        System.arraycopy(hash, 0, combined, salt.length, hash.length);
        
        return Base64.getEncoder().encodeToString(combined);
    }
    
    // Register user with secure password hashing
    public static boolean registerUser(String username, String password, Connection conn) {
        // Input validation - CWE-20
        if (username == null || username.trim().isEmpty() || username.length() > 100) {
            System.err.println("Invalid username");
            return false;
        }
        
        // Password policy enforcement - CWE-521
        if (password == null || password.length() < 12 || password.length() > 128) {
            System.err.println("Password must be between 12 and 128 characters");
            return false;
        }
        
        // Check password complexity
        if (!password.matches(".*[A-Z].*") || !password.matches(".*[a-z].*") || 
            !password.matches(".*[0-9].*") || !password.matches(".*[!@#$%^&*()_+\\\\-=\\\\[\\\\]{};':\\"\\\\\\\\|,.<>\\\\/?].*")) {
            System.err.println("Password must contain uppercase, lowercase, digit, and special character");
            return false;
        }
        
        try {
            // Generate unique salt for this user - CWE-759, CWE-330
            byte[] salt = generateSalt();
            
            // Hash password with salt using secure KDF - CWE-327
            String hashedPassword = hashPassword(password, salt);
            
            // Use prepared statement to prevent SQL injection - CWE-89
            String sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
            try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                pstmt.setString(1, username);
                pstmt.setString(2, hashedPassword);
                pstmt.executeUpdate();
                return true;
            }
        } catch (SQLIntegrityConstraintViolationException e) {
            // Generic error message - don't leak database details - CWE-209
            System.err.println("Registration failed: username may already exist");
            return false;
        } catch (Exception e) {
            // Generic error message - CWE-209
            System.err.println("Registration failed due to system error");
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Database connection with TLS enforcement - CWE-319
        String url = "jdbc:mysql://localhost:3306/userdb?useSSL=true&requireSSL=true&verifyServerCertificate=true";
        String dbUser = System.getenv("DB_USER"); // Credentials from environment - CWE-798
        String dbPassword = System.getenv("DB_PASSWORD"); // CWE-798
        
        if (dbUser == null || dbPassword == null) {
            System.err.println("Database credentials not found in environment variables");
            return;
        }
        
        try (Connection conn = DriverManager.getConnection(url, dbUser, dbPassword)) {
            // Test case 1: Valid registration
            System.out.println("Test 1: " + registerUser("alice", "SecurePass123!@#", conn));
            
            // Test case 2: Valid registration with different user
            System.out.println("Test 2: " + registerUser("bob", "MyP@ssw0rd2024!", conn));
            
            // Test case 3: Weak password (too short)
            System.out.println("Test 3: " + registerUser("charlie", "Short1!", conn));
            
            // Test case 4: Password without special character
            System.out.println("Test 4: " + registerUser("david", "NoSpecialChar123", conn));
            
            // Test case 5: Empty username
            System.out.println("Test 5: " + registerUser("", "ValidPass123!@#", conn));
            
        } catch (SQLException e) {
            System.err.println("Database connection failed");
        }
    }
}
