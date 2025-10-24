
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.Base64;

public class Task82 {
    private static final int SALT_LENGTH = 16;
    private static final int ITERATIONS = 210000;
    private static final int KEY_LENGTH = 256;
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";
    
    public static class HashResult {
        public final byte[] salt;
        public final byte[] hash;
        
        public HashResult(byte[] salt, byte[] hash) {
            this.salt = Arrays.copyOf(salt, salt.length);
            this.hash = Arrays.copyOf(hash, hash.length);
        }
    }
    
    public static HashResult hashPassword(char[] password, byte[] salt) {
        if (password == null || password.length == 0) {
            throw new IllegalArgumentException("Password cannot be null or empty");
        }
        if (salt == null || salt.length != SALT_LENGTH) {
            throw new IllegalArgumentException("Salt must be exactly " + SALT_LENGTH + " bytes");
        }
        
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] hash = factory.generateSecret(spec).getEncoded();
            spec.clearPassword();
            return new HashResult(salt, hash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error computing password hash", e);
        }
    }
    
    public static HashResult hashPassword(char[] password) {
        if (password == null || password.length == 0) {
            throw new IllegalArgumentException("Password cannot be null or empty");
        }
        
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        
        return hashPassword(password, salt);
    }
    
    public static boolean verifyPassword(char[] password, byte[] salt, byte[] expectedHash) {
        if (password == null || salt == null || expectedHash == null) {
            return false;
        }
        
        HashResult result = hashPassword(password, salt);
        return constantTimeEquals(result.hash, expectedHash);
    }
    
    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a.length != b.length) {
            return false;
        }
        
        int diff = 0;
        for (int i = 0; i < a.length; i++) {
            diff |= a[i] ^ b[i];
        }
        return diff == 0;
    }
    
    public static void main(String[] args) {
        System.out.println("Test Case 1: Hash password with auto-generated salt");
        char[] password1 = "SecurePassword123!".toCharArray();
        HashResult result1 = hashPassword(password1);
        System.out.println("Salt (Base64): " + Base64.getEncoder().encodeToString(result1.salt));
        System.out.println("Hash (Base64): " + Base64.getEncoder().encodeToString(result1.hash));
        System.out.println();
        
        System.out.println("Test Case 2: Hash password with provided salt");
        char[] password2 = "MyPassword456".toCharArray();
        byte[] salt2 = new byte[SALT_LENGTH];
        new SecureRandom().nextBytes(salt2);
        HashResult result2 = hashPassword(password2, salt2);
        System.out.println("Salt (Base64): " + Base64.getEncoder().encodeToString(result2.salt));
        System.out.println("Hash (Base64): " + Base64.getEncoder().encodeToString(result2.hash));
        System.out.println();
        
        System.out.println("Test Case 3: Verify correct password");
        char[] password3 = "TestPassword789".toCharArray();
        HashResult result3 = hashPassword(password3);
        boolean verified3 = verifyPassword(password3, result3.salt, result3.hash);
        System.out.println("Verification result: " + verified3);
        System.out.println();
        
        System.out.println("Test Case 4: Verify incorrect password");
        char[] wrongPassword = "WrongPassword".toCharArray();
        boolean verified4 = verifyPassword(wrongPassword, result3.salt, result3.hash);
        System.out.println("Verification result: " + verified4);
        System.out.println();
        
        System.out.println("Test Case 5: Same password, different salts produce different hashes");
        char[] password5 = "SamePassword".toCharArray();
        HashResult result5a = hashPassword(password5);
        HashResult result5b = hashPassword(password5);
        boolean hashesEqual = Arrays.equals(result5a.hash, result5b.hash);
        System.out.println("Hashes equal: " + hashesEqual + " (should be false)");
        System.out.println("Hash 1 (Base64): " + Base64.getEncoder().encodeToString(result5a.hash));
        System.out.println("Hash 2 (Base64): " + Base64.getEncoder().encodeToString(result5b.hash));
    }
}
