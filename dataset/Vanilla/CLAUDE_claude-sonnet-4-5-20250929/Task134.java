
import java.util.*;
import java.security.*;
import javax.crypto.*;
import javax.crypto.spec.*;
import java.util.Base64;

public class Task134 {
    private Map<String, String> keyStore;
    private SecureRandom secureRandom;
    
    public Task134() {
        this.keyStore = new HashMap<>();
        this.secureRandom = new SecureRandom();
    }
    
    public String generateKey(String keyId, int keySize) {
        try {
            KeyGenerator keyGen = KeyGenerator.getInstance("AES");
            keyGen.init(keySize, secureRandom);
            SecretKey secretKey = keyGen.generateKey();
            String encodedKey = Base64.getEncoder().encodeToString(secretKey.getEncoded());
            keyStore.put(keyId, encodedKey);
            return encodedKey;
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public String getKey(String keyId) {
        return keyStore.getOrDefault(keyId, "Key not found");
    }
    
    public boolean deleteKey(String keyId) {
        if (keyStore.containsKey(keyId)) {
            keyStore.remove(keyId);
            return true;
        }
        return false;
    }
    
    public List<String> listKeys() {
        return new ArrayList<>(keyStore.keySet());
    }
    
    public String rotateKey(String keyId, int keySize) {
        if (keyStore.containsKey(keyId)) {
            return generateKey(keyId, keySize);
        }
        return "Key not found";
    }
    
    public String encryptData(String keyId, String data) {
        try {
            String encodedKey = keyStore.get(keyId);
            if (encodedKey == null) {
                return "Key not found";
            }
            
            byte[] decodedKey = Base64.getDecoder().decode(encodedKey);
            SecretKey secretKey = new SecretKeySpec(decodedKey, 0, decodedKey.length, "AES");
            
            Cipher cipher = Cipher.getInstance("AES");
            cipher.init(Cipher.ENCRYPT_MODE, secretKey);
            byte[] encryptedData = cipher.doFinal(data.getBytes());
            return Base64.getEncoder().encodeToString(encryptedData);
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public String decryptData(String keyId, String encryptedData) {
        try {
            String encodedKey = keyStore.get(keyId);
            if (encodedKey == null) {
                return "Key not found";
            }
            
            byte[] decodedKey = Base64.getDecoder().decode(encodedKey);
            SecretKey secretKey = new SecretKeySpec(decodedKey, 0, decodedKey.length, "AES");
            
            Cipher cipher = Cipher.getInstance("AES");
            cipher.init(Cipher.DECRYPT_MODE, secretKey);
            byte[] decryptedData = cipher.doFinal(Base64.getDecoder().decode(encryptedData));
            return new String(decryptedData);
        } catch (Exception e) {
            return "Error: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) {
        Task134 keyManager = new Task134();
        
        System.out.println("Test Case 1: Generate and retrieve keys");
        String key1 = keyManager.generateKey("key1", 128);
        System.out.println("Generated key1: " + key1);
        System.out.println("Retrieved key1: " + keyManager.getKey("key1"));
        System.out.println();
        
        System.out.println("Test Case 2: List all keys");
        keyManager.generateKey("key2", 256);
        keyManager.generateKey("key3", 128);
        System.out.println("All keys: " + keyManager.listKeys());
        System.out.println();
        
        System.out.println("Test Case 3: Encrypt and decrypt data");
        String originalData = "Hello, World!";
        String encrypted = keyManager.encryptData("key1", originalData);
        System.out.println("Encrypted: " + encrypted);
        String decrypted = keyManager.decryptData("key1", encrypted);
        System.out.println("Decrypted: " + decrypted);
        System.out.println();
        
        System.out.println("Test Case 4: Rotate key");
        String oldKey = keyManager.getKey("key2");
        System.out.println("Old key2: " + oldKey);
        String newKey = keyManager.rotateKey("key2", 256);
        System.out.println("New key2: " + newKey);
        System.out.println("Keys are different: " + (!oldKey.equals(newKey)));
        System.out.println();
        
        System.out.println("Test Case 5: Delete key");
        System.out.println("Delete key3: " + keyManager.deleteKey("key3"));
        System.out.println("Remaining keys: " + keyManager.listKeys());
        System.out.println("Try to get deleted key: " + keyManager.getKey("key3"));
    }
}
