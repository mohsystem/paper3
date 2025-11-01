
import java.sql.*;
import java.util.Properties;
import java.io.FileInputStream;
import java.io.IOException;
import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.util.Base64;

public class Lse37 {
    private static final int ITERATIONS = 210000;
    private static final int KEY_LENGTH = 256;
    private static final int SALT_LENGTH = 16;
    
    // Securely loads database credentials from environment variables (Rules#3, #4)
    private static Properties loadDbConfig() {
        Properties props = new Properties();
        props.setProperty("url", System.getenv("DB_URL"));
        props.setProperty("user", System.getenv("DB_USER"));
        props.setProperty("password", System.getenv("DB_PASSWORD"));
        return props;
    }
    
    // Hashes password using PBKDF2-HMAC-SHA256 (Rules#1, #6, #7, #8, #9)
    private static String hashPassword(String password, byte[] salt) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] hash = skf.generateSecret(spec).getEncoded();
        spec.clearPassword(); // Clear sensitive data
        return Base64.getEncoder().encodeToString(hash);
    }
    
    // Generates cryptographically secure random salt (Rules#8, #9)
    private static byte[] generateSalt() {
        SecureRandom sr = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        sr.nextBytes(salt);
        return salt;
    }
    
    // Validates input to prevent injection attacks (Rules#1, #13)
    private static boolean validateInput(String input) {
        if (input == null || input.isEmpty() || input.length() > 255) {
            return false;
        }
        // Reject inputs with suspicious characters
        return input.matches("^[a-zA-Z0-9_@.\\\\-]+$");
    }
    
    // Authenticates user with parameterized queries to prevent SQL injection (Rules#1, #2)
    public static String authenticateUser(String username, String password) {
        // Input validation (Rules#1, #13)
        if (!validateInput(username) || password == null || password.isEmpty()) {
            return "cant_login";
        }
        
        Properties dbConfig = loadDbConfig();
        if (dbConfig.getProperty("url") == null || dbConfig.getProperty("user") == null) {
            return "internal_server_error";
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Use TLS 1.2+ for database connection if applicable (Rules#5, #6, #7)
            conn = DriverManager.getConnection(
                dbConfig.getProperty("url"),
                dbConfig.getProperty("user"),
                dbConfig.getProperty("password")
            );
            
            // Parameterized query prevents SQL injection (Rules#1, #2)
            String query = "SELECT password_hash, salt FROM users WHERE username = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setString(1, username);
            
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                String storedHash = rs.getString("password_hash");
                byte[] salt = Base64.getDecoder().decode(rs.getString("salt"));
                
                // Hash provided password with stored salt (Rules#6, #7, #8)
                String computedHash = hashPassword(password, salt);
                
                // Constant-time comparison to prevent timing attacks (Rules#6)
                if (MessageDigest.isEqual(
                    storedHash.getBytes(StandardCharsets.UTF_8),
                    computedHash.getBytes(StandardCharsets.UTF_8))) {
                    return "success";
                } else {
                    return "cant_login";
                }
            } else {
                return "cant_login";
            }
            
        } catch (SQLException e) {
            // Generic error message, detailed logging separately (Rules#1)
            System.err.println("Database error occurred");
            return "internal_server_error";
        } catch (Exception e) {
            // Handle cryptographic exceptions (Rules#17)
            System.err.println("Authentication error occurred");
            return "internal_server_error";
        } finally {
            // Ensure resources are closed (Rules#17)
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources");
            }
        }
    }
    
    public static void main(String[] args) {
        // Test cases with mock credentials
        System.out.println("Test 1 - Valid user: " + authenticateUser("validuser", "ValidPass123!"));
        System.out.println("Test 2 - Invalid password: " + authenticateUser("validuser", "wrongpass"));
        System.out.println("Test 3 - Invalid username: " + authenticateUser("invaliduser", "password"));
        System.out.println("Test 4 - SQL injection attempt: " + authenticateUser("admin' OR '1'='1", "pass"));
        System.out.println("Test 5 - Empty credentials: " + authenticateUser("", ""));
    }
}
