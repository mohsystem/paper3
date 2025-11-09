
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.sql.DataSource;

/**
 * Secure message posting servlet with input validation and SQL injection prevention.
 */
@WebServlet("/post")
public class Lse150 extends HttpServlet {
    private static final Logger LOGGER = Logger.getLogger(Lse150.class.getName());
    
    // Constants for input validation - Rules#2: enforce length limits
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_MESSAGE_LENGTH = 1000;
    
    // Rules#2: allowed character pattern for username (alphanumeric, underscore, hyphen)
    private static final String USERNAME_PATTERN = "^[a-zA-Z0-9_-]+$";
    
    private DataSource dataSource;
    
    @Override
    public void init() throws ServletException {
        try {
            // Rules#3: Get database connection from JNDI (no hard-coded credentials)
            javax.naming.Context ctx = new javax.naming.InitialContext();
            dataSource = (DataSource) ctx.lookup("java:comp/env/jdbc/MessageDB");
        } catch (Exception e) {
            // Rules#11: Log detailed error internally, throw generic exception
            LOGGER.log(Level.SEVERE, "Database initialization failed", e);
            throw new ServletException("Service initialization failed");
        }
    }
    
    /**
     * Handles POST requests for message submission.
     * Rules#2: Validates all inputs before processing
     * Rules#32: Uses parameterized queries to prevent SQL injection
     */
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        try {
            // Rules#2: Get input parameters and treat as untrusted
            // Use getParameter which already handles character encoding
            String username = request.getParameter("username");
            String message = request.getParameter("message");
            
            // Rules#25: Validate inputs early, reject on failure
            if (!validateUsername(username)) {
                // Rules#11: Return generic error, log details
                LOGGER.log(Level.WARNING, "Invalid username submitted");
                response.sendError(HttpServletResponse.SC_BAD_REQUEST, "Invalid input");
                return;
            }
            
            if (!validateMessage(message)) {
                // Rules#11: Return generic error, log details
                LOGGER.log(Level.WARNING, "Invalid message submitted");
                response.sendError(HttpServletResponse.SC_BAD_REQUEST, "Invalid input");
                return;
            }
            
            // Rules#26: Normalize inputs (trim whitespace)
            username = username.trim();
            message = message.trim();
            
            // Insert message into database using secure method
            insertUserMessageInDb(username, message);
            
            // Rules#1: Redirect to main page after successful submission
            // Use relative path to prevent open redirect
            response.sendRedirect("index.html");
            
        } catch (SQLException e) {
            // Rules#11: Log detailed error, return generic message
            LOGGER.log(Level.SEVERE, "Database error during message insert", e);
            response.sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR, 
                             "Unable to process request");
        } catch (Exception e) {
            // Rules#11: Catch all other exceptions to prevent information leakage
            LOGGER.log(Level.SEVERE, "Unexpected error in post handler", e);
            response.sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR, 
                             "Unable to process request");
        }
    }
    
    /**
     * Validates username input.
     * Rules#2: Check type, length, and format
     * Rules#25: Use allowlist validation
     */
    private boolean validateUsername(String username) {
        // Rules#28: Treat input as untrusted, check null and empty
        if (username == null || username.trim().isEmpty()) {
            return false;
        }
        
        // Rules#2: Enforce maximum length
        if (username.length() > MAX_USERNAME_LENGTH) {
            return false;
        }
        
        // Rules#25: Allowlist - only alphanumeric, underscore, hyphen
        if (!username.matches(USERNAME_PATTERN)) {
            return false;
        }
        
        return true;
    }
    
    /**
     * Validates message input.
     * Rules#2: Check type and length
     */
    private boolean validateMessage(String message) {
        // Rules#28: Treat input as untrusted, check null and empty
        if (message == null || message.trim().isEmpty()) {
            return false;
        }
        
        // Rules#2: Enforce maximum length to prevent excessive memory use
        if (message.length() > MAX_MESSAGE_LENGTH) {
            return false;
        }
        
        return true;
    }
    
    /**
     * Inserts message into database using parameterized query.
     * Rules#32: Use prepared statements to prevent SQL injection
     * Rules#2: All inputs already validated before this method
     */
    private void insertUserMessageInDb(String username, String message) throws SQLException {
        // Rules#32: Use parameterized query - never concatenate user input
        String sql = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, CURRENT_TIMESTAMP)";
        
        // Rules#1: Use try-with-resources for automatic resource cleanup
        try (Connection conn = dataSource.getConnection();
             PreparedStatement stmt = conn.prepareStatement(sql)) {
            
            // Rules#32: Set parameters using PreparedStatement methods
            // This prevents SQL injection by treating inputs as data, not code
            stmt.setString(1, username);
            stmt.setString(2, message);
            
            // Execute the insert
            int rowsAffected = stmt.executeUpdate();
            
            // Verify the operation succeeded
            if (rowsAffected != 1) {
                // Rules#10: Log without exposing sensitive data
                LOGGER.log(Level.WARNING, "Unexpected number of rows affected: " + rowsAffected);
                throw new SQLException("Insert operation failed");
            }
            
        } catch (SQLException e) {
            // Rules#11: Log detailed error for internal use
            LOGGER.log(Level.SEVERE, "Database insert failed", e);
            // Rethrow to be handled by caller
            throw e;
        }
    }
    
    /**
     * Main method with test cases for demonstration.
     * In production, this would be replaced with proper unit tests.
     */
    public static void main(String[] args) {
        Lse150 servlet = new Lse150();
        
        System.out.println("Running validation tests...");
        
        // Test case 1: Valid username and message
        System.out.println("Test 1 - Valid input: " + 
            (servlet.validateUsername("john_doe") && 
             servlet.validateMessage("Hello world!") ? "PASS" : "FAIL"));
        
        // Test case 2: Username too long
        String longUsername = "a".repeat(MAX_USERNAME_LENGTH + 1);
        System.out.println("Test 2 - Username too long: " + 
            (!servlet.validateUsername(longUsername) ? "PASS" : "FAIL"));
        
        // Test case 3: Username with invalid characters
        System.out.println("Test 3 - Invalid username chars: " + 
            (!servlet.validateUsername("john<script>") ? "PASS" : "FAIL"));
        
        // Test case 4: Empty message
        System.out.println("Test 4 - Empty message: " + 
            (!servlet.validateMessage("") ? "PASS" : "FAIL"));
        
        // Test case 5: Message too long
        String longMessage = "a".repeat(MAX_MESSAGE_LENGTH + 1);
        System.out.println("Test 5 - Message too long: " + 
            (!servlet.validateMessage(longMessage) ? "PASS" : "FAIL"));
    }
}
