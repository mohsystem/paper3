
import javax.crypto.Cipher;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Task42 {
    private static final int SALT_LENGTH = 16;
    private static final int IV_LENGTH = 12;
    private static final int TAG_LENGTH = 128;
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final String ALGORITHM = "AES/GCM/NoPadding";
    private static final SecureRandom secureRandom = new SecureRandom();
    
    private Map<String, byte[]> userDatabase;
    
    public Task42() {
        this.userDatabase = new HashMap<>();
    }
    
    public boolean registerUser(String username, String password) {
        if (username == null || username.isEmpty() || username.length() > 100) {
            return false;
        }
        if (password == null || password.length() < 8 || password.length() > 128) {
            return false;
        }
        if (userDatabase.containsKey(username)) {
            return false;
        }
        
        try {
            byte[] hashedPassword = hashPassword(password);
            userDatabase.put(username, hashedPassword);
            return true;
        } catch (Exception e) {
            return false;
        }
    }
    
    public boolean authenticateUser(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        
        byte[] storedHash = userDatabase.get(username);
        if (storedHash == null) {
            byte[] dummySalt = new byte[SALT_LENGTH];
            secureRandom.nextBytes(dummySalt);
            try {
                deriveKey(password, dummySalt);
            } catch (Exception e) {
                // Ignore
            }
            return false;
        }
        
        try {
            return verifyPassword(password, storedHash);
        } catch (Exception e) {
            return false;
        }
    }
    
    private byte[] hashPassword(String password) throws Exception {
        byte[] salt = new byte[SALT_LENGTH];
        secureRandom.nextBytes(salt);
        
        byte[] hash = deriveKey(password, salt);
        
        ByteBuffer buffer = ByteBuffer.allocate(SALT_LENGTH + hash.length);
        buffer.put(salt);
        buffer.put(hash);
        
        return buffer.array();
    }
    
    private boolean verifyPassword(String password, byte[] storedData) throws Exception {
        if (storedData.length < SALT_LENGTH + 32) {
            return false;
        }
        
        byte[] salt = Arrays.copyOfRange(storedData, 0, SALT_LENGTH);
        byte[] storedHash = Arrays.copyOfRange(storedData, SALT_LENGTH, storedData.length);
        
        byte[] computedHash = deriveKey(password, salt);
        
        return constantTimeEquals(storedHash, computedHash);
    }
    
    private byte[] deriveKey(String password, byte[] salt) throws Exception {
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, PBKDF2_ITERATIONS, 256);
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] key = factory.generateSecret(spec).getEncoded();
        Arrays.fill(spec.getPassword(), '\\0');
        return key;
    }
    
    private boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a.length != b.length) {
            return false;
        }
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    public static void main(String[] args) {
        Task42 auth = new Task42();
        
        System.out.println("Test 1 - Register valid user:");
        boolean test1 = auth.registerUser("alice", "SecurePass123!");
        System.out.println("Registration successful: " + test1);
        
        System.out.println("\\nTest 2 - Authenticate with correct password:");
        boolean test2 = auth.authenticateUser("alice", "SecurePass123!");
        System.out.println("Authentication successful: " + test2);
        
        System.out.println("\\nTest 3 - Authenticate with wrong password:");
        boolean test3 = auth.authenticateUser("alice", "WrongPassword");
        System.out.println("Authentication successful (should be false): " + test3);
        
        System.out.println("\\nTest 4 - Register duplicate user:");
        boolean test4 = auth.registerUser("alice", "AnotherPass456");
        System.out.println("Registration successful (should be false): " + test4);
        
        System.out.println("\\nTest 5 - Authenticate non-existent user:");
        boolean test5 = auth.authenticateUser("bob", "SomePassword");
        System.out.println("Authentication successful (should be false): " + test5);
    }
}
