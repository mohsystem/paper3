
import java.util.*;
import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Base64;

public class Lse53 {
    // Secure session management with timeout
    private static final Map<String, SessionData> activeSessions = new HashMap<>();
    private static final long SESSION_TIMEOUT_MS = 30 * 60 * 1000; // 30 minutes
    private static final SecureRandom secureRandom = new SecureRandom();
    
    // User database simulation with hashed passwords
    private static final Map<String, User> userDatabase = new HashMap<>();
    
    static class User {
        String username;
        String passwordHash;
        String salt;
        boolean isAdmin;
        
        User(String username, String passwordHash, String salt, boolean isAdmin) {
            this.username = username;
            this.passwordHash = passwordHash;
            this.salt = salt;
            this.isAdmin = isAdmin;
        }
    }
    
    static class SessionData {
        String username;
        boolean isAdmin;
        long createdTime;
        
        SessionData(String username, boolean isAdmin, long createdTime) {
            this.username = username;
            this.isAdmin = isAdmin;
            this.createdTime = createdTime;
        }
        
        boolean isExpired() {
            return System.currentTimeMillis() - createdTime > SESSION_TIMEOUT_MS;
        }
    }
    
    // Generate cryptographically secure salt
    private static String generateSalt() {
        byte[] salt = new byte[32];
        secureRandom.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    // Hash password with salt using SHA-256
    private static String hashPassword(String password, String salt) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            String saltedPassword = password + salt;
            byte[] hash = digest.digest(saltedPassword.getBytes(StandardCharsets.UTF_8));
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 algorithm not available", e);
        }
    }
    
    // Create user with secure password storage
    private static void createUser(String username, String password, boolean isAdmin) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            throw new IllegalArgumentException("Username and password cannot be empty");
        }
        
        // Generate unique salt for this user
        String salt = generateSalt();
        String passwordHash = hashPassword(password, salt);
        userDatabase.put(username, new User(username, passwordHash, salt, isAdmin));
    }
    
    // Authenticate user with timing-safe comparison
    private static boolean authenticateUser(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        
        User user = userDatabase.get(username);
        if (user == null) {
            // Perform dummy hash to prevent timing attacks
            hashPassword(password, generateSalt());
            return false;
        }
        
        String providedHash = hashPassword(password, user.salt);
        return MessageDigest.isEqual(
            providedHash.getBytes(StandardCharsets.UTF_8),
            user.passwordHash.getBytes(StandardCharsets.UTF_8)
        );
    }
    
    // Create secure session token
    private static String createSession(String username, boolean isAdmin) {
        byte[] tokenBytes = new byte[32];
        secureRandom.nextBytes(tokenBytes);
        String sessionToken = Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
        
        activeSessions.put(sessionToken, new SessionData(username, isAdmin, System.currentTimeMillis()));
        return sessionToken;
    }
    
    // Validate session and return redirect URL
    public static String redirectUser(String sessionToken) {
        // Input validation - check for null or empty token
        if (sessionToken == null || sessionToken.trim().isEmpty()) {
            return "/login";
        }
        
        // Sanitize session token - only allow base64 URL-safe characters
        if (!sessionToken.matches("^[A-Za-z0-9_-]+$")) {
            return "/login";
        }
        
        // Clean expired sessions
        activeSessions.entrySet().removeIf(entry -> entry.getValue().isExpired());
        
        SessionData session = activeSessions.get(sessionToken);
        
        // Not logged in or session expired
        if (session == null || session.isExpired()) {
            return "/login";
        }
        
        // Logged in as admin
        if (session.isAdmin) {
            return "/admin";
        }
        
        // Logged in as regular user
        return "/user";
    }
    
    // Login method with secure authentication
    public static String login(String username, String password) {
        // Input validation
        if (username == null || username.isEmpty() || password == null || password.isEmpty()) {
            return null; // Return null on invalid input
        }
        
        // Sanitize username - only allow alphanumeric and underscore
        if (!username.matches("^[a-zA-Z0-9_]{1,50}$")) {
            return null;
        }
        
        if (authenticateUser(username, password)) {
            User user = userDatabase.get(username);
            return createSession(username, user.isAdmin);
        }
        
        return null;
    }
    
    public static void main(String[] args) {
        // Initialize test users with secure password hashing
        createUser("regularuser", "SecurePass123!", false);
        createUser("adminuser", "AdminPass456!", true);
        
        // Test case 1: No session token (not logged in)
        System.out.println("Test 1 - No session: " + redirectUser(null));
        
        // Test case 2: Invalid session token
        System.out.println("Test 2 - Invalid token: " + redirectUser("invalid_token_123"));
        
        // Test case 3: Regular user logged in
        String regularToken = login("regularuser", "SecurePass123!");
        if (regularToken != null) {
            System.out.println("Test 3 - Regular user: " + redirectUser(regularToken));
        }
        
        // Test case 4: Admin user logged in
        String adminToken = login("adminuser", "AdminPass456!");
        if (adminToken != null) {
            System.out.println("Test 4 - Admin user: " + redirectUser(adminToken));
        }
        
        // Test case 5: Expired session simulation
        String expiredToken = login("regularuser", "SecurePass123!");
        if (expiredToken != null) {
            SessionData session = activeSessions.get(expiredToken);
            session.createdTime = System.currentTimeMillis() - (SESSION_TIMEOUT_MS + 1000);
            System.out.println("Test 5 - Expired session: " + redirectUser(expiredToken));
        }
    }
}
