
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task122 {
    private static Map<String, UserCredentials> userDatabase = new HashMap<>();
    
    static class UserCredentials {
        String username;
        String hashedPassword;
        String salt;
        
        UserCredentials(String username, String hashedPassword, String salt) {
            this.username = username;
            this.hashedPassword = hashedPassword;
            this.salt = salt;
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
    
    public static String signup(String username, String password) {
        if (username == null || username.trim().isEmpty()) {
            return "Error: Username cannot be empty";
        }
        if (password == null || password.length() < 6) {
            return "Error: Password must be at least 6 characters";
        }
        if (userDatabase.containsKey(username)) {
            return "Error: Username already exists";
        }
        
        String salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        userDatabase.put(username, new UserCredentials(username, hashedPassword, salt));
        return "Success: User " + username + " registered successfully";
    }
    
    public static boolean verifyPassword(String username, String password) {
        UserCredentials user = userDatabase.get(username);
        if (user == null) {
            return false;
        }
        String hashedInput = hashPassword(password, user.salt);
        return hashedInput.equals(user.hashedPassword);
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Valid signup");
        System.out.println(signup("john_doe", "password123"));
        
        System.out.println("\\nTest Case 2: Duplicate username");
        System.out.println(signup("john_doe", "newpassword456"));
        
        System.out.println("\\nTest Case 3: Weak password");
        System.out.println(signup("jane_doe", "123"));
        
        System.out.println("\\nTest Case 4: Empty username");
        System.out.println(signup("", "password123"));
        
        System.out.println("\\nTest Case 5: Multiple valid signups and verification");
        System.out.println(signup("alice", "securePass789"));
        System.out.println(signup("bob", "myPassword!"));
        System.out.println("Verify alice password: " + verifyPassword("alice", "securePass789"));
        System.out.println("Verify bob wrong password: " + verifyPassword("bob", "wrongPass"));
    }
}
