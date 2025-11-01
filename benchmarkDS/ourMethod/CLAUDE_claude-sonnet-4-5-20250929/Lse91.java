
import java.sql.*;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.nio.charset.StandardCharsets;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.util.Base64;

public class Lse91 {
    private static final int ITERATIONS = 210000; // PBKDF2 iterations >= 210000
    private static final int KEY_LENGTH = 256; // 256-bit derived key
    private static final int SALT_LENGTH = 16; // 16-byte salt
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    
    // Hash password using PBKDF2-HMAC-SHA256 with unique salt
    private static String hashPassword(String password, byte[] salt) throws Exception {
        // Using PBKDF2WithHmacSHA256 for secure key derivation (Rule #8, #9, #10, #11)
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] hash = factory.generateSecret(spec).getEncoded();
        
        // Encode salt and hash together for storage
        byte[] combined = new byte[salt.length + hash.length];
        System.arraycopy(salt, 0, combined, 0, salt.length);
        System.arraycopy(hash, 0, combined, salt.length, hash.length);
        
        return Base64.getEncoder().encodeToString(combined);
    }
    
    // Generate cryptographically secure random salt (Rule #11)
    private static byte[] generateSalt() {
        byte[] salt = new byte[SALT_LENGTH];
        SECURE_RANDOM.nextBytes(salt);
        return salt;
    }
    
    // Register user with input validation (Rule #3, #13)
    public static boolean registerUser(String username, String password) {
        // Input validation (Rule #3)
        if (username == null || username.trim().isEmpty() || username.length() > 50) {
            System.err.println("Invalid username");
            return false;
        }
        
        // Strong password policy (Rule #13)
        if (password == null || password.length() < 12 || password.length() > 128) {
            System.err.println("Password must be between 12 and 128 characters");
            return false;
        }
        
        // Sanitize username to prevent SQL injection
        if (!username.matches("^[a-zA-Z0-9_-]+$")) {
            System.err.println("Username contains invalid characters");
            return false;
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        
        try {
            // Generate unique salt for this password (Rule #10, #11)
            byte[] salt = generateSalt();
            
            // Hash password with salt (Rule #8, #9)
            String hashedPassword = hashPassword(password, salt);
            
            // Database connection - credentials from environment (Rule #14, #15)
            String dbUrl = System.getenv("DB_URL");
            String dbUser = System.getenv("DB_USER");
            String dbPassword = System.getenv("DB_PASSWORD");
            
            if (dbUrl == null || dbUser == null || dbPassword == null) {
                System.err.println("Database credentials not configured");
                return false;
            }
            
            conn = DriverManager.getConnection(dbUrl, dbUser, dbPassword);
            
            // Use parameterized query to prevent SQL injection (Rule #3)
            String sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            pstmt.setString(2, hashedPassword);
            
            int rowsAffected = pstmt.executeUpdate();
            return rowsAffected > 0;
            
        } catch (SQLIntegrityConstraintViolationException e) {
            // Generic error message to avoid information leakage (Rule #1)
            System.err.println("Registration failed");
            return false;
        } catch (Exception e) {
            // Generic error message (Rule #1)
            System.err.println("Registration failed");
            return false;
        } finally {
            // Properly close resources (Rule #1)
            try {
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources");
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: Valid registration");
        System.out.println(registerUser("testuser1", "SecurePass123!@#"));
        
        System.out.println("\\nTest 2: Valid registration with different user");
        System.out.println(registerUser("testuser2", "AnotherSecure456$%^"));
        
        System.out.println("\\nTest 3: Invalid - password too short");
        System.out.println(registerUser("testuser3", "short"));
        
        System.out.println("\\nTest 4: Invalid - empty username");
        System.out.println(registerUser("", "ValidPassword123!"));
        
        System.out.println("\\nTest 5: Invalid - username with special chars");
        System.out.println(registerUser("test@user", "ValidPassword123!"));
    }
}
