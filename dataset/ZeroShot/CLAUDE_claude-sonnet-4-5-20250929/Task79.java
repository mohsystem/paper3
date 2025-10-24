
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import java.util.Base64;

public class Task79 {
    public static String encrypt(String message, String secretKey) {
        try {
            // Create a SecretKey from the provided key
            byte[] keyBytes = new byte[16];
            byte[] paramBytes = secretKey.getBytes("UTF-8");
            System.arraycopy(paramBytes, 0, keyBytes, 0, Math.min(paramBytes.length, keyBytes.length));
            SecretKeySpec secretKeySpec = new SecretKeySpec(keyBytes, "AES");
            
            // Create cipher for encryption
            Cipher cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec);
            
            // Encrypt the message
            byte[] encryptedBytes = cipher.doFinal(message.getBytes("UTF-8"));
            
            // Return Base64 encoded string
            return Base64.getEncoder().encodeToString(encryptedBytes);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        String result1 = encrypt("Hello World", "mySecretKey12345");
        System.out.println("Test 1 - Encrypted: " + result1);
        
        // Test case 2
        String result2 = encrypt("Secure Message", "password1234567");
        System.out.println("Test 2 - Encrypted: " + result2);
        
        // Test case 3
        String result3 = encrypt("Testing 123", "key123456789012");
        System.out.println("Test 3 - Encrypted: " + result3);
        
        // Test case 4
        String result4 = encrypt("Encryption Test", "secretKey098765");
        System.out.println("Test 4 - Encrypted: " + result4);
        
        // Test case 5
        String result5 = encrypt("Java Crypto", "myKey1234567890");
        System.out.println("Test 5 - Encrypted: " + result5);
    }
}
