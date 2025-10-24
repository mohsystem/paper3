
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task122 {
    private static Map<String, UserCredentials> userDatabase = new HashMap<>();
    
    static class UserCredentials {
        String salt;
        String hashedPassword;
        
        UserCredentials(String salt, String hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }
    }
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static boolean signup(String username, String password) {
        if (username == null || username.trim().isEmpty() || 
            password == null || password.length() < 8) {
            return false;
        }
        
        if (userDatabase.containsKey(username)) {
            return false;
        }
        
        String salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        userDatabase.put(username, new UserCredentials(salt, hashedPassword));
        return true;
    }
    
    public static boolean verifyLogin(String username, String password) {
        if (!userDatabase.containsKey(username)) {
            return false;
        }
        
        UserCredentials creds = userDatabase.get(username);
        String hashedAttempt = hashPassword(password, creds.salt);
        return hashedAttempt.equals(creds.hashedPassword);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid signup");
        boolean result1 = signup("user1", "SecurePass123");
        System.out.println("Signup successful: " + result1);
        System.out.println("Login verification: " + verifyLogin("user1", "SecurePass123"));
        System.out.println();
        
        System.out.println("Test Case 2: Duplicate username");
        boolean result2 = signup("user1", "AnotherPass456");
        System.out.println("Signup successful: " + result2);
        System.out.println();
        
        System.out.println("Test Case 3: Weak password (too short)");
        boolean result3 = signup("user2", "short");
        System.out.println("Signup successful: " + result3);
        System.out.println();
        
        System.out.println("Test Case 4: Multiple valid users");
        boolean result4a = signup("alice", "AlicePass2024");
        boolean result4b = signup("bob", "BobSecure789");
        System.out.println("Alice signup: " + result4a + ", Bob signup: " + result4b);
        System.out.println("Alice login: " + verifyLogin("alice", "AlicePass2024"));
        System.out.println("Bob login: " + verifyLogin("bob", "BobSecure789"));
        System.out.println();
        
        System.out.println("Test Case 5: Invalid login attempt");
        signup("user3", "ValidPassword123");
        boolean validLogin = verifyLogin("user3", "ValidPassword123");
        boolean invalidLogin = verifyLogin("user3", "WrongPassword");
        System.out.println("Valid login: " + validLogin);
        System.out.println("Invalid login: " + invalidLogin);
    }
}
