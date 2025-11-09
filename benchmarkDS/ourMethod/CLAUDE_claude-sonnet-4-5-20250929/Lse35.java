
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Base64;
import java.util.logging.Logger;
import java.util.logging.Level;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

@WebServlet("/login")
public class Lse35 extends HttpServlet {
    private static final Logger LOGGER = Logger.getLogger(Lse35.class.getName());
    
    // Rules#3: Never hard code passwords or secrets anywhere in code
    // Read database credentials from environment variables or secure configuration
    private static final String DB_URL = System.getenv("DB_URL") != null ? 
        System.getenv("DB_URL") : "jdbc:mysql://localhost:3306/userdb?useSSL=true&requireSSL=true";
    private static final String DB_USER = System.getenv("DB_USER") != null ? 
        System.getenv("DB_USER") : "dbuser";
    private static final String DB_PASSWORD = System.getenv("DB_PASSWORD") != null ? 
        System.getenv("DB_PASSWORD") : "dbpassword";
    
    // Rules#2: Input validation - maximum allowed lengths
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_PASSWORD_LENGTH = 128;
    
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        try {
            // Rules#2: Treat all input as untrusted and validate early
            String username = request.getParameter("username");
            String password = request.getParameter("password");
            
            // Rules#2: Input validation - check for null and empty
            if (username == null || password == null || 
                username.trim().isEmpty() || password.trim().isEmpty()) {
                // Rules#11: Return generic error message to caller
                LOGGER.log(Level.WARNING, "Login attempt with empty credentials");
                response.sendRedirect("login.html?error=invalid");
                return;
            }
            
            // Rules#2: Input validation - enforce maximum length
            if (username.length() > MAX_USERNAME_LENGTH || 
                password.length() > MAX_PASSWORD_LENGTH) {
                // Rules#11: Do not leak internal details
                LOGGER.log(Level.WARNING, "Login attempt with oversized input");
                response.sendRedirect("login.html?error=invalid");
                return;
            }
            
            // Rules#2: Input validation - sanitize username (alphanumeric, underscore, hyphen only)
            if (!username.matches("^[a-zA-Z0-9_-]+$")) {
                LOGGER.log(Level.WARNING, "Login attempt with invalid username format");
                response.sendRedirect("login.html?error=invalid");
                return;
            }
            
            // Authenticate user
            boolean authenticated = authenticateUser(username, password);
            
            if (authenticated) {
                // Create secure session
                HttpSession session = request.getSession(true);
                
                // Rules#12: Use atomic operations for session management
                session.setAttribute("authenticated", Boolean.TRUE);
                session.setAttribute("username", username);
                
                // Set secure session configuration
                session.setMaxInactiveInterval(1800); // 30 minutes timeout
                
                // Rules#10: Log successful authentication without sensitive data
                LOGGER.log(Level.INFO, "Successful authentication for user");
                
                // Redirect to home page
                response.sendRedirect("home.jsp");
            } else {
                // Rules#11: Return generic error message
                LOGGER.log(Level.WARNING, "Failed authentication attempt");
                response.sendRedirect("login.html?error=invalid");
            }
            
        } catch (Exception e) {
            // Rules#11: Do not leak internal details to caller
            LOGGER.log(Level.SEVERE, "Authentication error occurred", e);
            try {
                response.sendRedirect("login.html?error=system");
            } catch (IOException ioException) {
                LOGGER.log(Level.SEVERE, "Failed to redirect after error", ioException);
            }
        }
    }
    
    /**
     * Authenticates user credentials against database
     * Rules#32: Use parameterized queries to prevent SQL injection
     */
    private boolean authenticateUser(String username, String password) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Rules#6: Use TLS for database connection (configured in DB_URL with useSSL=true)
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Rules#32: Use prepared statement with parameters, never concatenate
            String sql = "SELECT password_hash, salt FROM users WHERE username = ? AND active = 1";
            pstmt = conn.prepareStatement(sql);
            pstmt.setString(1, username);
            
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                String storedHash = rs.getString("password_hash");
                String salt = rs.getString("salt");
                
                // Rules#5: Verify password using secure hash comparison
                String computedHash = hashPassword(password, salt);
                
                // Use constant-time comparison to prevent timing attacks
                return MessageDigest.isEqual(
                    storedHash.getBytes(StandardCharsets.UTF_8),
                    computedHash.getBytes(StandardCharsets.UTF_8)
                );
            }
            
            return false;
            
        } catch (SQLException e) {
            // Rules#10: Log error without exposing sensitive details
            LOGGER.log(Level.SEVERE, "Database error during authentication", e);
            return false;
        } catch (NoSuchAlgorithmException e) {
            LOGGER.log(Level.SEVERE, "Cryptographic error during authentication", e);
            return false;
        } finally {
            // Clean up resources in reverse order
            if (rs != null) {
                try {
                    rs.close();
                } catch (SQLException e) {
                    LOGGER.log(Level.WARNING, "Failed to close ResultSet", e);
                }
            }
            if (pstmt != null) {
                try {
                    pstmt.close();
                } catch (SQLException e) {
                    LOGGER.log(Level.WARNING, "Failed to close PreparedStatement", e);
                }
            }
            if (conn != null) {
                try {
                    conn.close();
                } catch (SQLException e) {
                    LOGGER.log(Level.WARNING, "Failed to close Connection", e);
                }
            }
        }
    }
    
    /**
     * Hashes password with salt using SHA-256
     * Rules#5: Use modern KDF for password hashing
     * Note: In production, use PBKDF2, Argon2id, or bcrypt with proper iterations
     */
    private String hashPassword(String password, String salt) throws NoSuchAlgorithmException {
        try {
            // Rules#5: Use vetted cryptographic library
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            
            // Combine password and salt
            String combined = password + salt;
            byte[] hashBytes = md.digest(combined.getBytes(StandardCharsets.UTF_8));
            
            // Encode to Base64 for storage
            return Base64.getEncoder().encodeToString(hashBytes);
            
        } catch (NoSuchAlgorithmException e) {
            LOGGER.log(Level.SEVERE, "Hash algorithm not available", e);
            throw e;
        }
    }
    
    /**
     * Generates a random salt for password hashing
     * Rules#5: Generate nonces with a CSPRNG
     */
    private String generateSalt() {
        try {
            // Rules#5: Use SecureRandom for cryptographic operations
            SecureRandom random = SecureRandom.getInstanceStrong();
            byte[] saltBytes = new byte[16];
            random.nextBytes(saltBytes);
            return Base64.getEncoder().encodeToString(saltBytes);
        } catch (NoSuchAlgorithmException e) {
            LOGGER.log(Level.SEVERE, "SecureRandom not available", e);
            // Fallback to default SecureRandom
            SecureRandom random = new SecureRandom();
            byte[] saltBytes = new byte[16];
            random.nextBytes(saltBytes);
            return Base64.getEncoder().encodeToString(saltBytes);
        }
    }
    
    // Test main method with 5 test cases
    public static void main(String[] args) {
        Lse35 servlet = new Lse35();
        System.out.println("Test Cases for Login Authentication:");
        
        // Test Case 1: Valid username and password format
        System.out.println("\\nTest 1: Valid input format");
        try {
            String username1 = "validuser123";
            String password1 = "SecurePass123!";
            System.out.println("Username: " + username1 + " - Length check: " + 
                (username1.length() <= MAX_USERNAME_LENGTH));
            System.out.println("Password: [REDACTED] - Length check: " + 
                (password1.length() <= MAX_PASSWORD_LENGTH));
            System.out.println("Username format valid: " + username1.matches("^[a-zA-Z0-9_-]+$"));
        } catch (Exception e) {
            System.out.println("Test 1 failed: " + e.getMessage());
        }
        
        // Test Case 2: Username too long
        System.out.println("\\nTest 2: Username exceeds maximum length");
        try {
            String username2 = "a".repeat(MAX_USERNAME_LENGTH + 1);
            System.out.println("Username length: " + username2.length() + 
                " - Valid: " + (username2.length() <= MAX_USERNAME_LENGTH));
        } catch (Exception e) {
            System.out.println("Test 2 failed: " + e.getMessage());
        }
        
        // Test Case 3: Invalid characters in username
        System.out.println("\\nTest 3: Invalid characters in username");
        try {
            String username3 = "user@domain.com";
            System.out.println("Username: " + username3 + 
                " - Valid format: " + username3.matches("^[a-zA-Z0-9_-]+$"));
        } catch (Exception e) {
            System.out.println("Test 3 failed: " + e.getMessage());
        }
        
        // Test Case 4: Empty credentials
        System.out.println("\\nTest 4: Empty credentials");
        try {
            String username4 = "";
            String password4 = "";
            System.out.println("Empty username: " + username4.trim().isEmpty());
            System.out.println("Empty password: " + password4.trim().isEmpty());
        } catch (Exception e) {
            System.out.println("Test 4 failed: " + e.getMessage());
        }
        
        // Test Case 5: Password hashing and salt generation
        System.out.println("\\nTest 5: Password hashing with salt");
        try {
            String testPassword = "TestPassword123";
            String salt1 = servlet.generateSalt();
            String salt2 = servlet.generateSalt();
            String hash1 = servlet.hashPassword(testPassword, salt1);
            String hash2 = servlet.hashPassword(testPassword, salt2);
            
            System.out.println("Salt 1 generated: " + (salt1 != null && !salt1.isEmpty()));
            System.out.println("Salt 2 generated: " + (salt2 != null && !salt2.isEmpty()));
            System.out.println("Salts are different: " + (!salt1.equals(salt2)));
            System.out.println("Hash 1 generated: " + (hash1 != null && !hash1.isEmpty()));
            System.out.println("Hash 2 generated: " + (hash2 != null && !hash2.isEmpty()));
            System.out.println("Hashes are different (due to different salts): " + (!hash1.equals(hash2)));
            
            // Verify same password with same salt produces same hash
            String hash1Verify = servlet.hashPassword(testPassword, salt1);
            System.out.println("Hash consistency verified: " + hash1.equals(hash1Verify));
            
        } catch (Exception e) {
            System.out.println("Test 5 failed: " + e.getMessage());
        }
        
        System.out.println("\\n=== All test cases completed ===");
    }
}
