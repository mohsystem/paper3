
import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;

public class Task72 {
    // SECURITY WARNING: Static IV is a critical vulnerability in production!
    // This should NEVER be used in real applications.
    // Each encryption should use a unique, randomly generated IV.
    private static final byte[] STATIC_IV = new byte[16]; // All zeros - INSECURE!
    
    static {
        // Initialize static IV (using zeros for demonstration - HIGHLY INSECURE)
        Arrays.fill(STATIC_IV, (byte) 0x00);
    }
    
    /**
     * Encrypts data using AES-CBC with PKCS7 padding
     * WARNING: Uses static IV - NOT SECURE for production use!
     * 
     * @param data The plaintext data to encrypt
     * @param key The 256-bit encryption key (32 bytes)
     * @return Base64 encoded encrypted data
     * @throws Exception if encryption fails
     */
    public static String encryptData(String data, byte[] key) throws Exception {
        if (data == null || data.isEmpty()) {
            throw new IllegalArgumentException("Data cannot be null or empty");
        }
        
        if (key == null || (key.length != 16 && key.length != 24 && key.length != 32)) {
            throw new IllegalArgumentException("Key must be 16, 24, or 32 bytes for AES");
        }
        
        // Step 1: Create static IV (already created as class variable)
        IvParameterSpec ivSpec = new IvParameterSpec(STATIC_IV);
        
        // Step 2: Initialize cipher with AES in CBC mode
        SecretKeySpec secretKey = new SecretKeySpec(key, "AES");
        Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, ivSpec);
        
        // Step 3 & 4: Padding is handled automatically by PKCS5Padding
        // Convert input to bytes and encrypt
        byte[] inputBytes = data.getBytes(StandardCharsets.UTF_8);
        byte[] encryptedBytes = cipher.doFinal(inputBytes);
        
        // Step 5: Return encrypted data as Base64 string
        return Base64.getEncoder().encodeToString(encryptedBytes);
    }
    
    /**
     * Generates a secure random key for AES encryption
     * 
     * @param keySize Key size in bits (128, 192, or 256)
     * @return Random key bytes
     */
    public static byte[] generateSecureKey(int keySize) {
        byte[] key = new byte[keySize / 8];
        new SecureRandom().nextBytes(key);
        return key;
    }
    
    public static void main(String[] args) {
        try {
            System.out.println("=== AES-CBC Encryption Demo ===");
            System.out.println("WARNING: This uses a static IV - NOT SECURE!\\n");
            
            // Generate a secure key for testing
            byte[] key = generateSecureKey(256);
            
            // Test Case 1: Simple text
            String test1 = "Hello, World!";
            String encrypted1 = encryptData(test1, key);
            System.out.println("Test 1 - Input: " + test1);
            System.out.println("Encrypted: " + encrypted1 + "\\n");
            
            // Test Case 2: Longer text
            String test2 = "This is a longer message that will span multiple blocks.";
            String encrypted2 = encryptData(test2, key);
            System.out.println("Test 2 - Input: " + test2);
            System.out.println("Encrypted: " + encrypted2 + "\\n");
            
            // Test Case 3: Special characters
            String test3 = "Special chars: !@#$%^&*()_+-={}[]|:;<>?,./";
            String encrypted3 = encryptData(test3, key);
            System.out.println("Test 3 - Input: " + test3);
            System.out.println("Encrypted: " + encrypted3 + "\\n");
            
            // Test Case 4: Unicode text
            String test4 = "Unicode: 你好世界 🌍";
            String encrypted4 = encryptData(test4, key);
            System.out.println("Test 4 - Input: " + test4);
            System.out.println("Encrypted: " + encrypted4 + "\\n");
            
            // Test Case 5: Demonstrating static IV vulnerability
            String test5a = "Same message";
            String test5b = "Same message";
            String encrypted5a = encryptData(test5a, key);
            String encrypted5b = encryptData(test5b, key);
            System.out.println("Test 5 - Demonstrating Static IV Vulnerability:");
            System.out.println("Message A: " + test5a);
            System.out.println("Encrypted A: " + encrypted5a);
            System.out.println("Message B: " + test5b);
            System.out.println("Encrypted B: " + encrypted5b);
            System.out.println("Same ciphertext? " + encrypted5a.equals(encrypted5b));
            System.out.println("^ This is a CRITICAL security flaw!");
            
        } catch (Exception e) {
            System.err.println("Encryption error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
