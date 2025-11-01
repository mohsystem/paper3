
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

public class Lse55 {
    // Secure session store with session tokens
    private static final Map<String, UserSession> sessions = new HashMap<>();
    // User database with hashed passwords and salts
    private static final Map<String, UserData> users = new HashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();
    
    static class UserData {
        String username;
        byte[] passwordHash;
        byte[] salt;
        boolean isAdmin;
        
        UserData(String username, byte[] passwordHash, byte[] salt, boolean isAdmin) {
            this.username = username;
            this.passwordHash = passwordHash;
            this.salt = salt;
            this.isAdmin = isAdmin;
        }
    }
    
    static class UserSession {
        String username;
        boolean isAdmin;
        long expiryTime;
        
        UserSession(String username, boolean isAdmin) {
            this.username = username;
            this.isAdmin = isAdmin;
            // Session expires in 30 minutes
            this.expiryTime = System.currentTimeMillis() + (30 * 60 * 1000);
        }
        
        boolean isValid() {
            return System.currentTimeMillis() < expiryTime;
        }
    }
    
    // Initialize with test users using secure password hashing
    static {
        try {
            // Create test users with unique salts per CWE-759 mitigation
            createUser("user1", "Password123!", false);
            createUser("admin1", "AdminPass456!", true);
            createUser("user2", "SecurePass789!", false);
            createUser("admin2", "AdminSecure000!", true);
            createUser("testuser", "TestPass111!", false);
        } catch (Exception e) {
            System.err.println("Error initializing users: " + e.getMessage());
        }
    }
    
    // Create user with PBKDF2-HMAC-SHA-256 hashing and unique salt per CWE-759
    private static void createUser(String username, String password, boolean isAdmin) 
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        // Generate unique cryptographically secure salt per CWE-759
        byte[] salt = new byte[16];
        secureRandom.nextBytes(salt);
        
        // Use PBKDF2-HMAC-SHA-256 with 210000 iterations per security rules
        byte[] hash = hashPassword(password, salt);
        users.put(username, new UserData(username, hash, salt, isAdmin));
    }
    
    // Secure password hashing with PBKDF2-HMAC-SHA-256
    private static byte[] hashPassword(String password, byte[] salt) 
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        // Use PBKDF2WithHmacSHA256 with 210000 iterations per security policy
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, 210000, 256);
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        return factory.generateSecret(spec).getEncoded();
    }
    
    // Constant-time comparison to prevent timing attacks
    private static boolean constantTimeCompare(byte[] a, byte[] b) {
        if (a.length != b.length) {
            return false;
        }
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    // GET /login - Display login form
    public static String login(Map<String, String> params) {
        // Return simple login form HTML
        return "<html><body>" +
               "<h2>Login</h2>" +
               "<form method='POST' action='/do_login'>" +
               "Username: <input type='text' name='username' required><br>" +
               "Password: <input type='password' name='password' required><br>" +
               "<input type='submit' value='Login'>" +
               "</form></body></html>";
    }
    
    // POST /do_login - Process login credentials
    public static Map<String, Object> doLogin(Map<String, String> params) {
        Map<String, Object> result = new HashMap<>();
        
        // Input validation: check required parameters
        String username = params.get("username");
        String password = params.get("password");
        
        if (username == null || username.trim().isEmpty() || 
            password == null || password.isEmpty()) {
            result.put("success", false);
            result.put("message", "Invalid credentials");
            return result;
        }
        
        // Sanitize username: limit length and validate format
        username = username.trim();
        if (username.length() > 50 || !username.matches("^[a-zA-Z0-9_]+$")) {
            result.put("success", false);
            result.put("message", "Invalid credentials");
            return result;
        }
        
        // Validate password length
        if (password.length() > 128) {
            result.put("success", false);
            result.put("message", "Invalid credentials");
            return result;
        }
        
        try {
            // Lookup user
            UserData userData = users.get(username);
            if (userData == null) {
                // Generic error message to prevent user enumeration
                result.put("success", false);
                result.put("message", "Invalid credentials");
                return result;
            }
            
            // Verify password using constant-time comparison
            byte[] providedHash = hashPassword(password, userData.salt);
            if (!constantTimeCompare(providedHash, userData.passwordHash)) {
                result.put("success", false);
                result.put("message", "Invalid credentials");
                return result;
            }
            
            // Generate secure session token using CSPRNG
            String sessionToken = UUID.randomUUID().toString();
            UserSession session = new UserSession(userData.username, userData.isAdmin);
            sessions.put(sessionToken, session);
            
            result.put("success", true);
            result.put("sessionToken", sessionToken);
            result.put("message", "Login successful");
            
        } catch (Exception e) {
            // Generic error message, log details securely
            System.err.println("Login error: " + e.getClass().getName());
            result.put("success", false);
            result.put("message", "Authentication error");
        }
        
        return result;
    }
    
    // GET /index - Public home page
    public static String index(Map<String, String> params) {
        return "<html><body>" +
               "<h1>Welcome to the Application</h1>" +
               "<p>This is a public page.</p>" +
               "<a href='/login'>Login</a>" +
               "</body></html>";
    }
    
    // GET /user_page - Authenticated user page
    public static Map<String, Object> userPage(Map<String, String> params) {
        Map<String, Object> result = new HashMap<>();
        
        // Validate session token
        String sessionToken = params.get("sessionToken");
        if (sessionToken == null || sessionToken.trim().isEmpty()) {
            result.put("authorized", false);
            result.put("message", "Authentication required");
            return result;
        }
        
        // Check session validity
        UserSession session = sessions.get(sessionToken);
        if (session == null || !session.isValid()) {
            // Clean up expired session
            if (session != null) {
                sessions.remove(sessionToken);
            }
            result.put("authorized", false);
            result.put("message", "Session expired or invalid");
            return result;
        }
        
        result.put("authorized", true);
        result.put("username", session.username);
        result.put("content", "Welcome to your user page, " + session.username + "!");
        return result;
    }
    
    // GET /admin_page - Admin-only page
    public static Map<String, Object> adminPage(Map<String, String> params) {
        Map<String, Object> result = new HashMap<>();
        
        // Validate session token
        String sessionToken = params.get("sessionToken");
        if (sessionToken == null || sessionToken.trim().isEmpty()) {
            result.put("authorized", false);
            result.put("message", "Authentication required");
            return result;
        }
        
        // Check session validity and admin role
        UserSession session = sessions.get(sessionToken);
        if (session == null || !session.isValid()) {
            // Clean up expired session
            if (session != null) {
                sessions.remove(sessionToken);
            }
            result.put("authorized", false);
            result.put("message", "Session expired or invalid");
            return result;
        }
        
        // Check admin privilege
        if (!session.isAdmin) {
            result.put("authorized", false);
            result.put("message", "Insufficient privileges");
            return result;
        }
        
        result.put("authorized", true);
        result.put("username", session.username);
        result.put("content", "Welcome to the admin page, " + session.username + "!");
        return result;
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Login page (GET) ===");
        String loginPage = login(new HashMap<>());
        System.out.println(loginPage.substring(0, Math.min(100, loginPage.length())) + "...");
        
        System.out.println("\\n=== Test Case 2: Valid login (POST) ===");
        Map<String, String> loginParams = new HashMap<>();
        loginParams.put("username", "user1");
        loginParams.put("password", "Password123!");
        Map<String, Object> loginResult = doLogin(loginParams);
        System.out.println("Success: " + loginResult.get("success"));
        String sessionToken = (String) loginResult.get("sessionToken");
        System.out.println("Session token: " + (sessionToken != null ? "Generated" : "None"));
        
        System.out.println("\\n=== Test Case 3: Invalid login ===");
        Map<String, String> invalidParams = new HashMap<>();
        invalidParams.put("username", "user1");
        invalidParams.put("password", "WrongPassword!");
        Map<String, Object> invalidResult = doLogin(invalidParams);
        System.out.println("Success: " + invalidResult.get("success"));
        System.out.println("Message: " + invalidResult.get("message"));
        
        System.out.println("\\n=== Test Case 4: User page access ===");
        if (sessionToken != null) {
            Map<String, String> userParams = new HashMap<>();
            userParams.put("sessionToken", sessionToken);
            Map<String, Object> userResult = userPage(userParams);
            System.out.println("Authorized: " + userResult.get("authorized"));
            System.out.println("Content: " + userResult.get("content"));
        }
        
        System.out.println("\\n=== Test Case 5: Admin page access (user) ===");
        if (sessionToken != null) {
            Map<String, String> adminParams = new HashMap<>();
            adminParams.put("sessionToken", sessionToken);
            Map<String, Object> adminResult = adminPage(adminParams);
            System.out.println("Authorized: " + adminResult.get("authorized"));
            System.out.println("Message: " + adminResult.get("message"));
        }
        
        System.out.println("\\n=== Bonus: Admin login and access ===");
        Map<String, String> adminLoginParams = new HashMap<>();
        adminLoginParams.put("username", "admin1");
        adminLoginParams.put("password", "AdminPass456!");
        Map<String, Object> adminLoginResult = doLogin(adminLoginParams);
        String adminToken = (String) adminLoginResult.get("sessionToken");
        if (adminToken != null) {
            Map<String, String> adminAccessParams = new HashMap<>();
            adminAccessParams.put("sessionToken", adminToken);
            Map<String, Object> adminAccessResult = adminPage(adminAccessParams);
            System.out.println("Admin authorized: " + adminAccessResult.get("authorized"));
            System.out.println("Admin content: " + adminAccessResult.get("content"));
        }
    }
}
