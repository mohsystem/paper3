
import java.util.Base64;
import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;
import java.security.MessageDigest;
import java.util.Arrays;

public class Task51 {
    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/ECB/PKCS5Padding";
    
    public static String encrypt(String plainText, String secretKey) {
        try {
            SecretKeySpec key = generateKey(secretKey);
            Cipher cipher = Cipher.getInstance(TRANSFORMATION);
            cipher.init(Cipher.ENCRYPT_MODE, key);
            byte[] encryptedBytes = cipher.doFinal(plainText.getBytes("UTF-8"));
            return Base64.getEncoder().encodeToString(encryptedBytes);
        } catch (Exception e) {
            throw new RuntimeException("Encryption failed", e);
        }
    }
    
    public static String decrypt(String encryptedText, String secretKey) {
        try {
            SecretKeySpec key = generateKey(secretKey);
            Cipher cipher = Cipher.getInstance(TRANSFORMATION);
            cipher.init(Cipher.DECRYPT_MODE, key);
            byte[] decodedBytes = Base64.getDecoder().decode(encryptedText);
            byte[] decryptedBytes = cipher.doFinal(decodedBytes);
            return new String(decryptedBytes, "UTF-8");
        } catch (Exception e) {
            throw new RuntimeException("Decryption failed", e);
        }
    }
    
    private static SecretKeySpec generateKey(String secretKey) throws Exception {
        MessageDigest sha = MessageDigest.getInstance("SHA-256");
        byte[] key = secretKey.getBytes("UTF-8");
        key = sha.digest(key);
        key = Arrays.copyOf(key, 16);
        return new SecretKeySpec(key, ALGORITHM);
    }
    
    public static void main(String[] args) {
        // Test case 1
        String text1 = "Hello World";
        String key1 = "mySecretKey123";
        String encrypted1 = encrypt(text1, key1);
        String decrypted1 = decrypt(encrypted1, key1);
        System.out.println("Test 1:");
        System.out.println("Original: " + text1);
        System.out.println("Encrypted: " + encrypted1);
        System.out.println("Decrypted: " + decrypted1);
        System.out.println();
        
        // Test case 2
        String text2 = "Secure Programming 2024";
        String key2 = "strongPassword!@#";
        String encrypted2 = encrypt(text2, key2);
        String decrypted2 = decrypt(encrypted2, key2);
        System.out.println("Test 2:");
        System.out.println("Original: " + text2);
        System.out.println("Encrypted: " + encrypted2);
        System.out.println("Decrypted: " + decrypted2);
        System.out.println();
        
        // Test case 3
        String text3 = "12345 Special!@#$%";
        String key3 = "encryptionKey789";
        String encrypted3 = encrypt(text3, key3);
        String decrypted3 = decrypt(encrypted3, key3);
        System.out.println("Test 3:");
        System.out.println("Original: " + text3);
        System.out.println("Encrypted: " + encrypted3);
        System.out.println("Decrypted: " + decrypted3);
        System.out.println();
        
        // Test case 4
        String text4 = "The quick brown fox jumps over the lazy dog";
        String key4 = "testKey2024";
        String encrypted4 = encrypt(text4, key4);
        String decrypted4 = decrypt(encrypted4, key4);
        System.out.println("Test 4:");
        System.out.println("Original: " + text4);
        System.out.println("Encrypted: " + encrypted4);
        System.out.println("Decrypted: " + decrypted4);
        System.out.println();
        
        // Test case 5
        String text5 = "AES Encryption Test";
        String key5 = "anotherSecretKey";
        String encrypted5 = encrypt(text5, key5);
        String decrypted5 = decrypt(encrypted5, key5);
        System.out.println("Test 5:");
        System.out.println("Original: " + text5);
        System.out.println("Encrypted: " + encrypted5);
        System.out.println("Decrypted: " + decrypted5);
    }
}
