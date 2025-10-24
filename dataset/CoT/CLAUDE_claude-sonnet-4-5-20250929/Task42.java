
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

class Task42 {
    private static final int SALT_LENGTH = 16;
    private static final int MAX_LOGIN_ATTEMPTS = 3;
    private Map<String, UserCredentials> userDatabase;
    private Map<String, Integer> loginAttempts;
    
    static class UserCredentials {
        String passwordHash;
        String salt;
        
        UserCredentials(String passwordHash, String salt) {
            this.passwordHash = passwordHash;
            this.salt = salt;
        }
    }
    
    public Task42() {
        this.userDatabase = new HashMap<>();
        this.loginAttempts = new HashMap<>();
    }
    
    private String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    private String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    public boolean registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || 
            password == null || password.length() < 8) {
            return false;
        }
        
        if (userDatabase.containsKey(username)) {
            return false;
        }
        
        String salt = generateSalt();
        String passwordHash = hashPassword(password, salt);
        userDatabase.put(username, new UserCredentials(passwordHash, salt));
        loginAttempts.put(username, 0);
        return true;
    }
    
    public boolean authenticateUser(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        
        if (!userDatabase.containsKey(username)) {
            return false;
        }
        
        int attempts = loginAttempts.getOrDefault(username, 0);
        if (attempts >= MAX_LOGIN_ATTEMPTS) {
            return false;
        }
        
        UserCredentials credentials = userDatabase.get(username);
        String passwordHash = hashPassword(password, credentials.salt);
        
        if (passwordHash.equals(credentials.passwordHash)) {
            loginAttempts.put(username, 0);
            return true;
        } else {
            loginAttempts.put(username, attempts + 1);
            return false;
        }
    }
    
    public boolean changePassword(String username, String oldPassword, String newPassword) {
        if (!authenticateUser(username, oldPassword)) {
            return false;
        }
        
        if (newPassword == null || newPassword.length() < 8) {
            return false;
        }
        
        String salt = generateSalt();
        String passwordHash = hashPassword(newPassword, salt);
        userDatabase.put(username, new UserCredentials(passwordHash, salt));
        return true;
    }
    
    public void resetLoginAttempts(String username) {
        if (userDatabase.containsKey(username)) {
            loginAttempts.put(username, 0);
        }
    }
    
    public static void main(String[] args) {
        Task42 authSystem = new Task42();
        
        System.out.println("Test Case 1: User Registration");
        boolean result1 = authSystem.registerUser("alice", "SecurePass123");
        System.out.println("Register alice: " + (result1 ? "SUCCESS" : "FAILED"));
        
        System.out.println("\\nTest Case 2: Successful Authentication");
        boolean result2 = authSystem.authenticateUser("alice", "SecurePass123");
        System.out.println("Authenticate alice: " + (result2 ? "SUCCESS" : "FAILED"));
        
        System.out.println("\\nTest Case 3: Failed Authentication (wrong password)");
        boolean result3 = authSystem.authenticateUser("alice", "WrongPass123");
        System.out.println("Authenticate with wrong password: " + (result3 ? "SUCCESS" : "FAILED"));
        
        System.out.println("\\nTest Case 4: Password Change");
        boolean result4 = authSystem.changePassword("alice", "SecurePass123", "NewSecure456");
        System.out.println("Change password: " + (result4 ? "SUCCESS" : "FAILED"));
        boolean result4b = authSystem.authenticateUser("alice", "NewSecure456");
        System.out.println("Authenticate with new password: " + (result4b ? "SUCCESS" : "FAILED"));
        
        System.out.println("\\nTest Case 5: Account Lockout after multiple failed attempts");
        authSystem.registerUser("bob", "BobPass123");
        authSystem.authenticateUser("bob", "wrong1");
        authSystem.authenticateUser("bob", "wrong2");
        authSystem.authenticateUser("bob", "wrong3");
        boolean result5 = authSystem.authenticateUser("bob", "BobPass123");
        System.out.println("Authenticate after 3 failed attempts: " + (result5 ? "SUCCESS" : "FAILED (LOCKED)"));
    }
}
