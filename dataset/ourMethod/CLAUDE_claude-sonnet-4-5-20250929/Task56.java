
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;

public class Task56 {
    private static final int TOKEN_LENGTH = 32;
    private static final int SALT_LENGTH = 16;
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final int KEY_LENGTH = 256;
    
    public static String generateToken() {
        SecureRandom secureRandom = new SecureRandom();
        byte[] tokenBytes = new byte[TOKEN_LENGTH];
        secureRandom.nextBytes(tokenBytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
    }
    
    public static String hashToken(String token) {
        if (token == null || token.isEmpty()) {
            throw new IllegalArgumentException("Token cannot be null or empty");
        }
        
        try {
            SecureRandom secureRandom = new SecureRandom();
            byte[] salt = new byte[SALT_LENGTH];
            secureRandom.nextBytes(salt);
            
            PBEKeySpec spec = new PBEKeySpec(
                token.toCharArray(),
                salt,
                PBKDF2_ITERATIONS,
                KEY_LENGTH
            );
            
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] hash = factory.generateSecret(spec).getEncoded();
            spec.clearPassword();
            
            byte[] combined = new byte[salt.length + hash.length];
            System.arraycopy(salt, 0, combined, 0, salt.length);
            System.arraycopy(hash, 0, combined, salt.length, hash.length);
            
            return Base64.getEncoder().encodeToString(combined);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new RuntimeException("Error hashing token", e);
        }
    }
    
    public static boolean verifyToken(String token, String storedHash) {
        if (token == null || token.isEmpty() || storedHash == null || storedHash.isEmpty()) {
            return false;
        }
        
        try {
            byte[] combined = Base64.getDecoder().decode(storedHash);
            
            if (combined.length < SALT_LENGTH) {
                return false;
            }
            
            byte[] salt = new byte[SALT_LENGTH];
            byte[] hash = new byte[combined.length - SALT_LENGTH];
            System.arraycopy(combined, 0, salt, 0, SALT_LENGTH);
            System.arraycopy(combined, SALT_LENGTH, hash, 0, hash.length);
            
            PBEKeySpec spec = new PBEKeySpec(
                token.toCharArray(),
                salt,
                PBKDF2_ITERATIONS,
                KEY_LENGTH
            );
            
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] testHash = factory.generateSecret(spec).getEncoded();
            spec.clearPassword();
            
            return constantTimeEquals(hash, testHash);
        } catch (Exception e) {
            return false;
        }
    }
    
    private static boolean constantTimeEquals(byte[] a, byte[] b) {
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
        System.out.println("Test Case 1: Generate token");
        String token1 = generateToken();
        System.out.println("Generated token: " + token1);
        System.out.println("Token length valid: " + (token1.length() > 0));
        
        System.out.println("\\nTest Case 2: Hash token");
        String hash1 = hashToken(token1);
        System.out.println("Hashed token stored (length): " + hash1.length());
        
        System.out.println("\\nTest Case 3: Verify correct token");
        boolean verified = verifyToken(token1, hash1);
        System.out.println("Token verified: " + verified);
        
        System.out.println("\\nTest Case 4: Verify incorrect token");
        String token2 = generateToken();
        boolean notVerified = verifyToken(token2, hash1);
        System.out.println("Different token rejected: " + !notVerified);
        
        System.out.println("\\nTest Case 5: Multiple unique tokens");
        String token3 = generateToken();
        String token4 = generateToken();
        String token5 = generateToken();
        boolean allUnique = !token3.equals(token4) && !token3.equals(token5) && !token4.equals(token5);
        System.out.println("All tokens unique: " + allUnique);
    }
}
