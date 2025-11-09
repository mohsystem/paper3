
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.*;
import javax.servlet.*;
import javax.servlet.http.*;
import javax.servlet.annotation.*;

@WebServlet(name = "Lse52", urlPatterns = {"/", "/login", "/do_login", "/secret", "/logout"})
public class Lse52 extends HttpServlet {
    
    // Secure password storage simulation - In production, use a proper database with hashed passwords
    // and a secrets manager for credentials
    private static final Map<String, String> USER_DATABASE = new HashMap<>();
    
    static {
        // Initialize with a test user - password is hashed using SHA-256
        // In production: use Argon2id or scrypt with proper salt and iterations
        // Password for "testuser" is "SecurePass123!" (hashed)
        try {
            String salt = "fixed_salt_for_demo"; // In production: generate unique salt per user
            USER_DATABASE.put("testuser", hashPassword("SecurePass123!", salt));
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Failed to initialize user database", e);
        }
    }
    
    // Hash password using SHA-256 with salt
    // In production: use Argon2id with proper parameters
    private static String hashPassword(String password, String salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt.getBytes(StandardCharsets.UTF_8));
        byte[] hash = md.digest(password.getBytes(StandardCharsets.UTF_8));
        StringBuilder hexString = new StringBuilder();
        for (byte b : hash) {
            String hex = Integer.toHexString(0xff & b);
            if (hex.length() == 1) hexString.append('0');
            hexString.append(hex);
        }
        return hexString.toString();
    }
    
    @Override
    protected void doGet(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        
        // Set secure headers for all responses
        response.setHeader("X-Content-Type-Options", "nosniff");
        response.setHeader("X-Frame-Options", "DENY");
        response.setHeader("Content-Security-Policy", "default-src 'self'");
        response.setCharacterEncoding("UTF-8");
        response.setContentType("text/html;charset=UTF-8");
        
        String path = request.getServletPath();
        
        // Route requests to appropriate handlers
        switch (path) {
            case "/":
                showIndex(request, response);
                break;
            case "/login":
                showLogin(request, response);
                break;
            case "/secret":
                showSecret(request, response);
                break;
            case "/logout":
                doLogout(request, response);
                break;
            default:
                response.sendError(HttpServletResponse.SC_NOT_FOUND);
        }
    }
    
    @Override
    protected void doPost(HttpServletRequest request, HttpServletResponse response)
            throws ServletException, IOException {
        
        // Set secure headers
        response.setHeader("X-Content-Type-Options", "nosniff");
        response.setHeader("X-Frame-Options", "DENY");
        response.setHeader("Content-Security-Policy", "default-src 'self'");
        response.setCharacterEncoding("UTF-8");
        
        String path = request.getServletPath();
        
        if ("/do_login".equals(path)) {
            doLogin(request, response);
        } else {
            response.sendError(HttpServletResponse.SC_METHOD_NOT_ALLOWED);
        }
    }
    
    // Display index page with link to login
    private void showIndex(HttpServletRequest request, HttpServletResponse response)
            throws IOException {
        PrintWriter out = response.getWriter();
        out.println("<!DOCTYPE html>");
        out.println("<html>");
        out.println("<head>");
        out.println("<meta charset=\\"UTF-8\\">");
        out.println("<title>Home</title>");
        out.println("</head>");
        out.println("<body>");
        out.println("<h1>Welcome to the Home Page</h1>");
        
        // Check if user is already logged in
        HttpSession session = request.getSession(false);
        if (session != null && session.getAttribute("username") != null) {
            // Sanitize output to prevent XSS
            String username = sanitizeOutput((String) session.getAttribute("username"));
            out.println("<p>You are logged in as: " + username + "</p>");
            out.println("<p><a href=\\"/secret\\">Go to Secret Page</a></p>");
            out.println("<p><a href=\\"/logout\\">Logout</a></p>");
        } else {
            out.println("<p><a href=\\"/login\\">Login</a></p>");
        }
        
        out.println("</body>");
        out.println("</html>");
    }
    
    // Display login form
    private void showLogin(HttpServletRequest request, HttpServletResponse response)
            throws IOException {
        PrintWriter out = response.getWriter();
        out.println("<!DOCTYPE html>");
        out.println("<html>");
        out.println("<head>");
        out.println("<meta charset=\\"UTF-8\\">");
        out.println("<title>Login</title>");
        out.println("</head>");
        out.println("<body>");
        out.println("<h1>Login</h1>");
        
        // Display error message if present (sanitized)
        String error = request.getParameter("error");
        if (error != null) {
            out.println("<p style=\\"color: red;\\">Invalid credentials</p>");
        }
        
        out.println("<form method=\\"post\\" action=\\"/do_login\\">");
        out.println("<div>");
        out.println("<label for=\\"username\\">Username:</label>");
        out.println("<input type=\\"text\\" id=\\"username\\" name=\\"username\\" required maxlength=\\"50\\">");
        out.println("</div>");
        out.println("<div>");
        out.println("<label for=\\"password\\">Password:</label>");
        out.println("<input type=\\"password\\" id=\\"password\\" name=\\"password\\" required maxlength=\\"100\\">");
        out.println("</div>");
        out.println("<div>");
        out.println("<button type=\\"submit\\">Login</button>");
        out.println("</div>");
        out.println("</form>");
        out.println("<p><a href=\\"/\\">Back to Home</a></p>");
        out.println("</body>");
        out.println("</html>");
    }
    
    // Process login form submission
    private void doLogin(HttpServletRequest request, HttpServletResponse response)
            throws IOException {
        
        // Input validation: check parameters exist and are not empty
        String username = request.getParameter("username");
        String password = request.getParameter("password");
        
        // Validate input is not null and within acceptable length
        if (username == null || password == null || 
            username.trim().isEmpty() || password.trim().isEmpty() ||
            username.length() > 50 || password.length() > 100) {
            response.sendRedirect("/login?error=1");
            return;
        }
        
        // Sanitize username - allow only alphanumeric and underscore
        username = username.trim();
        if (!username.matches("^[a-zA-Z0-9_]+$")) {
            response.sendRedirect("/login?error=1");
            return;
        }
        
        try {
            // Hash the provided password with the same salt used in storage
            String hashedPassword = hashPassword(password, "fixed_salt_for_demo");
            
            // Verify credentials
            if (USER_DATABASE.containsKey(username) && 
                USER_DATABASE.get(username).equals(hashedPassword)) {
                
                // Invalidate any existing session to prevent session fixation
                HttpSession oldSession = request.getSession(false);
                if (oldSession != null) {
                    oldSession.invalidate();
                }
                
                // Create new session with secure settings
                HttpSession session = request.getSession(true);
                session.setAttribute("username", username);
                
                // Set session timeout (30 minutes)
                session.setMaxInactiveInterval(1800);
                
                // Configure secure cookie settings
                Cookie sessionCookie = new Cookie("JSESSIONID", session.getId());
                sessionCookie.setHttpOnly(true); // Prevent XSS access to cookie
                sessionCookie.setSecure(true);   // Only send over HTTPS
                sessionCookie.setPath("/");
                sessionCookie.setMaxAge(-1);     // Session cookie
                response.addCookie(sessionCookie);
                
                // Redirect to secret page after successful login
                response.sendRedirect("/secret");
            } else {
                // Invalid credentials - redirect back to login with error
                response.sendRedirect("/login?error=1");
            }
        } catch (NoSuchAlgorithmException e) {
            // Log error securely without exposing details to user
            System.err.println("Authentication error occurred");
            response.sendRedirect("/login?error=1");
        }
    }
    
    // Display secret page (requires authentication)
    private void showSecret(HttpServletRequest request, HttpServletResponse response)
            throws IOException {
        
        // Check if user is authenticated
        HttpSession session = request.getSession(false);
        if (session == null || session.getAttribute("username") == null) {
            // Not authenticated - redirect to login
            response.sendRedirect("/login");
            return;
        }
        
        // User is authenticated - show secret page
        PrintWriter out = response.getWriter();
        out.println("<!DOCTYPE html>");
        out.println("<html>");
        out.println("<head>");
        out.println("<meta charset=\\"UTF-8\\">");
        out.println("<title>Secret Settings</title>");
        out.println("</head>");
        out.println("<body>");
        out.println("<h1>Secret settings</h1>");
        
        // Sanitize username before displaying
        String username = sanitizeOutput((String) session.getAttribute("username"));
        out.println("<p>Welcome, " + username + "!</p>");
        
        out.println("<p><a href=\\"/\\">Back to Home</a></p>");
        out.println("<p><a href=\\"/logout\\">Logout</a></p>");
        out.println("</body>");
        out.println("</html>");
    }
    
    // Handle logout
    private void doLogout(HttpServletRequest request, HttpServletResponse response)
            throws IOException {
        
        // Invalidate session
        HttpSession session = request.getSession(false);
        if (session != null) {
            session.invalidate();
        }
        
        // Redirect to home page
        response.sendRedirect("/");
    }
    
    // Sanitize output to prevent XSS attacks
    private String sanitizeOutput(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\\"", "&quot;")
                    .replace("'", "&#x27;");
    }
    
    // Main method with test cases
    public static void main(String[] args) {
        System.out.println("=== Lse52 Security Test Cases ===\\n");
        
        // Test Case 1: Valid password hashing
        try {
            String hash1 = hashPassword("SecurePass123!", "fixed_salt_for_demo");
            String hash2 = hashPassword("SecurePass123!", "fixed_salt_for_demo");
            System.out.println("Test 1 - Password hashing consistency: " + hash1.equals(hash2));
        } catch (NoSuchAlgorithmException e) {
            System.out.println("Test 1 - Failed: " + e.getMessage());
        }
        
        // Test Case 2: Different passwords produce different hashes
        try {
            String hash1 = hashPassword("Password1", "salt");
            String hash2 = hashPassword("Password2", "salt");
            System.out.println("Test 2 - Different passwords: " + !hash1.equals(hash2));
        } catch (NoSuchAlgorithmException e) {
            System.out.println("Test 2 - Failed: " + e.getMessage());
        }
        
        // Test Case 3: XSS sanitization
        Lse52 servlet = new Lse52();
        String xssInput = "<script>alert('xss')</script>";
        String sanitized = servlet.sanitizeOutput(xssInput);
        System.out.println("Test 3 - XSS sanitization: " + !sanitized.contains("<script>"));
        
        // Test Case 4: Username validation pattern
        String validUsername = "testuser123";
        String invalidUsername = "test<script>";
        System.out.println("Test 4 - Username validation: " + 
            validUsername.matches("^[a-zA-Z0-9_]+$") && 
            !invalidUsername.matches("^[a-zA-Z0-9_]+$"));
        
        // Test Case 5: User database lookup
        boolean userExists = USER_DATABASE.containsKey("testuser");
        boolean fakeUserExists = USER_DATABASE.containsKey("fakeuser");
        System.out.println("Test 5 - User database: " + (userExists && !fakeUserExists));
        
        System.out.println("\\nAll security tests completed.");
    }
}
