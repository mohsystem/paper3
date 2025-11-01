
import java.sql.*;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class Lse99 {
    // Database connection parameters - should be loaded from environment variables in production
    private static final String DB_URL = System.getenv().getOrDefault("DB_URL", "jdbc:postgresql://localhost:5432/testdb");
    private static final String DB_USER = System.getenv().getOrDefault("DB_USER", "testuser");
    private static final String DB_PASSWORD = System.getenv().getOrDefault("DB_PASSWORD", "testpass");
    
    // Password policy constants
    private static final int MIN_PASSWORD_LENGTH = 12;
    private static final int MAX_PASSWORD_LENGTH = 128;
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int PBKDF2_ITERATIONS = 210000; // OWASP recommendation for PBKDF2-HMAC-SHA256
    private static final int SALT_LENGTH = 16; // 128 bits
    
    /**
     * Generates a cryptographically secure random salt
     * Uses SecureRandom to ensure unpredictability (CWE-330, CWE-759)
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }
    
    /**
     * Hashes password using PBKDF2-HMAC-SHA256 with secure salt
     * Prevents rainbow table attacks (CWE-759) and uses strong KDF (CWE-916)
     */
    private static String hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException {
        try {
            // Use PBKDF2WithHmacSHA256 as recommended for password hashing
            javax.crypto.SecretKeyFactory factory = javax.crypto.SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            javax.crypto.spec.PBEKeySpec spec = new javax.crypto.spec.PBEKeySpec(
                password.toCharArray(), 
                salt, 
                PBKDF2_ITERATIONS, 
                256 // 256-bit output
            );
            byte[] hash = factory.generateSecret(spec).getEncoded();
            
            // Clear sensitive data from memory
            spec.clearPassword();
            
            // Return salt + hash encoded in Base64 for storage
            byte[] combined = new byte[salt.length + hash.length];
            System.arraycopy(salt, 0, combined, 0, salt.length);
            System.arraycopy(hash, 0, combined, salt.length, hash.length);
            
            return Base64.getEncoder().encodeToString(combined);
        } catch (Exception e) {
            throw new NoSuchAlgorithmException("Password hashing failed", e);
        }
    }
    
    /**
     * Validates username input to prevent injection attacks (CWE-89)
     * Checks length and character whitelist
     */
    private static boolean isValidUsername(String username) {
        if (username == null || username.isEmpty()) {
            return false;
        }
        
        // Check length constraints (CWE-20)
        if (username.length() > MAX_USERNAME_LENGTH) {
            return false;
        }
        
        // Whitelist: alphanumeric, underscore, hyphen only
        return username.matches("^[a-zA-Z0-9_-]+$");
    }
    
    /**
     * Validates password meets security policy (CWE-521)
     */
    private static boolean isValidPassword(String password) {
        if (password == null) {
            return false;
        }
        
        // Check length constraints
        if (password.length() < MIN_PASSWORD_LENGTH || password.length() > MAX_PASSWORD_LENGTH) {
            return false;
        }
        
        // Check complexity: must have uppercase, lowercase, digit, and special character
        boolean hasUpper = password.chars().anyMatch(Character::isUpperCase);
        boolean hasLower = password.chars().anyMatch(Character::isLowerCase);
        boolean hasDigit = password.chars().anyMatch(Character::isDigit);
        boolean hasSpecial = password.chars().anyMatch(ch -> "!@#$%^&*()_+-=[]{}|;:,.<>?".indexOf(ch) >= 0);
        
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }
    
    /**
     * Inserts user into database with parameterized query to prevent SQL injection (CWE-89)
     */
    public static boolean registerUser(String username, String password) {
        // Input validation (CWE-20)
        if (!isValidUsername(username)) {
            System.err.println("Invalid username format");
            return false;
        }
        
        if (!isValidPassword(password)) {
            System.err.println("Password does not meet security requirements");
            return false;
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            // Generate unique salt for this password (CWE-759, CWE-330)
            byte[] salt = generateSalt();
            
            // Hash password with salt using PBKDF2 (CWE-916)
            String hashedPassword = hashPassword(password, salt);
            
            // Establish database connection
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Use parameterized query to prevent SQL injection (CWE-89)
            String sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            
            int rowsAffected = pstmt.executeUpdate();
            
            return rowsAffected > 0;
            
        } catch (SQLException e) {
            // Generic error message to prevent information disclosure (CWE-209)
            System.err.println("Database operation failed");
            return false;
        } catch (NoSuchAlgorithmException e) {
            System.err.println("Cryptographic operation failed");
            return false;
        } finally {
            // Ensure resources are closed (CWE-404)
            try {
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Resource cleanup failed");
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases with secure passwords meeting policy requirements
        System.out.println("Test 1: Valid user");
        System.out.println(registerUser("testuser1", "SecureP@ssw0rd123"));
        
        System.out.println("\\nTest 2: Valid user with special chars");
        System.out.println(registerUser("john_doe", "MyP@ssw0rd!2024"));
        
        System.out.println("\\nTest 3: Invalid username (too long)");
        System.out.println(registerUser("a".repeat(51), "ValidP@ss123"));
        
        System.out.println("\\nTest 4: Invalid password (too short)");
        System.out.println(registerUser("alice", "Short1!"));
        
        System.out.println("\\nTest 5: Invalid username (special chars)");
        System.out.println(registerUser("user'; DROP TABLE users--", "SecureP@ssw0rd123"));
    }
}
