
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import java.security.MessageDigest;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task124 {
    private static final String ALGORITHM = "AES";
    private Map<String, String> encryptedData;
    private SecretKey secretKey;
    
    public Task124() throws Exception {
        encryptedData = new HashMap<>();
        // In production, use proper key management system
        secretKey = generateKey();
    }
    
    private SecretKey generateKey() throws Exception {
        KeyGenerator keyGen = KeyGenerator.getInstance(ALGORITHM);
        keyGen.init(128);
        return keyGen.generateKey();
    }
    
    public String encryptData(String data) throws Exception {
        Cipher cipher = Cipher.getInstance(ALGORITHM);
        cipher.init(Cipher.ENCRYPT_MODE, secretKey);
        byte[] encrypted = cipher.doFinal(data.getBytes());
        return Base64.getEncoder().encodeToString(encrypted);
    }
    
    public String decryptData(String encryptedData) throws Exception {
        Cipher cipher = Cipher.getInstance(ALGORITHM);
        cipher.init(Cipher.DECRYPT_MODE, secretKey);
        byte[] decrypted = cipher.doFinal(Base64.getDecoder().decode(encryptedData));
        return new String(decrypted);
    }
    
    public String hashPassword(String password) throws Exception {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        byte[] hash = md.digest(password.getBytes());
        return Base64.getEncoder().encodeToString(hash);
    }
    
    public void storeSecureData(String key, String value) throws Exception {
        String encrypted = encryptData(value);
        encryptedData.put(key, encrypted);
    }
    
    public String retrieveSecureData(String key) throws Exception {
        String encrypted = encryptedData.get(key);
        if (encrypted == null) return null;
        return decryptData(encrypted);
    }
    
    public static void main(String[] args) {
        try {
            Task124 secure = new Task124();
            
            // Test Case 1: Store and retrieve encrypted data
            System.out.println("Test 1: Basic encryption/decryption");
            secure.storeSecureData("user1", "1234-XXXX-XXXX-5678");
            System.out.println("Retrieved: " + secure.retrieveSecureData("user1"));
            
            // Test Case 2: Hash password
            System.out.println("\\nTest 2: Password hashing");
            String hashedPwd = secure.hashPassword("myPassword123");
            System.out.println("Hashed password: " + hashedPwd.substring(0, 20) + "...");
            
            // Test Case 3: Store multiple records
            System.out.println("\\nTest 3: Multiple records");
            secure.storeSecureData("user2", "9876-XXXX-XXXX-5432");
            secure.storeSecureData("user3", "5555-XXXX-XXXX-1111");
            System.out.println("User2: " + secure.retrieveSecureData("user2"));
            System.out.println("User3: " + secure.retrieveSecureData("user3"));
            
            // Test Case 4: Non-existent key
            System.out.println("\\nTest 4: Non-existent key");
            System.out.println("User4: " + secure.retrieveSecureData("user4"));
            
            // Test Case 5: Verify encryption
            System.out.println("\\nTest 5: Verify data is encrypted");
            secure.storeSecureData("test", "SensitiveData123");
            System.out.println("Original stored (encrypted): " + secure.encryptedData.get("test").substring(0, 20) + "...");
            System.out.println("Retrieved (decrypted): " + secure.retrieveSecureData("test"));
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
