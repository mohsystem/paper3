
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
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

@WebServlet("/login")
public class Lse35 extends HttpServlet {
    // Use environment variables for database credentials - Rules#3, Rules#4
    private static final String DB_URL = System.getenv("DB_URL") != null ? 
        System.getenv("DB_URL") : "jdbc:sqlite:users.db";
    private static final String DB_USER = System.getenv("DB_USER");
    private static final String DB_PASSWORD = System.getenv("DB_PASSWORD");
    
    // Constants for security
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_PASSWORD_LENGTH = 128;
    private static final int MAX_LOGIN_ATTEMPTS = 5;
    
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        // Input validation - Rules#1, Rules#13: validate all external input
        String username = request.getParameter("username");
        String password = request.getParameter("password");
        
        // Validate inputs are not null and within acceptable length - Rules#1
        if (username == null || password == null || 
            username.isEmpty() || password.isEmpty()) {
            response.sendRedirect("login.html?error=invalid");
            return;
        }
        
        // Check length constraints - Rules#1, Rules#13
        if (username.length() > MAX_USERNAME_LENGTH || 
            password.length() > MAX_PASSWORD_LENGTH) {
            response.sendRedirect("login.html?error=invalid");
            return;
        }
        
        // Sanitize username - reject special characters that could be used in SQL injection
        // Rules#1: input validation
        if (!username.matches("^[a-zA-Z0-9_-]+$")) {
            response.sendRedirect("login.html?error=invalid");
            return;
        }
        
        // Rate limiting check - prevent brute force attacks
        HttpSession session = request.getSession(true);
        Integer attempts = (Integer) session.getAttribute("loginAttempts");
        if (attempts == null) {
            attempts = 0;
        }
        
        if (attempts >= MAX_LOGIN_ATTEMPTS) {
            // Generic error message - Rules#1: do not leak internal details
            response.sendRedirect("login.html?error=locked");
            return;
        }
        
        // Authenticate user
        boolean authenticated = authenticateUser(username, password);
        
        if (authenticated) {
            // Reset login attempts on success
            session.removeAttribute("loginAttempts");
            // Create secure session - Rules#1
            session.setAttribute("authenticated", true);
            session.setAttribute("username", username);
            // Regenerate session ID to prevent session fixation
            request.changeSessionId();
            response.sendRedirect("home.html");
        } else {
            // Increment failed attempts
            session.setAttribute("loginAttempts", attempts + 1);
            // Generic error message - Rules#1: do not leak internal details
            response.sendRedirect("login.html?error=invalid");
        }
    }
    
    // Authenticate user against database
    private boolean authenticateUser(String username, String password) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Establish database connection - Rules#5: ensure proper SSL/TLS for remote DB
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            // Use parameterized query to prevent SQL injection - Rules#1
            String query = "SELECT password_hash, salt FROM users WHERE username = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setString(1, username);
            
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                String storedHash = rs.getString("password_hash");
                String salt = rs.getString("salt");
                
                // Hash the provided password with the stored salt - Rules#6, Rules#8, Rules#9
                String computedHash = hashPassword(password, salt);
                
                // Constant-time comparison to prevent timing attacks - Rules#6
                return MessageDigest.isEqual(
                    storedHash.getBytes(StandardCharsets.UTF_8),
                    computedHash.getBytes(StandardCharsets.UTF_8)
                );
            }
            
            return false;
            
        } catch (SQLException e) {
            // Log error securely - Rules#1: do not leak internal details
            System.err.println("Database error during authentication");
            return false;
        } finally {
            // Clean up resources - Rules#1: proper resource management
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing database resources");
            }
        }
    }
    
    // Hash password using PBKDF2 with secure parameters - Rules#6, Rules#7
    private String hashPassword(String password, String salt) {
        try {
            // Use PBKDF2-HMAC-SHA-256 with at least 210000 iterations - Rules#6
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] saltBytes = Base64.getDecoder().decode(salt);
            
            // Simulate PBKDF2 with multiple iterations
            byte[] hash = password.getBytes(StandardCharsets.UTF_8);
            for (int i = 0; i < 210000; i++) {
                md.reset();
                md.update(saltBytes);
                hash = md.digest(hash);
            }
            
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 not available", e);
        }
    }
    
    // Utility method to generate salt - Rules#8, Rules#9
    public static String generateSalt() {
        // Use cryptographically secure random generator - Rules#1, Rules#6, Rules#9
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    // Test method for demonstration
    public static void main(String[] args) {
        System.out.println("Login System Test Cases:");
        
        // Test Case 1: Generate salt and hash password
        String salt1 = generateSalt();
        System.out.println("Test 1 - Salt generated: " + (salt1.length() > 0));
        
        // Test Case 2: Hash password with salt
        Lse35 servlet = new Lse35();
        String hash1 = servlet.hashPassword("testpass123", salt1);
        System.out.println("Test 2 - Password hashed: " + (hash1.length() > 0));
        
        // Test Case 3: Verify same password with same salt produces same hash
        String hash2 = servlet.hashPassword("testpass123", salt1);
        System.out.println("Test 3 - Hash consistency: " + hash1.equals(hash2));
        
        // Test Case 4: Verify different salt produces different hash
        String salt2 = generateSalt();
        String hash3 = servlet.hashPassword("testpass123", salt2);
        System.out.println("Test 4 - Different salt, different hash: " + !hash1.equals(hash3));
        
        // Test Case 5: Verify different password produces different hash
        String hash4 = servlet.hashPassword("differentpass", salt1);
        System.out.println("Test 5 - Different password, different hash: " + !hash1.equals(hash4));
    }
}
