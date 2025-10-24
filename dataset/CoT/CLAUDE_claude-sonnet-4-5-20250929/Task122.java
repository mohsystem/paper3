
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task122 {
    private static final int SALT_LENGTH = 16;
    private static final int ITERATIONS = 10000;
    private static Map<String, UserCredentials> userDatabase = new HashMap<>();
    
    static class UserCredentials {
        String username;
        String passwordHash;
        String salt;
        
        UserCredentials(String username, String passwordHash, String salt) {
            this.username = username;
            this.passwordHash = passwordHash;
            this.salt = salt;
        }
    }
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            
            byte[] hashedPassword = md.digest(password.getBytes());
            for (int i = 0; i < ITERATIONS; i++) {
                md.reset();
                hashedPassword = md.digest(hashedPassword);
            }
            
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static boolean validatePassword(String password) {
        if (password == null || password.length() < 8) {
            return false;
        }
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }
    
    public static String signupUser(String username, String password) {
        if (username == null || username.trim().isEmpty()) {
            return "Error: Username cannot be empty";
        }
        
        if (userDatabase.containsKey(username)) {
            return "Error: Username already exists";
        }
        
        if (!validatePassword(password)) {
            return "Error: Password must be at least 8 characters and contain uppercase, lowercase, digit, and special character";
        }
        
        String salt = generateSalt();
        String passwordHash = hashPassword(password, salt);
        
        userDatabase.put(username, new UserCredentials(username, passwordHash, salt));
        
        return "Success: User registered successfully";
    }
    
    public static boolean verifyLogin(String username, String password) {
        UserCredentials user = userDatabase.get(username);
        if (user == null) {
            return false;
        }
        
        String hashedAttempt = hashPassword(password, user.salt);
        return hashedAttempt.equals(user.passwordHash);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid signup");
        System.out.println(signupUser("john_doe", "SecurePass123!"));
        
        System.out.println("\\nTest Case 2: Duplicate username");
        System.out.println(signupUser("john_doe", "AnotherPass456@"));
        
        System.out.println("\\nTest Case 3: Weak password");
        System.out.println(signupUser("jane_doe", "weak"));
        
        System.out.println("\\nTest Case 4: Valid signup");
        System.out.println(signupUser("alice_smith", "MyP@ssw0rd!"));
        
        System.out.println("\\nTest Case 5: Empty username");
        System.out.println(signupUser("", "ValidPass123!"));
        
        System.out.println("\\nLogin Verification Test:");
        System.out.println("Correct password: " + verifyLogin("john_doe", "SecurePass123!"));
        System.out.println("Wrong password: " + verifyLogin("john_doe", "WrongPass123!"));
    }
}
