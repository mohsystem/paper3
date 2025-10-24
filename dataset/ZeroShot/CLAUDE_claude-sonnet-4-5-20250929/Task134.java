
import java.security.*;
import javax.crypto.*;
import javax.crypto.spec.*;
import java.util.*;
import java.io.*;
import java.nio.charset.StandardCharsets;

public class Task134 {
    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/GCM/NoPadding";
    private static final int KEY_SIZE = 256;
    private static final int GCM_IV_LENGTH = 12;
    private static final int GCM_TAG_LENGTH = 128;
    
    private Map<String, SecretKey> keyStore;
    private SecureRandom secureRandom;
    
    public Task134() {
        this.keyStore = new HashMap<>();
        this.secureRandom = new SecureRandom();
    }
    
    // Generate a new AES key
    public String generateKey(String keyId) {
        try {
            KeyGenerator keyGen = KeyGenerator.getInstance(ALGORITHM);
            keyGen.init(KEY_SIZE, secureRandom);
            SecretKey key = keyGen.generateKey();
            keyStore.put(keyId, key);
            return "Key generated successfully: " + keyId;
        } catch (Exception e) {
            return "Error generating key: " + e.getMessage();
        }
    }
    
    // Encrypt data using a stored key
    public String encrypt(String keyId, String plaintext) {
        try {
            SecretKey key = keyStore.get(keyId);
            if (key == null) {
                return "Key not found: " + keyId;
            }
            
            byte[] iv = new byte[GCM_IV_LENGTH];
            secureRandom.nextBytes(iv);
            
            Cipher cipher = Cipher.getInstance(TRANSFORMATION);
            GCMParameterSpec parameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
            cipher.init(Cipher.ENCRYPT_MODE, key, parameterSpec);
            
            byte[] ciphertext = cipher.doFinal(plaintext.getBytes(StandardCharsets.UTF_8));
            
            // Combine IV and ciphertext
            byte[] combined = new byte[iv.length + ciphertext.length];
            System.arraycopy(iv, 0, combined, 0, iv.length);
            System.arraycopy(ciphertext, 0, combined, iv.length, ciphertext.length);
            
            return Base64.getEncoder().encodeToString(combined);
        } catch (Exception e) {
            return "Encryption error: " + e.getMessage();
        }
    }
    
    // Decrypt data using a stored key
    public String decrypt(String keyId, String encryptedData) {
        try {
            SecretKey key = keyStore.get(keyId);
            if (key == null) {
                return "Key not found: " + keyId;
            }
            
            byte[] combined = Base64.getDecoder().decode(encryptedData);
            
            // Extract IV and ciphertext
            byte[] iv = new byte[GCM_IV_LENGTH];
            byte[] ciphertext = new byte[combined.length - GCM_IV_LENGTH];
            System.arraycopy(combined, 0, iv, 0, iv.length);
            System.arraycopy(combined, GCM_IV_LENGTH, ciphertext, 0, ciphertext.length);
            
            Cipher cipher = Cipher.getInstance(TRANSFORMATION);
            GCMParameterSpec parameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
            cipher.init(Cipher.DECRYPT_MODE, key, parameterSpec);
            
            byte[] plaintext = cipher.doFinal(ciphertext);
            return new String(plaintext, StandardCharsets.UTF_8);
        } catch (Exception e) {
            return "Decryption error: " + e.getMessage();
        }
    }
    
    // Delete a key from the store
    public String deleteKey(String keyId) {
        if (keyStore.containsKey(keyId)) {
            keyStore.remove(keyId);
            return "Key deleted: " + keyId;
        }
        return "Key not found: " + keyId;
    }
    
    // List all key IDs
    public List<String> listKeys() {
        return new ArrayList<>(keyStore.keySet());
    }
    
    public static void main(String[] args) {
        Task134 keyManager = new Task134();
        
        // Test Case 1: Generate a key
        System.out.println("Test 1 - Generate Key:");
        System.out.println(keyManager.generateKey("key1"));
        System.out.println();
        
        // Test Case 2: Encrypt data
        System.out.println("Test 2 - Encrypt Data:");
        String encrypted = keyManager.encrypt("key1", "Hello, Secure World!");
        System.out.println("Encrypted: " + encrypted);
        System.out.println();
        
        // Test Case 3: Decrypt data
        System.out.println("Test 3 - Decrypt Data:");
        String decrypted = keyManager.decrypt("key1", encrypted);
        System.out.println("Decrypted: " + decrypted);
        System.out.println();
        
        // Test Case 4: List all keys
        System.out.println("Test 4 - List Keys:");
        keyManager.generateKey("key2");
        keyManager.generateKey("key3");
        System.out.println("Keys: " + keyManager.listKeys());
        System.out.println();
        
        // Test Case 5: Delete a key
        System.out.println("Test 5 - Delete Key:");
        System.out.println(keyManager.deleteKey("key2"));
        System.out.println("Remaining keys: " + keyManager.listKeys());
    }
}
