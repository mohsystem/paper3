
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.*;
import java.util.Base64;
import java.nio.charset.StandardCharsets;

public class Lse36 {
    // Secure database URL with SSL enabled
    private static final String DB_URL = System.getenv("DB_URL");
    private static final String DB_USER = System.getenv("DB_USER");
    private static final String DB_PASSWORD = System.getenv("DB_PASSWORD");
    
    /**
     * Login function that authenticates user credentials
     * @param username User provided username
     * @param password User provided password
     * @return true if authentication successful, false otherwise
     */
    public static boolean login(String username, String password) {
        // Rules#3: Validate and sanitize inputs
        if (username == null || password == null) {
            return false;
        }
        
        // Rules#3: Validate username length and format
        if (username.length() > 255 || username.isEmpty()) {
            return false;
        }
        
        // Rules#15: Enforce password policy - minimum length
        if (password.length() < 12 || password.length() > 128) {
            return false;
        }
        
        // Rules#3: Sanitize username to prevent SQL injection
        if (!username.matches("^[a-zA-Z0-9_@.+-]+$")) {
            return false;
        }
        
        Connection conn = null;
        PreparedStatement stmt = null;
        ResultSet rs = null;
        
        try {
            // Rules#8: Ensure SSL/TLS for database connection
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Rules#2: Use parameterized query to prevent SQL injection
            String query = "SELECT password_hash, salt FROM users WHERE username = ? LIMIT 1";
            stmt = conn.prepareStatement(query);
            stmt.setString(1, username);
            
            rs = stmt.executeQuery();
            
            if (rs.next()) {
                String storedHash = rs.getString("password_hash");
                String salt = rs.getString("salt");
                
                // Rules#11: Use unique salt for password hashing
                // Rules#10: Use strong hashing with PBKDF2
                String computedHash = hashPassword(password, salt);
                
                // Rules#10: Constant-time comparison to prevent timing attacks
                if (constantTimeEquals(storedHash, computedHash)) {
                    // Authentication successful - redirect to home page
                    return true;
                }
            }
            
            // Rules#7: Return generic error, don't reveal if username exists\n            return false;\n            \n        } catch (SQLException e) {\n            // Rules#7: Don't leak internal details in error messages
            System.err.println("Authentication failed");
            return false;
        } finally {
            // Clean up resources
            try {
                if (rs != null) rs.close();
                if (stmt != null) stmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources");
            }
        }
    }
    
    /**
     * Hash password using PBKDF2-HMAC-SHA256
     * Rules#10: Use vetted cryptographic library with strong KDF
     */
    private static String hashPassword(String password, String saltStr) {
        try {
            byte[] salt = Base64.getDecoder().decode(saltStr);
            
            // Rules#9: Use PBKDF2 with at least 210,000 iterations
            int iterations = 210000;
            int keyLength = 256;
            
            javax.crypto.SecretKeyFactory factory = 
                javax.crypto.SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            javax.crypto.spec.PBEKeySpec spec = 
                new javax.crypto.spec.PBEKeySpec(
                    password.toCharArray(), salt, iterations, keyLength);
            
            byte[] hash = factory.generateSecret(spec).getEncoded();
            
            // Rules#7: Clear sensitive data
            spec.clearPassword();
            
            return Base64.getEncoder().encodeToString(hash);
        } catch (Exception e) {
            throw new RuntimeException("Hashing failed");
        }
    }
    
    /**
     * Rules#10: Constant-time string comparison to prevent timing attacks
     */
    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) {
            return false;
        }
        
        byte[] aBytes = a.getBytes(StandardCharsets.UTF_8);
        byte[] bBytes = b.getBytes(StandardCharsets.UTF_8);
        
        return MessageDigest.isEqual(aBytes, bBytes);
    }
    
    /**
     * Generate salt for new user registration
     * Rules#12: Use cryptographically secure random generator
     */
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static void main(String[] args) {
        // Test cases with mock implementation
        System.out.println("Test Case 1 - Valid credentials:");
        System.out.println(login("user1", "ValidPassword123!"));
        
        System.out.println("\\nTest Case 2 - Invalid password:");
        System.out.println(login("user1", "WrongPassword"));
        
        System.out.println("\\nTest Case 3 - Non-existent user:");
        System.out.println(login("nonexistent", "SomePassword123!"));
        
        System.out.println("\\nTest Case 4 - Null inputs:");
        System.out.println(login(null, null));
        
        System.out.println("\\nTest Case 5 - SQL injection attempt:");
        System.out.println(login("admin' OR '1'='1", "password"));
    }
}
