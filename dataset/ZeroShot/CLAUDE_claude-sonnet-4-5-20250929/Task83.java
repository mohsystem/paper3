
import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;
import javax.crypto.spec.IvParameterSpec;
import java.util.Base64;
import java.security.SecureRandom;

public class Task83 {
    public static String encryptAES(String key, String plainText) {
        try {
            // Generate a random IV (Initialization Vector)
            byte[] iv = new byte[16];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv);
            IvParameterSpec ivSpec = new IvParameterSpec(iv);
            
            // Create secret key from the provided key
            byte[] keyBytes = new byte[16];
            byte[] inputKeyBytes = key.getBytes("UTF-8");
            System.arraycopy(inputKeyBytes, 0, keyBytes, 0, Math.min(inputKeyBytes.length, keyBytes.length));
            SecretKeySpec secretKey = new SecretKeySpec(keyBytes, "AES");
            
            // Create cipher instance
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, secretKey, ivSpec);
            
            // Encrypt the plaintext
            byte[] encrypted = cipher.doFinal(plainText.getBytes("UTF-8"));
            
            // Combine IV and encrypted data
            byte[] combined = new byte[iv.length + encrypted.length];
            System.arraycopy(iv, 0, combined, 0, iv.length);
            System.arraycopy(encrypted, 0, combined, iv.length, encrypted.length);
            
            // Return Base64 encoded result
            return Base64.getEncoder().encodeToString(combined);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        String[] keys = {
            "mySecretKey12345",
            "anotherKey456789",
            "testKey123456789",
            "secureKey9876543",
            "randomKey1122334"
        };
        
        String[] plainTexts = {
            "Hello World",
            "This is a test message",
            "AES encryption in CBC mode",
            "Secure data transmission",
            "Testing encryption"
        };
        
        for (int i = 0; i < keys.length; i++) {
            String encrypted = encryptAES(keys[i], plainTexts[i]);
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Key: " + keys[i]);
            System.out.println("Plain Text: " + plainTexts[i]);
            System.out.println("Encrypted: " + encrypted);
            System.out.println();
        }
    }
}
