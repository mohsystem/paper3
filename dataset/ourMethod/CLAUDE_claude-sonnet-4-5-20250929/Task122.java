
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task122 {
    private static final int SALT_LENGTH = 16;
    private static final int ITERATIONS = 210000;
    private static final int KEY_LENGTH = 256;
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";
    
    private Map<String, StoredPassword> passwordStore;
    private SecureRandom secureRandom;
    
    public Task122() {
        this.passwordStore = new HashMap<>();
        this.secureRandom = new SecureRandom();
    }
    
    private static class StoredPassword {
        private final byte[] salt;
        private final byte[] hash;
        
        public StoredPassword(byte[] salt, byte[] hash) {
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
        }
        
        public byte[] getSalt() {
            return Arrays.copyOf(salt, salt.length);
        }
        
        public byte[] getHash() {
            return Arrays.copyOf(hash, hash.length);
        }
    }
    
    public boolean signup(String username, String password) {
        if (username == null || username.trim().isEmpty() || username.length() > 255) {
            return false;
        }
        
        if (password == null || password.length() < 8 || password.length() > 128) {
            return false;
        }
        
        if (passwordStore.containsKey(username)) {
            return false;
        }
        
        try {
            byte[] salt = new byte[SALT_LENGTH];
            secureRandom.nextBytes(salt);
            
            byte[] hash = hashPassword(password, salt);
            
            passwordStore.put(username, new StoredPassword(salt, hash));
            
            return true;
        } catch (Exception e) {
            return false;
        }
    }
    
    public boolean verifyPassword(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        
        StoredPassword stored = passwordStore.get(username);
        if (stored == null) {
            return false;
        }
        
        try {
            byte[] computedHash = hashPassword(password, stored.getSalt());
            return constantTimeEquals(computedHash, stored.getHash());
        } catch (Exception e) {
            return false;
        }
    }
    
    private byte[] hashPassword(String password, byte[] salt) 
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        KeySpec spec = new PBEKeySpec(
            password.toCharArray(),
            salt,
            ITERATIONS,
            KEY_LENGTH
        );
        SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
        return factory.generateSecret(spec).getEncoded();
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
        Task122 system = new Task122();
        
        System.out.println("Test 1: Valid signup");
        boolean result1 = system.signup("user1", "SecurePass123!");
        System.out.println("Signup user1: " + result1);
        
        System.out.println("\\nTest 2: Verify correct password");
        boolean result2 = system.verifyPassword("user1", "SecurePass123!");
        System.out.println("Verify user1 correct password: " + result2);
        
        System.out.println("\\nTest 3: Verify incorrect password");
        boolean result3 = system.verifyPassword("user1", "WrongPassword");
        System.out.println("Verify user1 wrong password: " + result3);
        
        System.out.println("\\nTest 4: Duplicate username");
        boolean result4 = system.signup("user1", "AnotherPass456!");
        System.out.println("Signup duplicate user1: " + result4);
        
        System.out.println("\\nTest 5: Invalid password length");
        boolean result5 = system.signup("user2", "short");
        System.out.println("Signup with short password: " + result5);
    }
}
