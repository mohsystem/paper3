
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse57 {
    private static class User {
        String email;
        String passwordHash;
        String salt;
        
        User(String email, String passwordHash, String salt) {
            this.email = email;
            this.passwordHash = passwordHash;
            this.salt = salt;
        }
    }
    
    private static Map<String, User> userDatabase = new HashMap<>();
    private static Map<String, String> activeSessions = new HashMap<>();
    
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$"
    );
    
    public static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update((password + salt).getBytes());
            return Base64.getEncoder().encodeToString(md.digest());
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static boolean isValidEmail(String email) {
        return email != null && EMAIL_PATTERN.matcher(email).matches();
    }
    
    public static String registerUser(String email, String password) {
        if (!isValidEmail(email)) {
            return "Invalid email format";
        }
        if (password == null || password.length() < 8) {
            return "Password must be at least 8 characters";
        }
        if (userDatabase.containsKey(email)) {
            return "Email already exists";
        }
        
        String salt = generateSalt();
        String passwordHash = hashPassword(password, salt);
        userDatabase.put(email, new User(email, passwordHash, salt));
        return "Registration successful";
    }
    
    public static String login(String email, String password) {
        if (!userDatabase.containsKey(email)) {
            return null;
        }
        
        User user = userDatabase.get(email);
        String passwordHash = hashPassword(password, user.salt);
        
        if (passwordHash.equals(user.passwordHash)) {
            String sessionToken = Base64.getEncoder().encodeToString(
                (email + System.currentTimeMillis()).getBytes()
            );
            activeSessions.put(sessionToken, email);
            return sessionToken;
        }
        return null;
    }
    
    public static String changeEmail(String sessionToken, String oldEmail, 
                                    String newEmail, String confirmPassword) {
        // Verify session
        if (sessionToken == null || !activeSessions.containsKey(sessionToken)) {
            return "User not logged in";
        }
        
        String loggedInEmail = activeSessions.get(sessionToken);
        
        // Verify old email matches logged in user
        if (!loggedInEmail.equals(oldEmail)) {
            return "Old email does not match logged in user";
        }
        
        // Verify old email exists
        if (!userDatabase.containsKey(oldEmail)) {
            return "Old email does not exist";
        }
        
        // Validate new email format
        if (!isValidEmail(newEmail)) {
            return "Invalid new email format";
        }
        
        // Check if new email already exists
        if (userDatabase.containsKey(newEmail)) {
            return "New email already exists";
        }
        
        // Verify confirm password
        User user = userDatabase.get(oldEmail);
        String passwordHash = hashPassword(confirmPassword, user.salt);
        
        if (!passwordHash.equals(user.passwordHash)) {
            return "Incorrect password";
        }
        
        // Change email
        user.email = newEmail;
        userDatabase.put(newEmail, user);
        userDatabase.remove(oldEmail);
        activeSessions.put(sessionToken, newEmail);
        
        return "Email changed successfully";
    }
    
    public static void main(String[] args) {
        System.out.println("=== Test Case 1: Successful email change ===");
        registerUser("user1@example.com", "password123");
        String token1 = login("user1@example.com", "password123");
        System.out.println(changeEmail(token1, "user1@example.com", 
                                      "newuser1@example.com", "password123"));
        
        System.out.println("\\n=== Test Case 2: Invalid session token ===");
        System.out.println(changeEmail("invalid_token", "user2@example.com", 
                                      "newuser2@example.com", "password123"));
        
        System.out.println("\\n=== Test Case 3: Incorrect password ===");
        registerUser("user3@example.com", "password123");
        String token3 = login("user3@example.com", "password123");
        System.out.println(changeEmail(token3, "user3@example.com", 
                                      "newuser3@example.com", "wrongpassword"));
        
        System.out.println("\\n=== Test Case 4: New email already exists ===");
        registerUser("user4@example.com", "password123");
        registerUser("existing@example.com", "password123");
        String token4 = login("user4@example.com", "password123");
        System.out.println(changeEmail(token4, "user4@example.com", 
                                      "existing@example.com", "password123"));
        
        System.out.println("\\n=== Test Case 5: Invalid new email format ===");
        registerUser("user5@example.com", "password123");
        String token5 = login("user5@example.com", "password123");
        System.out.println(changeEmail(token5, "user5@example.com", 
                                      "invalid-email", "password123"));
    }
}
