
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse56 {
    // Simulated user database - in production, use a proper database with encrypted credentials
    private static class User {
        String email;
        byte[] passwordHash;
        byte[] salt;
        boolean loggedIn;
        
        User(String email, String password) {
            this.email = email;
            this.salt = generateSalt();
            this.passwordHash = hashPassword(password, this.salt);
            this.loggedIn = false;
        }
    }
    
    private static final Map<String, User> users = new HashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();
    
    // Generate cryptographically secure random salt
    private static byte[] generateSalt() {
        byte[] salt = new byte[16];
        secureRandom.nextBytes(salt);
        return salt;
    }
    
    // Hash password using PBKDF2-like approach with SHA-256
    private static byte[] hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            // Multiple iterations to strengthen the hash
            byte[] hash = md.digest(password.getBytes(StandardCharsets.UTF_8));
            for (int i = 0; i < 10000; i++) {
                md.reset();
                hash = md.digest(hash);
            }
            return hash;
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 not available", e);
        }
    }
    
    // Constant-time comparison to prevent timing attacks
    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a.length != b.length) {
            return false;
        }
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    // Validate email format
    private static boolean isValidEmail(String email) {
        if (email == null || email.length() == 0 || email.length() > 254) {
            return false;
        }
        // Basic email validation - alphanumeric, @, dot
        return email.matches("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");
    }
    
    // Sanitize input to prevent injection attacks
    private static String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        // Remove any control characters and limit length
        return input.replaceAll("[\\\\x00-\\\\x1F\\\\x7F]", "").substring(0, Math.min(input.length(), 254));
    }
    
    // Register a new user
    public static String registerUser(String email, String password) {
        // Input validation
        email = sanitizeInput(email);
        if (!isValidEmail(email)) {
            return "Error: Invalid email format";
        }
        
        if (password == null || password.length() < 8) {
            return "Error: Password must be at least 8 characters";
        }
        
        if (users.containsKey(email)) {
            return "Error: User already exists";
        }
        
        users.put(email, new User(email, password));
        return "Success: User registered";
    }
    
    // Login user
    public static String login(String email, String password) {
        // Input validation and sanitization
        email = sanitizeInput(email);
        if (!isValidEmail(email)) {
            return "Error: Invalid credentials";
        }
        
        if (password == null || password.isEmpty()) {
            return "Error: Invalid credentials";
        }
        
        User user = users.get(email);
        if (user == null) {
            return "Error: Invalid credentials";
        }
        
        // Verify password using constant-time comparison
        byte[] providedHash = hashPassword(password, user.salt);
        if (!constantTimeEquals(user.passwordHash, providedHash)) {
            return "Error: Invalid credentials";
        }
        
        user.loggedIn = true;
        return "Success: Logged in";
    }
    
    // Change email - requires authentication
    public static String changeEmail(String oldEmail, String newEmail, String password) {
        // Input validation and sanitization
        oldEmail = sanitizeInput(oldEmail);
        newEmail = sanitizeInput(newEmail);
        
        if (!isValidEmail(oldEmail) || !isValidEmail(newEmail)) {
            return "Error: Invalid email format";
        }
        
        if (password == null || password.isEmpty()) {
            return "Error: Password required";
        }
        
        // Check if user exists
        User user = users.get(oldEmail);
        if (user == null) {
            return "Error: User not found";
        }
        
        // Check if user is logged in
        if (!user.loggedIn) {
            return "Error: User must be logged in";
        }
        
        // Verify password using constant-time comparison
        byte[] providedHash = hashPassword(password, user.salt);
        if (!constantTimeEquals(user.passwordHash, providedHash)) {
            return "Error: Incorrect password";
        }
        
        // Check if new email is different from old email
        if (oldEmail.equals(newEmail)) {
            return "Error: New email must be different from old email";
        }
        
        // Check if new email already exists
        if (users.containsKey(newEmail)) {
            return "Error: New email already in use";
        }
        
        // Update email
        users.remove(oldEmail);
        user.email = newEmail;
        users.put(newEmail, user);
        
        return "Success: Email changed";
    }
    
    // Logout user
    public static String logout(String email) {
        email = sanitizeInput(email);
        User user = users.get(email);
        if (user != null) {
            user.loggedIn = false;
            return "Success: Logged out";
        }
        return "Error: User not found";
    }
    
    public static void main(String[] args) {
        // Test Case 1: Register and login
        System.out.println("Test 1: Register and Login");
        System.out.println(registerUser("user1@example.com", "SecurePass123!"));
        System.out.println(login("user1@example.com", "SecurePass123!"));
        
        // Test Case 2: Change email with valid credentials
        System.out.println("\\nTest 2: Change email successfully");
        System.out.println(changeEmail("user1@example.com", "newuser1@example.com", "SecurePass123!"));
        
        // Test Case 3: Try to change email without being logged in
        System.out.println("\\nTest 3: Change email without login");
        System.out.println(registerUser("user2@example.com", "Password456!"));
        System.out.println(changeEmail("user2@example.com", "newuser2@example.com", "Password456!"));
        
        // Test Case 4: Try to change email with wrong password
        System.out.println("\\nTest 4: Change email with wrong password");
        System.out.println(registerUser("user3@example.com", "MyPass789!"));
        System.out.println(login("user3@example.com", "MyPass789!"));
        System.out.println(changeEmail("user3@example.com", "newuser3@example.com", "WrongPassword"));
        
        // Test Case 5: Try to change to same email
        System.out.println("\\nTest 5: Change to same email");
        System.out.println(registerUser("user4@example.com", "TestPass000!"));
        System.out.println(login("user4@example.com", "TestPass000!"));
        System.out.println(changeEmail("user4@example.com", "user4@example.com", "TestPass000!"));
    }
}
