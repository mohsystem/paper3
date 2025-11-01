
import java.sql.*;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class Lse38 {
    private static final String DB_URL = System.getenv("DB_URL");
    private static final String DB_USER = System.getenv("DB_USER");
    private static final String DB_PASSWORD = System.getenv("DB_PASSWORD");
    private static final int MAX_LOGIN_ATTEMPTS = 5;
    
    // Secure password hashing with PBKDF2
    public static String hashPassword(String password, byte[] salt) {
        try {
            // Using PBKDF2-HMAC-SHA256 with 210000 iterations
            javax.crypto.SecretKeyFactory factory = javax.crypto.SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            javax.crypto.spec.PBEKeySpec spec = new javax.crypto.spec.PBEKeySpec(
                password.toCharArray(), 
                salt, 
                210000, 
                256
            );
            byte[] hash = factory.generateSecret(spec).getEncoded();
            spec.clearPassword(); // Clear sensitive data
            return Base64.getEncoder().encodeToString(hash);
        } catch (Exception e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    // Generate cryptographically secure random salt
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
    
    // Secure login function with parameterized queries to prevent SQL injection
    public static String login(String username, String password) {
        // Input validation - reject null, empty, or excessively long inputs
        if (username == null || username.isEmpty() || username.length() > 255) {
            return "redirect:/login?error=invalid";
        }
        if (password == null || password.isEmpty() || password.length() > 1024) {
            return "redirect:/login?error=invalid";
        }
        
        // Sanitize username - allow only alphanumeric and underscore
        if (!username.matches("^[a-zA-Z0-9_]{3,255}$")) {
            return "redirect:/login?error=invalid";
        }
        
        Connection conn = null;
        PreparedStatement stmt = null;
        ResultSet rs = null;
        
        try {
            // Establish secure database connection with TLS
            java.util.Properties props = new java.util.Properties();
            props.setProperty("user", DB_USER);
            props.setProperty("password", DB_PASSWORD);
            props.setProperty("ssl", "true");
            props.setProperty("sslmode", "verify-full");
            
            conn = DriverManager.getConnection(DB_URL, props);
            
            // Use parameterized query to prevent SQL injection
            String query = "SELECT password_hash, salt FROM users WHERE username = ? AND account_locked = FALSE";
            stmt = conn.prepareStatement(query);
            stmt.setString(1, username);
            
            rs = stmt.executeQuery();
            
            if (rs.next()) {
                String storedHash = rs.getString("password_hash");
                byte[] salt = Base64.getDecoder().decode(rs.getString("salt"));
                
                // Hash provided password with stored salt
                String computedHash = hashPassword(password, salt);
                
                // Constant-time comparison to prevent timing attacks
                if (MessageDigest.isEqual(
                    computedHash.getBytes(StandardCharsets.UTF_8),
                    storedHash.getBytes(StandardCharsets.UTF_8)
                )) {
                    // Successful authentication - redirect to dashboard
                    return "redirect:/dashboard";
                } else {
                    // Failed authentication - generic error message
                    return "redirect:/login?error=invalid";
                }
            } else {
                // User not found - generic error message (prevent username enumeration)
                return "redirect:/login?error=invalid";
            }
            
        } catch (SQLException e) {
            // Log error securely without exposing sensitive details
            System.err.println("Database error during login");
            return "redirect:/login?error=system";
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
    
    public static void main(String[] args) {
        // Test case 1: Valid login
        System.out.println("Test 1: " + login("validuser", "ValidPass123!"));
        
        // Test case 2: Invalid password
        System.out.println("Test 2: " + login("validuser", "wrongpassword"));
        
        // Test case 3: Non-existent user
        System.out.println("Test 3: " + login("nonexistent", "password"));
        
        // Test case 4: SQL injection attempt (blocked by validation)
        System.out.println("Test 4: " + login("admin' OR '1'='1", "password"));
        
        // Test case 5: Empty credentials
        System.out.println("Test 5: " + login("", ""));
    }
}
