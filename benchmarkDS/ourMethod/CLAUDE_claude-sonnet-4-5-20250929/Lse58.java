
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse58 {
    // Email validation pattern - RFC 5322 simplified
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$"
    );
    
    // Password minimum length requirement
    private static final int MIN_PASSWORD_LENGTH = 8;
    private static final int SALT_LENGTH = 16;
    private static final int PBKDF2_ITERATIONS = 210000;
    
    // In-memory user store (in production, use a secure database)
    private static Map<String, UserAccount> userStore = new HashMap<>();
    private static Map<String, String> sessionStore = new HashMap<>();
    
    static class UserAccount {
        String email;
        byte[] passwordHash;
        byte[] salt;
        
        UserAccount(String email, byte[] passwordHash, byte[] salt) {
            this.email = email;
            this.passwordHash = passwordHash;
            this.salt = salt;
        }
    }
    
    // Securely hash password with salt using PBKDF2-HMAC-SHA-256
    private static byte[] hashPassword(String password, byte[] salt) {
        try {
            javax.crypto.spec.PBEKeySpec spec = new javax.crypto.spec.PBEKeySpec(
                password.toCharArray(), salt, PBKDF2_ITERATIONS, 256);
            javax.crypto.SecretKeyFactory factory = javax.crypto.SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = factory.generateSecret(spec).getEncoded();
            spec.clearPassword(); // Clear password from memory
            return hash;
        } catch (Exception e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    // Generate cryptographically secure random salt
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }
    
    // Validate email format
    private static boolean isValidEmail(String email) {
        if (email == null || email.length() > 254) {
            return false;
        }
        return EMAIL_PATTERN.matcher(email).matches();
    }
    
    // Validate password strength
    private static boolean isValidPassword(String password) {
        if (password == null || password.length() < MIN_PASSWORD_LENGTH) {
            return false;
        }
        // Check for at least one digit, one lowercase, one uppercase
        boolean hasDigit = false;
        boolean hasLower = false;
        boolean hasUpper = false;
        
        for (char c : password.toCharArray()) {
            if (Character.isDigit(c)) hasDigit = true;
            if (Character.isLowerCase(c)) hasLower = true;
            if (Character.isUpperCase(c)) hasUpper = true;
        }
        
        return hasDigit && hasLower && hasUpper;
    }
    
    // Register a new user (for testing purposes)
    public static String registerUser(String email, String password) {
        // Validate input parameters
        if (email == null || password == null) {
            return "Error: Invalid input";
        }
        
        if (!isValidEmail(email)) {
            return "Error: Invalid email format";
        }
        
        if (!isValidPassword(password)) {
            return "Error: Password must be at least " + MIN_PASSWORD_LENGTH + 
                   " characters and contain uppercase, lowercase, and digit";
        }
        
        // Check if user already exists
        if (userStore.containsKey(email)) {
            return "Error: User already exists";
        }
        
        // Generate unique salt for this user
        byte[] salt = generateSalt();
        
        // Hash password with salt
        byte[] passwordHash = hashPassword(password, salt);
        
        // Store user account
        userStore.put(email, new UserAccount(email, passwordHash, salt));
        
        return "Success: User registered";
    }
    
    // Login and create session
    public static String login(String email, String password) {
        // Validate input parameters
        if (email == null || password == null) {
            return null;
        }
        
        // Retrieve user account
        UserAccount account = userStore.get(email);
        if (account == null) {
            return null; // User not found - generic error
        }
        
        // Hash provided password with stored salt
        byte[] providedHash = hashPassword(password, account.salt);
        
        // Constant-time comparison to prevent timing attacks
        if (!MessageDigest.isEqual(providedHash, account.passwordHash)) {
            return null; // Invalid password - generic error
        }
        
        // Generate secure session token
        SecureRandom random = new SecureRandom();
        byte[] tokenBytes = new byte[32];
        random.nextBytes(tokenBytes);
        String sessionToken = Base64.getEncoder().encodeToString(tokenBytes);
        
        // Store session
        sessionStore.put(sessionToken, email);
        
        return sessionToken;
    }
    
    // Validate session token
    private static String getEmailFromSession(String sessionToken) {
        if (sessionToken == null || sessionToken.isEmpty()) {
            return null;
        }
        return sessionStore.get(sessionToken);
    }
    
    // Change email with authentication
    public static String changeEmail(String sessionToken, String oldEmail, 
                                    String newEmail, String password) {
        // Validate all input parameters
        if (sessionToken == null || oldEmail == null || 
            newEmail == null || password == null) {
            return "Error: Invalid input";
        }
        
        // Verify session is valid (user must be logged in)
        String sessionEmail = getEmailFromSession(sessionToken);
        if (sessionEmail == null) {
            return "Error: Not authenticated";
        }
        
        // Verify old email matches session email
        if (!sessionEmail.equals(oldEmail)) {
            return "Error: Email mismatch";
        }
        
        // Validate new email format
        if (!isValidEmail(newEmail)) {
            return "Error: Invalid new email format";
        }
        
        // Check if new email is already taken
        if (userStore.containsKey(newEmail) && !newEmail.equals(oldEmail)) {
            return "Error: Email already in use";
        }
        
        // Retrieve user account
        UserAccount account = userStore.get(oldEmail);
        if (account == null) {
            return "Error: User not found";
        }
        
        // Verify password using constant-time comparison
        byte[] providedHash = hashPassword(password, account.salt);
        if (!MessageDigest.isEqual(providedHash, account.passwordHash)) {
            return "Error: Invalid password";
        }
        
        // Update email in user store
        userStore.remove(oldEmail);
        account.email = newEmail;
        userStore.put(newEmail, account);
        
        // Update session
        sessionStore.put(sessionToken, newEmail);
        
        return "Success: Email changed";
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Register and change email successfully ===");
        String result1 = registerUser("user1@example.com", "SecurePass123");
        System.out.println("Register: " + result1);
        String token1 = login("user1@example.com", "SecurePass123");
        System.out.println("Login: " + (token1 != null ? "Success" : "Failed"));
        String change1 = changeEmail(token1, "user1@example.com", 
                                    "newemail1@example.com", "SecurePass123");
        System.out.println("Change email: " + change1);
        System.out.println();
        
        System.out.println("=== Test Case 2: Wrong old email ===");
        String result2 = registerUser("user2@example.com", "StrongPass456");
        System.out.println("Register: " + result2);
        String token2 = login("user2@example.com", "StrongPass456");
        String change2 = changeEmail(token2, "wrong@example.com", 
                                    "new2@example.com", "StrongPass456");
        System.out.println("Change email with wrong old email: " + change2);
        System.out.println();
        
        System.out.println("=== Test Case 3: Wrong password ===");
        String result3 = registerUser("user3@example.com", "MyPassword789");
        System.out.println("Register: " + result3);
        String token3 = login("user3@example.com", "MyPassword789");
        String change3 = changeEmail(token3, "user3@example.com", 
                                    "new3@example.com", "WrongPass123");
        System.out.println("Change email with wrong password: " + change3);
        System.out.println();
        
        System.out.println("=== Test Case 4: Not logged in ===");
        String change4 = changeEmail("invalid_token", "user4@example.com", 
                                    "new4@example.com", "AnyPass123");
        System.out.println("Change email without login: " + change4);
        System.out.println();
        
        System.out.println("=== Test Case 5: Invalid new email format ===");
        String result5 = registerUser("user5@example.com", "ValidPass999");
        System.out.println("Register: " + result5);
        String token5 = login("user5@example.com", "ValidPass999");
        String change5 = changeEmail(token5, "user5@example.com", 
                                    "invalid-email", "ValidPass999");
        System.out.println("Change to invalid email: " + change5);
    }
}
