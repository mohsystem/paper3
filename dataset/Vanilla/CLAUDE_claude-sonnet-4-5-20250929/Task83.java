
import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;
import javax.crypto.spec.IvParameterSpec;
import java.util.Base64;
import java.security.SecureRandom;

public class Task83 {
    public static String encryptAES(String plainText, String key) {
        try {
            // Ensure key is 16 bytes (128-bit) for AES
            byte[] keyBytes = new byte[16];
            byte[] inputKeyBytes = key.getBytes("UTF-8");
            System.arraycopy(inputKeyBytes, 0, keyBytes, 0, Math.min(inputKeyBytes.length, keyBytes.length));
            
            // Generate random IV
            byte[] iv = new byte[16];
            SecureRandom random = new SecureRandom();
            random.nextBytes(iv);
            
            // Create cipher
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            SecretKeySpec secretKey = new SecretKeySpec(keyBytes, "AES");
            IvParameterSpec ivSpec = new IvParameterSpec(iv);
            
            // Encrypt
            cipher.init(Cipher.ENCRYPT_MODE, secretKey, ivSpec);
            byte[] encrypted = cipher.doFinal(plainText.getBytes("UTF-8"));
            
            // Combine IV and encrypted data
            byte[] combined = new byte[iv.length + encrypted.length];
            System.arraycopy(iv, 0, combined, 0, iv.length);
            System.arraycopy(encrypted, 0, combined, iv.length, encrypted.length);
            
            return Base64.getEncoder().encodeToString(combined);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        String encrypted1 = encryptAES("Hello World", "mySecretKey12345");
        System.out.println("Test 1 - Encrypted: " + encrypted1);
        
        // Test case 2
        String encrypted2 = encryptAES("Sensitive Data", "password1234567");
        System.out.println("Test 2 - Encrypted: " + encrypted2);
        
        // Test case 3
        String encrypted3 = encryptAES("AES Encryption", "key123456789012");
        System.out.println("Test 3 - Encrypted: " + encrypted3);
        
        // Test case 4
        String encrypted4 = encryptAES("Test Message", "secretKey123456");
        System.out.println("Test 4 - Encrypted: " + encrypted4);
        
        // Test case 5
        String encrypted5 = encryptAES("Cipher Block", "encryptionKey16");
        System.out.println("Test 5 - Encrypted: " + encrypted5);
    }
}
