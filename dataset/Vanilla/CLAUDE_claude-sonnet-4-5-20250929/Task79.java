
import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;
import java.util.Base64;

public class Task79 {
    public static String encryptMessage(String message, String secretKey) {
        try {
            // Ensure the key is 16 bytes (128 bits) for AES
            byte[] key = new byte[16];
            byte[] keyBytes = secretKey.getBytes("UTF-8");
            System.arraycopy(keyBytes, 0, key, 0, Math.min(keyBytes.length, key.length));
            
            SecretKeySpec secretKeySpec = new SecretKeySpec(key, "AES");
            Cipher cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec);
            
            byte[] encryptedBytes = cipher.doFinal(message.getBytes("UTF-8"));
            return Base64.getEncoder().encodeToString(encryptedBytes);
        } catch (Exception e) {
            return "Encryption error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        // Test case 1
        String message1 = "Hello World";
        String key1 = "mySecretKey12345";
        System.out.println("Test 1:");
        System.out.println("Message: " + message1);
        System.out.println("Key: " + key1);
        System.out.println("Encrypted: " + encryptMessage(message1, key1));
        System.out.println();
        
        // Test case 2
        String message2 = "This is a secret message";
        String key2 = "password123";
        System.out.println("Test 2:");
        System.out.println("Message: " + message2);
        System.out.println("Key: " + key2);
        System.out.println("Encrypted: " + encryptMessage(message2, key2));
        System.out.println();
        
        // Test case 3
        String message3 = "OpenAI GPT";
        String key3 = "key123";
        System.out.println("Test 3:");
        System.out.println("Message: " + message3);
        System.out.println("Key: " + key3);
        System.out.println("Encrypted: " + encryptMessage(message3, key3));
        System.out.println();
        
        // Test case 4
        String message4 = "Encryption test 2024";
        String key4 = "supersecret";
        System.out.println("Test 4:");
        System.out.println("Message: " + message4);
        System.out.println("Key: " + key4);
        System.out.println("Encrypted: " + encryptMessage(message4, key4));
        System.out.println();
        
        // Test case 5
        String message5 = "12345";
        String key5 = "numerickey";
        System.out.println("Test 5:");
        System.out.println("Message: " + message5);
        System.out.println("Key: " + key5);
        System.out.println("Encrypted: " + encryptMessage(message5, key5));
    }
}
