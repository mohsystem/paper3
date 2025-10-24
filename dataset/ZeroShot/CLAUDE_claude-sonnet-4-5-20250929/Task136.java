
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task136 {
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
    
    public static boolean registerUser(String username, String password) {
        if (username == null || username.isEmpty() || password == null || password.length() < 8) {
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
    
    public static boolean authenticateUser(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        
        UserCredentials credentials = userDatabase.get(username);
        if (credentials == null) {
            return false;
        }
        
        String hashedPassword = hashPassword(password, credentials.salt);
        return hashedPassword.equals(credentials.hashedPassword);
    }
    
    public static void main(String[] args) {
        // Test Case 1: Successful registration and authentication
        System.out.println("Test 1 - Register and authenticate user:");
        boolean registered = registerUser("alice", "SecurePass123");
        System.out.println("Registration: " + registered);
        boolean authenticated = authenticateUser("alice", "SecurePass123");
        System.out.println("Authentication: " + authenticated);
        System.out.println();
        
        // Test Case 2: Failed authentication with wrong password
        System.out.println("Test 2 - Wrong password:");
        boolean wrongPass = authenticateUser("alice", "WrongPassword");
        System.out.println("Authentication: " + wrongPass);
        System.out.println();
        
        // Test Case 3: Registration with weak password
        System.out.println("Test 3 - Weak password:");
        boolean weakPass = registerUser("bob", "weak");
        System.out.println("Registration: " + weakPass);
        System.out.println();
        
        // Test Case 4: Duplicate username registration
        System.out.println("Test 4 - Duplicate username:");
        boolean duplicate = registerUser("alice", "AnotherPass456");
        System.out.println("Registration: " + duplicate);
        System.out.println();
        
        // Test Case 5: Non-existent user authentication
        System.out.println("Test 5 - Non-existent user:");
        boolean nonExistent = authenticateUser("charlie", "SomePassword789");
        System.out.println("Authentication: " + nonExistent);
    }
}
