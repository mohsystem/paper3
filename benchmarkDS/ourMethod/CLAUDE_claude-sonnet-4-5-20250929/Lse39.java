
import java.io.Console;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse39 {
    // Secure storage for user credentials (hashed passwords with unique salts)
    // In production, use a proper database with encrypted storage
    private static final Map<String, UserCredential> userDatabase = new HashMap<>();
    private static final int MAX_LOGIN_ATTEMPTS = 3;
    private static final int LOCKOUT_TIME_MS = 300000; // 5 minutes
    
    static class UserCredential {
        byte[] passwordHash;
        byte[] salt;
        int failedAttempts;
        long lockoutUntil;
        
        UserCredential(byte[] passwordHash, byte[] salt) {
            this.passwordHash = passwordHash;
            this.salt = salt;
            this.failedAttempts = 0;
            this.lockoutUntil = 0;
        }
    }
    
    // Initialize with test users (passwords hashed with unique salts)
    static {
        try {
            addUser("admin", "SecurePass123!");
            addUser("user1", "MyPassword456@");
            addUser("testuser", "Test789#Pass");
        } catch (NoSuchAlgorithmException e) {
            System.err.println("Cryptographic algorithm not available");
        }
    }
    
    // Hash password with unique salt using SHA-256 and iterations
    private static byte[] hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt);
        byte[] hash = md.digest(password.getBytes(StandardCharsets.UTF_8));
        
        // Multiple iterations to slow down brute force attacks
        for (int i = 0; i < 10000; i++) {
            md.reset();
            hash = md.digest(hash);
        }
        
        return hash;
    }
    
    // Generate cryptographically secure random salt
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16]; // 128-bit salt
        random.nextBytes(salt);
        return salt;
    }
    
    // Add user to database with hashed password
    private static void addUser(String username, String password) throws NoSuchAlgorithmException {
        // Input validation - reject null or empty values
        if (username == null || username.trim().isEmpty() || 
            password == null || password.isEmpty()) {
            throw new IllegalArgumentException("Username and password cannot be empty");
        }
        
        // Validate username format (alphanumeric only, max 50 chars)
        if (!username.matches("^[a-zA-Z0-9]{1,50}$")) {
            throw new IllegalArgumentException("Invalid username format");
        }
        
        byte[] salt = generateSalt();
        byte[] hash = hashPassword(password, salt);
        userDatabase.put(username, new UserCredential(hash, salt));
    }
    
    // Verify password using constant-time comparison to prevent timing attacks
    private static boolean verifyPassword(String username, String password) {
        try {
            // Input validation
            if (username == null || username.trim().isEmpty() || 
                password == null || password.isEmpty()) {
                return false;
            }
            
            UserCredential cred = userDatabase.get(username);
            if (cred == null) {
                // Perform dummy hash to prevent timing attacks
                byte[] dummySalt = new byte[16];
                hashPassword(password, dummySalt);
                return false;
            }
            
            // Check if account is locked
            if (cred.lockoutUntil > System.currentTimeMillis()) {
                return false;
            }
            
            byte[] providedHash = hashPassword(password, cred.salt);
            
            // Constant-time comparison to prevent timing attacks
            boolean matches = MessageDigest.isEqual(providedHash, cred.passwordHash);
            
            if (matches) {
                // Reset failed attempts on success
                cred.failedAttempts = 0;
                cred.lockoutUntil = 0;
            } else {
                // Increment failed attempts
                cred.failedAttempts++;
                if (cred.failedAttempts >= MAX_LOGIN_ATTEMPTS) {
                    cred.lockoutUntil = System.currentTimeMillis() + LOCKOUT_TIME_MS;
                }
            }
            
            return matches;
            
        } catch (NoSuchAlgorithmException e) {
            // Log error securely without exposing details
            System.err.println("Authentication error occurred");
            return false;
        }
    }
    
    // Simulate session page redirect
    private static void redirectToSessionPage(String username) {
        System.out.println("Login successful! Redirecting to session page...");
        System.out.println("Welcome, " + username + "!");
        // In a real application, create secure session token here
        // and set appropriate session cookies with HttpOnly, Secure, SameSite flags
    }
    
    // Main login function
    public static String login(String username, String password) {
        // Input validation - check for null/empty
        if (username == null || username.trim().isEmpty()) {
            return "Invalid username or password";
        }
        
        if (password == null || password.isEmpty()) {
            return "Invalid username or password";
        }
        
        // Sanitize username input - allow only alphanumeric
        String sanitizedUsername = username.replaceAll("[^a-zA-Z0-9]", "");
        if (sanitizedUsername.isEmpty() || !sanitizedUsername.equals(username)) {
            return "Invalid username or password";
        }
        
        // Length validation
        if (username.length() > 50 || password.length() > 128) {
            return "Invalid username or password";
        }
        
        // Check if account is locked
        UserCredential cred = userDatabase.get(sanitizedUsername);
        if (cred != null && cred.lockoutUntil > System.currentTimeMillis()) {
            return "Account temporarily locked due to multiple failed attempts";
        }
        
        // Verify credentials
        if (verifyPassword(sanitizedUsername, password)) {
            redirectToSessionPage(sanitizedUsername);
            return "SUCCESS";
        } else {
            // Generic error message to prevent username enumeration
            return "Invalid username or password";
        }
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Login System Test Cases ===\\n");
        
        // Test case 1: Valid login
        System.out.println("Test 1: Valid credentials (admin/SecurePass123!)");
        String result1 = login("admin", "SecurePass123!");
        System.out.println("Result: " + result1);
        System.out.println();
        
        // Test case 2: Invalid password
        System.out.println("Test 2: Invalid password");
        String result2 = login("admin", "WrongPassword");
        System.out.println("Result: " + result2);
        System.out.println();
        
        // Test case 3: Non-existent user
        System.out.println("Test 3: Non-existent user");
        String result3 = login("nonexistent", "password");
        System.out.println("Result: " + result3);
        System.out.println();
        
        // Test case 4: Empty credentials
        System.out.println("Test 4: Empty username");
        String result4 = login("", "password");
        System.out.println("Result: " + result4);
        System.out.println();
        
        // Test case 5: Valid login for another user
        System.out.println("Test 5: Valid credentials (user1/MyPassword456@)");
        String result5 = login("user1", "MyPassword456@");
        System.out.println("Result: " + result5);
        System.out.println();
    }
}
