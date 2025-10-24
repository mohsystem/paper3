
import javax.crypto.*;
import javax.crypto.spec.*;
import java.security.*;
import java.security.spec.*;
import java.util.*;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class Task134 {
    private static final String AES_ALGORITHM = "AES/GCM/NoPadding";
    private static final String RSA_ALGORITHM = "RSA";
    private static final int AES_KEY_SIZE = 256;
    private static final int RSA_KEY_SIZE = 2048;
    private static final int GCM_IV_LENGTH = 12;
    private static final int GCM_TAG_LENGTH = 128;
    private static final int PBKDF2_ITERATIONS = 100000;
    
    private Map<String, byte[]> keyStore;
    private SecretKey masterKey;
    
    public Task134(char[] masterPassword) throws Exception {
        this.keyStore = new HashMap<>();
        this.masterKey = deriveMasterKey(masterPassword);
        Arrays.fill(masterPassword, '\\0'); // Clear password from memory
    }
    
    private SecretKey deriveMasterKey(char[] password) throws Exception {
        byte[] salt = new byte[32];
        SecureRandom random = new SecureRandom();
        random.nextBytes(salt);
        
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        KeySpec spec = new PBEKeySpec(password, salt, PBKDF2_ITERATIONS, AES_KEY_SIZE);
        SecretKey tmp = factory.generateSecret(spec);
        return new SecretKeySpec(tmp.getEncoded(), "AES");
    }
    
    public String generateAESKey(String keyId) throws Exception {
        if (keyId == null || keyId.trim().isEmpty()) {
            throw new IllegalArgumentException("Key ID cannot be null or empty");
        }
        
        KeyGenerator keyGen = KeyGenerator.getInstance("AES");
        keyGen.init(AES_KEY_SIZE, new SecureRandom());
        SecretKey key = keyGen.generateKey();
        
        byte[] encryptedKey = encryptKey(key.getEncoded());
        keyStore.put(keyId, encryptedKey);
        
        return keyId;
    }
    
    public String generateRSAKeyPair(String keyId) throws Exception {
        if (keyId == null || keyId.trim().isEmpty()) {
            throw new IllegalArgumentException("Key ID cannot be null or empty");
        }
        
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance(RSA_ALGORITHM);
        keyGen.initialize(RSA_KEY_SIZE, new SecureRandom());
        KeyPair keyPair = keyGen.generateKeyPair();
        
        byte[] privateKeyBytes = keyPair.getPrivate().getEncoded();
        byte[] publicKeyBytes = keyPair.getPublic().getEncoded();
        
        byte[] encryptedPrivateKey = encryptKey(privateKeyBytes);
        byte[] encryptedPublicKey = encryptKey(publicKeyBytes);
        
        keyStore.put(keyId + "_private", encryptedPrivateKey);
        keyStore.put(keyId + "_public", encryptedPublicKey);
        
        return keyId;
    }
    
    private byte[] encryptKey(byte[] keyBytes) throws Exception {
        Cipher cipher = Cipher.getInstance(AES_ALGORITHM);
        byte[] iv = new byte[GCM_IV_LENGTH];
        SecureRandom random = new SecureRandom();
        random.nextBytes(iv);
        
        GCMParameterSpec parameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        cipher.init(Cipher.ENCRYPT_MODE, masterKey, parameterSpec);
        
        byte[] encryptedKey = cipher.doFinal(keyBytes);
        
        byte[] combined = new byte[iv.length + encryptedKey.length];
        System.arraycopy(iv, 0, combined, 0, iv.length);
        System.arraycopy(encryptedKey, 0, combined, iv.length, encryptedKey.length);
        
        return combined;
    }
    
    private byte[] decryptKey(byte[] encryptedData) throws Exception {
        byte[] iv = new byte[GCM_IV_LENGTH];
        byte[] encryptedKey = new byte[encryptedData.length - GCM_IV_LENGTH];
        
        System.arraycopy(encryptedData, 0, iv, 0, iv.length);
        System.arraycopy(encryptedData, iv.length, encryptedKey, 0, encryptedKey.length);
        
        Cipher cipher = Cipher.getInstance(AES_ALGORITHM);
        GCMParameterSpec parameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        cipher.init(Cipher.DECRYPT_MODE, masterKey, parameterSpec);
        
        return cipher.doFinal(encryptedKey);
    }
    
    public boolean deleteKey(String keyId) {
        if (keyId == null) {
            return false;
        }
        return keyStore.remove(keyId) != null;
    }
    
    public boolean keyExists(String keyId) {
        return keyId != null && keyStore.containsKey(keyId);
    }
    
    public Set<String> listKeys() {
        return new HashSet<>(keyStore.keySet());
    }
    
    public void clearAllKeys() {
        keyStore.clear();
    }
    
    public static void main(String[] args) {
        try {
            // Test Case 1: Initialize key manager
            System.out.println("Test Case 1: Initialize Key Manager");
            Task134 keyManager = new Task134("SecurePassword123!@#".toCharArray());
            System.out.println("Key manager initialized successfully");
            System.out.println();
            
            // Test Case 2: Generate AES keys
            System.out.println("Test Case 2: Generate AES Keys");
            String aesKey1 = keyManager.generateAESKey("aes-key-001");
            String aesKey2 = keyManager.generateAESKey("aes-key-002");
            System.out.println("Generated AES keys: " + aesKey1 + ", " + aesKey2);
            System.out.println("Key exists (aes-key-001): " + keyManager.keyExists("aes-key-001"));
            System.out.println();
            
            // Test Case 3: Generate RSA key pairs
            System.out.println("Test Case 3: Generate RSA Key Pairs");
            String rsaKey1 = keyManager.generateRSAKeyPair("rsa-keypair-001");
            System.out.println("Generated RSA key pair: " + rsaKey1);
            System.out.println("Private key exists: " + keyManager.keyExists("rsa-keypair-001_private"));
            System.out.println("Public key exists: " + keyManager.keyExists("rsa-keypair-001_public"));
            System.out.println();
            
            // Test Case 4: List all keys
            System.out.println("Test Case 4: List All Keys");
            Set<String> allKeys = keyManager.listKeys();
            System.out.println("Total keys stored: " + allKeys.size());
            System.out.println("Keys: " + allKeys);
            System.out.println();
            
            // Test Case 5: Delete keys
            System.out.println("Test Case 5: Delete Keys");
            boolean deleted = keyManager.deleteKey("aes-key-001");
            System.out.println("Deleted aes-key-001: " + deleted);
            System.out.println("Key exists after deletion: " + keyManager.keyExists("aes-key-001"));
            System.out.println("Remaining keys: " + keyManager.listKeys().size());
            
            keyManager.clearAllKeys();
            System.out.println("All keys cleared. Remaining keys: " + keyManager.listKeys().size());
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
