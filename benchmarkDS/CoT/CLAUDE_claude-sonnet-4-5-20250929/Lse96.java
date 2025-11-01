
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse96 {
    private static final int SALT_LENGTH = 16;
    private static final int ITERATIONS = 10000;
    private static final int KEY_LENGTH = 256;
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";
    
    // Simulated database
    private static Map<String, String> database = new HashMap<>();
    
    public static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }
    
    public static String hashPassword(String password, String salt) 
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        byte[] saltBytes = Base64.getDecoder().decode(salt);
        PBEKeySpec spec = new PBEKeySpec(password.toCharArray(), saltBytes, ITERATIONS, KEY_LENGTH);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
        byte[] hash = factory.generateSecret(spec).getEncoded();
        spec.clearPassword();
        return Base64.getEncoder().encodeToString(hash);
    }
    
    public static boolean registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || 
            password == null || password.length() < 8) {
            return false;
        }
        
        try {
            String salt = generateSalt();
            String hashedPassword = hashPassword(password, salt);
            String storedValue = salt + ":" + hashedPassword;
            
            // Simulated database storage (in production, use prepared statements)
            database.put(username, storedValue);
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }
    
    public static boolean verifyUser(String username, String password) {
        String storedValue = database.get(username);
        if (storedValue == null) {
            return false;
        }
        
        try {
            String[] parts = storedValue.split(":");
            String salt = parts[0];
            String storedHash = parts[1];
            String computedHash = hashPassword(password, salt);
            return storedHash.equals(computedHash);
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test Case 1: Valid registration
        System.out.println("Test 1 - Valid registration: " + 
            registerUser("user1", "SecurePass123!"));
        
        // Test Case 2: Valid registration and verification
        registerUser("user2", "MyP@ssw0rd");
        System.out.println("Test 2 - Valid login: " + 
            verifyUser("user2", "MyP@ssw0rd"));
        
        // Test Case 3: Invalid password (too short)
        System.out.println("Test 3 - Short password: " + 
            registerUser("user3", "short"));
        
        // Test Case 4: Invalid login
        registerUser("user4", "ValidPass123");
        System.out.println("Test 4 - Wrong password: " + 
            verifyUser("user4", "WrongPass123"));
        
        // Test Case 5: Empty username
        System.out.println("Test 5 - Empty username: " + 
            registerUser("", "ValidPass123"));
    }
}
