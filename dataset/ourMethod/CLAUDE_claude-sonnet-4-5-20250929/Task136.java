
import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Task136 {
    private static final int SALT_LENGTH = 16;
    private static final int ITERATIONS = 210000;
    private static final int KEY_LENGTH = 256;
    private static final int GCM_IV_LENGTH = 12;
    private static final int GCM_TAG_LENGTH = 128;
    private static final SecureRandom secureRandom = new SecureRandom();
    
    private Map<String, UserCredentials> userDatabase;
    
    public Task136() {
        this.userDatabase = new HashMap<>();
    }
    
    private static class UserCredentials {
        byte[] passwordHash;
        byte[] salt;
        
        UserCredentials(byte[] passwordHash, byte[] salt) {
            this.passwordHash = passwordHash;
            this.salt = salt;
        }
    }
    
    public boolean registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || username.length() > 100) {
            return false;
        }
        if (password == null || password.length() < 8 || password.length() > 128) {
            return false;
        }
        
        username = username.trim();
        
        if (userDatabase.containsKey(username)) {
            return false;
        }
        
        byte[] salt = new byte[SALT_LENGTH];
        secureRandom.nextBytes(salt);
        
        byte[] passwordHash = hashPassword(password, salt);
        if (passwordHash == null) {
            return false;
        }
        
        userDatabase.put(username, new UserCredentials(passwordHash, salt));
        return true;
    }
    
    public boolean authenticateUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || username.length() > 100) {
            return false;
        }
        if (password == null || password.length() < 8 || password.length() > 128) {
            return false;
        }
        
        username = username.trim();
        
        UserCredentials credentials = userDatabase.get(username);
        if (credentials == null) {
            return false;
        }
        
        byte[] computedHash = hashPassword(password, credentials.salt);
        if (computedHash == null) {
            return false;
        }
        
        return MessageDigest.isEqual(computedHash, credentials.passwordHash);
    }
    
    private byte[] hashPassword(String password, byte[] salt) {
        try {
            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return factory.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            return null;
        }
    }
    
    public boolean deleteUser(String username) {
        if (username == null || username.trim().isEmpty()) {
            return false;
        }
        return userDatabase.remove(username.trim()) != null;
    }
    
    public boolean userExists(String username) {
        if (username == null || username.trim().isEmpty()) {
            return false;
        }
        return userDatabase.containsKey(username.trim());
    }
    
    public static void main(String[] args) {
        Task136 auth = new Task136();
        
        // Test case 1: Register and authenticate valid user
        System.out.println("Test 1 - Register and authenticate:");
        boolean registered1 = auth.registerUser("alice", "SecurePass123!");
        boolean authenticated1 = auth.authenticateUser("alice", "SecurePass123!");
        System.out.println("Registered: " + registered1 + ", Authenticated: " + authenticated1);
        
        // Test case 2: Authenticate with wrong password
        System.out.println("\\nTest 2 - Wrong password:");
        boolean authenticated2 = auth.authenticateUser("alice", "WrongPassword");
        System.out.println("Authenticated with wrong password: " + authenticated2);
        
        // Test case 3: Register duplicate user
        System.out.println("\\nTest 3 - Duplicate registration:");
        boolean registered3 = auth.registerUser("alice", "AnotherPass456!");
        System.out.println("Duplicate registration: " + registered3);
        
        // Test case 4: Authenticate non-existent user
        System.out.println("\\nTest 4 - Non-existent user:");
        boolean authenticated4 = auth.authenticateUser("bob", "SomePass789!");
        System.out.println("Non-existent user authenticated: " + authenticated4);
        
        // Test case 5: Invalid input validation
        System.out.println("\\nTest 5 - Invalid inputs:");
        boolean registered5a = auth.registerUser("", "password123");
        boolean registered5b = auth.registerUser("charlie", "short");
        boolean registered5c = auth.registerUser(null, "password123");
        System.out.println("Empty username: " + registered5a + ", Short password: " + registered5b + ", Null username: " + registered5c);
    }
}
