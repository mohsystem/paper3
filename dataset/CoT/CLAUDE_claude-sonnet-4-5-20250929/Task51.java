
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.SecureRandom;
import java.util.Base64;

public class Task51 {
    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/GCM/NoPadding";
    private static final int GCM_TAG_LENGTH = 128;
    private static final int GCM_IV_LENGTH = 12;
    private static final int AES_KEY_SIZE = 256;
    
    public static String encrypt(String plaintext, String base64Key) throws Exception {
        if (plaintext == null || plaintext.isEmpty()) {
            throw new IllegalArgumentException("Plaintext cannot be null or empty");
        }
        if (base64Key == null || base64Key.isEmpty()) {
            throw new IllegalArgumentException("Key cannot be null or empty");
        }
        
        byte[] keyBytes = Base64.getDecoder().decode(base64Key);
        SecretKey secretKey = new SecretKeySpec(keyBytes, ALGORITHM);
        
        byte[] iv = new byte[GCM_IV_LENGTH];
        SecureRandom secureRandom = new SecureRandom();
        secureRandom.nextBytes(iv);
        
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmParameterSpec);
        
        byte[] encryptedBytes = cipher.doFinal(plaintext.getBytes("UTF-8"));
        
        byte[] encryptedWithIv = new byte[iv.length + encryptedBytes.length];
        System.arraycopy(iv, 0, encryptedWithIv, 0, iv.length);
        System.arraycopy(encryptedBytes, 0, encryptedWithIv, iv.length, encryptedBytes.length);
        
        return Base64.getEncoder().encodeToString(encryptedWithIv);
    }
    
    public static String decrypt(String encryptedText, String base64Key) throws Exception {
        if (encryptedText == null || encryptedText.isEmpty()) {
            throw new IllegalArgumentException("Encrypted text cannot be null or empty");
        }
        if (base64Key == null || base64Key.isEmpty()) {
            throw new IllegalArgumentException("Key cannot be null or empty");
        }
        
        byte[] keyBytes = Base64.getDecoder().decode(base64Key);
        SecretKey secretKey = new SecretKeySpec(keyBytes, ALGORITHM);
        
        byte[] encryptedWithIv = Base64.getDecoder().decode(encryptedText);
        
        byte[] iv = new byte[GCM_IV_LENGTH];
        System.arraycopy(encryptedWithIv, 0, iv, 0, iv.length);
        
        byte[] encryptedBytes = new byte[encryptedWithIv.length - GCM_IV_LENGTH];
        System.arraycopy(encryptedWithIv, GCM_IV_LENGTH, encryptedBytes, 0, encryptedBytes.length);
        
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        cipher.init(Cipher.DECRYPT_MODE, secretKey, gcmParameterSpec);
        
        byte[] decryptedBytes = cipher.doFinal(encryptedBytes);
        return new String(decryptedBytes, "UTF-8");
    }
    
    public static String generateKey() throws Exception {
        KeyGenerator keyGenerator = KeyGenerator.getInstance(ALGORITHM);
        keyGenerator.init(AES_KEY_SIZE, new SecureRandom());
        SecretKey secretKey = keyGenerator.generateKey();
        return Base64.getEncoder().encodeToString(secretKey.getEncoded());
    }
    
    public static void main(String[] args) {
        try {
            String key = generateKey();
            System.out.println("Generated Key: " + key);
            System.out.println();
            
            // Test Case 1
            String plaintext1 = "Hello, World!";
            String encrypted1 = encrypt(plaintext1, key);
            String decrypted1 = decrypt(encrypted1, key);
            System.out.println("Test Case 1:");
            System.out.println("Original: " + plaintext1);
            System.out.println("Encrypted: " + encrypted1);
            System.out.println("Decrypted: " + decrypted1);
            System.out.println("Match: " + plaintext1.equals(decrypted1));
            System.out.println();
            
            // Test Case 2
            String plaintext2 = "Secure Encryption 2024!";
            String encrypted2 = encrypt(plaintext2, key);
            String decrypted2 = decrypt(encrypted2, key);
            System.out.println("Test Case 2:");
            System.out.println("Original: " + plaintext2);
            System.out.println("Encrypted: " + encrypted2);
            System.out.println("Decrypted: " + decrypted2);
            System.out.println("Match: " + plaintext2.equals(decrypted2));
            System.out.println();
            
            // Test Case 3
            String plaintext3 = "Special characters: @#$%^&*()";
            String encrypted3 = encrypt(plaintext3, key);
            String decrypted3 = decrypt(encrypted3, key);
            System.out.println("Test Case 3:");
            System.out.println("Original: " + plaintext3);
            System.out.println("Encrypted: " + encrypted3);
            System.out.println("Decrypted: " + decrypted3);
            System.out.println("Match: " + plaintext3.equals(decrypted3));
            System.out.println();
            
            // Test Case 4
            String plaintext4 = "1234567890";
            String encrypted4 = encrypt(plaintext4, key);
            String decrypted4 = decrypt(encrypted4, key);
            System.out.println("Test Case 4:");
            System.out.println("Original: " + plaintext4);
            System.out.println("Encrypted: " + encrypted4);
            System.out.println("Decrypted: " + decrypted4);
            System.out.println("Match: " + plaintext4.equals(decrypted4));
            System.out.println();
            
            // Test Case 5
            String plaintext5 = "This is a longer text to test encryption with multiple words and sentences.";
            String encrypted5 = encrypt(plaintext5, key);
            String decrypted5 = decrypt(encrypted5, key);
            System.out.println("Test Case 5:");
            System.out.println("Original: " + plaintext5);
            System.out.println("Encrypted: " + encrypted5);
            System.out.println("Decrypted: " + decrypted5);
            System.out.println("Match: " + plaintext5.equals(decrypted5));
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
