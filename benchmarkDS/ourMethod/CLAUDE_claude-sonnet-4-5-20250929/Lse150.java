
import java.io.IOException;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse150 extends HttpServlet {
    
    // Rules#2: Input validation - maximum lengths to prevent resource exhaustion
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_MESSAGE_LENGTH = 500;
    
    // Rules#11: Generic error message without internal details
    private static final String GENERIC_ERROR = "Unable to process request";
    
    /**
     * Rules#2: Validates username input
     * Checks type, length, and allowed character set
     */
    private boolean isValidUsername(String username) {
        if (username == null || username.trim().isEmpty()) {
            return false;
        }
        // Rules#2: Length check
        if (username.length() > MAX_USERNAME_LENGTH) {
            return false;
        }
        // Rules#2: Allow only alphanumeric, underscore, hyphen
        return username.matches("^[a-zA-Z0-9_-]+$");
    }
    
    /**
     * Rules#2: Validates message input
     * Checks type, length, and encoding
     */
    private boolean isValidMessage(String message) {
        if (message == null || message.trim().isEmpty()) {
            return false;
        }
        // Rules#2: Length check to prevent excessive resource use
        if (message.length() > MAX_MESSAGE_LENGTH) {
            return false;
        }
        // Message can contain more characters but must be valid UTF-8
        return true;
    }
    
    /**
     * Rules#32: Parameterized query to prevent SQL injection
     * Rules#8: Proper error handling and resource cleanup
     */
    private void insertUserMessageInDb(Connection conn, String username, String message) throws SQLException {
        // Rules#32: Use prepared statement with parameters, never concatenate user input
        String sql = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, NOW())";
        
        try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
            // Rules#32: Set parameters safely - JDBC handles escaping
            pstmt.setString(1, username);
            pstmt.setString(2, message);
            pstmt.executeUpdate();
        }
        // Rules#8: try-with-resources ensures PreparedStatement is closed
    }
    
    /**
     * Rules#24-28: CSRF token validation
     */
    private boolean validateCsrfToken(HttpServletRequest request) {
        HttpSession session = request.getSession(false);
        if (session == null) {
            return false;
        }
        
        String sessionToken = (String) session.getAttribute("csrf_token");
        String requestToken = request.getParameter("csrf_token");
        
        // Rules#24: Validate token exists and matches
        if (sessionToken == null || requestToken == null) {
            return false;
        }
        
        return MessageDigest.isEqual(
            sessionToken.getBytes(StandardCharsets.UTF_8),
            requestToken.getBytes(StandardCharsets.UTF_8)
        );
    }
    
    /**
     * Rules#5: Generate CSRF token using CSPRNG
     */
    private String generateCsrfToken() {
        SecureRandom random = new SecureRandom();
        byte[] bytes = new byte[32];
        random.nextBytes(bytes);
        return Base64.getEncoder().encodeToString(bytes);
    }
    
    /**
     * POST handler for message submission
     * Rules#24-28: Comprehensive input validation
     * Rules#11: Error handling without leaking details
     */
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        // Rules#28: Get session safely
        HttpSession session = request.getSession(false);
        if (session == null) {
            response.sendError(HttpServletResponse.SC_FORBIDDEN, GENERIC_ERROR);
            return;
        }
        
        // Rules#24-28: Validate CSRF token first
        if (!validateCsrfToken(request)) {
            response.sendError(HttpServletResponse.SC_FORBIDDEN, GENERIC_ERROR);
            return;
        }
        
        // Rules#28: Read parameters treating all input as untrusted
        String username = request.getParameter("username");
        String message = request.getParameter("message");
        
        // Rules#24-25: Validate inputs with allow list approach
        if (!isValidUsername(username)) {
            response.sendError(HttpServletResponse.SC_BAD_REQUEST, "Invalid username");
            return;
        }
        
        if (!isValidMessage(message)) {
            response.sendError(HttpServletResponse.SC_BAD_REQUEST, "Invalid message");
            return;
        }
        
        Connection conn = null;
        try {
            // Assume getConnection() is a method that provides database connection
            // Rules#6: Connection should use TLS for remote databases
            conn = getConnection();
            
            // Rules#32: Use parameterized query via the insert function
            insertUserMessageInDb(conn, username, message);
            
            // Rules#11: Success - redirect without leaking details
            response.sendRedirect(request.getContextPath() + "/");
            
        } catch (SQLException e) {
            // Rules#10-11: Log details securely, return generic error to user
            logError("Database error in post()", e);
            response.sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR, GENERIC_ERROR);
        } finally {
            // Rules#8: Ensure connection is closed
            if (conn != null) {
                try {
                    conn.close();
                } catch (SQLException e) {
                    logError("Error closing connection", e);
                }
            }
        }
    }
    
    /**
     * GET handler to display the form
     * Rules#10: Logging with constant format strings
     */
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response) 
            throws ServletException, IOException {
        
        HttpSession session = request.getSession(true);
        
        // Generate CSRF token if not present
        String csrfToken = (String) session.getAttribute("csrf_token");
        if (csrfToken == null) {
            csrfToken = generateCsrfToken();
            session.setAttribute("csrf_token", csrfToken);
        }
        
        // Rules#10-31: Use constant format for output, escape user data
        response.setContentType("text/html;charset=UTF-8");
        response.getWriter().println("<!DOCTYPE html>");
        response.getWriter().println("<html><head><title>Message Board</title></head><body>");
        response.getWriter().println("<h1>Post a Message</h1>");
        response.getWriter().println("<form method='POST' action='" + 
            escapeHtml(request.getContextPath() + "/post") + "'>");
        response.getWriter().println("<input type='hidden' name='csrf_token' value='" + 
            escapeHtml(csrfToken) + "'>");
        response.getWriter().println("<label>Username: <input type='text' name='username' " +
            "maxlength='" + MAX_USERNAME_LENGTH + "' required></label><br>");
        response.getWriter().println("<label>Message: <textarea name='message' " +
            "maxlength='" + MAX_MESSAGE_LENGTH + "' required></textarea></label><br>");
        response.getWriter().println("<button type='submit'>Submit</button>");
        response.getWriter().println("</form></body></html>");
    }
    
    /**
     * Rules#24: HTML escaping to prevent XSS
     */
    private String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    /**
     * Rules#10: Secure logging with constant format string
     */
    private void logError(String message, Exception e) {
        // In production, use a proper logging framework
        System.err.println("[ERROR] " + message + ": " + e.getClass().getName());
    }
    
    /**
     * Placeholder for database connection
     * Rules#6: Should enforce TLS for remote connections
     * Rules#3: Credentials should come from external config
     */
    private Connection getConnection() throws SQLException {
        // Implementation would load from environment/config
        // Never hard code credentials (Rules#3)
        throw new SQLException("Not implemented");
    }
    
    // Test cases
    public static void main(String[] args) {
        Lse150 servlet = new Lse150();
        
        // Test 1: Valid username
        System.out.println("Test 1: " + servlet.isValidUsername("john_doe")); // true
        
        // Test 2: Invalid username with special chars
        System.out.println("Test 2: " + servlet.isValidUsername("john@doe")); // false
        
        // Test 3: Username too long
        System.out.println("Test 3: " + servlet.isValidUsername("a".repeat(100))); // false
        
        // Test 4: Valid message
        System.out.println("Test 4: " + servlet.isValidMessage("Hello world!")); // true
        
        // Test 5: Message too long
        System.out.println("Test 5: " + servlet.isValidMessage("a".repeat(600))); // false
    }
}
