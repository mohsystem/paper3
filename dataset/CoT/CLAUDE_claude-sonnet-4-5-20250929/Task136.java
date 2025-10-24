
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Task136 {
    private static final int SALT_LENGTH = 16;
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,20}$");
    private static final Pattern PASSWORD_PATTERN = Pattern.compile("^.{8,}$");
    private static final int MAX_LOGIN_ATTEMPTS = 3;
    
    private Map<String, UserCredential> userDatabase;
    private Map<String, Integer> loginAttempts;
    
    static class UserCredential {
        String username;
        String hashedPassword;
        String salt;
        
        UserCredential(String username, String hashedPassword, String salt) {
            this.username = username;
            this.hashedPassword = hashedPassword;
            this.salt = salt;
        }
    }
    
    public Task136() {
        this.userDatabase = new HashMap<>();
        this.loginAttempts = new HashMap<>();
    }
    
    public String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes("UTF-8"));
            
            // Apply additional rounds for stronger security
            for (int i = 0; i < 10000; i++) {
                md.reset();
                hashedPassword = md.digest(hashedPassword);
            }
            
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (Exception e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public boolean validateUsername(String username) {
        if (username == null || username.isEmpty()) {
            return false;
        }
        return USERNAME_PATTERN.matcher(username).matches();
    }
    
    public boolean validatePassword(String password) {
        if (password == null || password.isEmpty()) {
            return false;
        }
        return PASSWORD_PATTERN.matcher(password).matches();
    }
    
    public String registerUser(String username, String password) {
        if (!validateUsername(username)) {
            return "Invalid username. Must be 3-20 alphanumeric characters or underscore.";
        }
        
        if (!validatePassword(password)) {
            return "Invalid password. Must be at least 8 characters long.";
        }
        
        if (userDatabase.containsKey(username)) {
            return "Username already exists.";
        }
        
        String salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        
        userDatabase.put(username, new UserCredential(username, hashedPassword, salt));
        loginAttempts.put(username, 0);
        
        return "User registered successfully.";
    }
    
    public boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) {
            return false;
        }
        
        if (a.length() != b.length()) {
            return false;
        }
        
        int result = 0;
        for (int i = 0; i < a.length(); i++) {
            result |= a.charAt(i) ^ b.charAt(i);
        }
        
        return result == 0;
    }
    
    public String authenticateUser(String username, String password) {
        if (!validateUsername(username)) {
            return "Authentication failed.";
        }
        
        if (loginAttempts.getOrDefault(username, 0) >= MAX_LOGIN_ATTEMPTS) {
            return "Account locked due to too many failed attempts.";
        }
        
        UserCredential userCred = userDatabase.get(username);
        
        if (userCred == null) {
            // Perform dummy hash to prevent timing attacks
            hashPassword(password, generateSalt());
            return "Authentication failed.";
        }
        
        String hashedInputPassword = hashPassword(password, userCred.salt);
        
        if (constantTimeEquals(hashedInputPassword, userCred.hashedPassword)) {
            loginAttempts.put(username, 0);
            return "Authentication successful.";
        } else {
            int attempts = loginAttempts.getOrDefault(username, 0) + 1;
            loginAttempts.put(username, attempts);
            
            if (attempts >= MAX_LOGIN_ATTEMPTS) {
                return "Account locked due to too many failed attempts.";
            }
            
            return "Authentication failed.";
        }
    }
    
    public String resetLoginAttempts(String username) {
        if (userDatabase.containsKey(username)) {
            loginAttempts.put(username, 0);
            return "Login attempts reset successfully.";
        }
        return "User not found.";
    }
    
    public static void main(String[] args) {
        Task136 auth = new Task136();
        
        System.out.println("=== Test Case 1: Valid User Registration ===");
        System.out.println(auth.registerUser("john_doe", "SecurePass123"));
        
        System.out.println("\\n=== Test Case 2: Invalid Username (too short) ===");
        System.out.println(auth.registerUser("ab", "SecurePass123"));
        
        System.out.println("\\n=== Test Case 3: Invalid Password (too short) ===");
        System.out.println(auth.registerUser("jane_doe", "short"));
        
        System.out.println("\\n=== Test Case 4: Successful Authentication ===");
        System.out.println(auth.authenticateUser("john_doe", "SecurePass123"));
        
        System.out.println("\\n=== Test Case 5: Failed Authentication and Account Lockout ===");
        System.out.println(auth.authenticateUser("john_doe", "WrongPass1"));
        System.out.println(auth.authenticateUser("john_doe", "WrongPass2"));
        System.out.println(auth.authenticateUser("john_doe", "WrongPass3"));
        System.out.println(auth.authenticateUser("john_doe", "SecurePass123"));
        System.out.println(auth.resetLoginAttempts("john_doe"));
        System.out.println(auth.authenticateUser("john_doe", "SecurePass123"));
    }
}
