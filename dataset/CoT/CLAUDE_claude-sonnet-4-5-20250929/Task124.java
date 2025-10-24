
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task124 {
    private static final String ALGORITHM = "AES/GCM/NoPadding";
    private static final int KEY_SIZE = 256;
    private static final int GCM_TAG_LENGTH = 128;
    private static final int GCM_IV_LENGTH = 12;
    
    private SecretKey secretKey;
    private Map<String, String> encryptedStorage;
    
    public Task124() throws Exception {
        this.secretKey = generateKey();
        this.encryptedStorage = new HashMap<>();
    }
    
    private SecretKey generateKey() throws Exception {
        KeyGenerator keyGenerator = KeyGenerator.getInstance("AES");
        keyGenerator.init(KEY_SIZE, new SecureRandom());
        return keyGenerator.generateKey();
    }
    
    public String encryptData(String plainText) throws Exception {
        if (plainText == null || plainText.isEmpty()) {
            throw new IllegalArgumentException("Data cannot be null or empty");
        }
        
        byte[] iv = new byte[GCM_IV_LENGTH];
        SecureRandom random = new SecureRandom();
        random.nextBytes(iv);
        
        Cipher cipher = Cipher.getInstance(ALGORITHM);
        GCMParameterSpec parameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, parameterSpec);
        
        byte[] encryptedData = cipher.doFinal(plainText.getBytes("UTF-8"));
        
        byte[] encryptedDataWithIv = new byte[iv.length + encryptedData.length];
        System.arraycopy(iv, 0, encryptedDataWithIv, 0, iv.length);
        System.arraycopy(encryptedData, 0, encryptedDataWithIv, iv.length, encryptedData.length);
        
        return Base64.getEncoder().encodeToString(encryptedDataWithIv);
    }
    
    public String decryptData(String encryptedText) throws Exception {
        if (encryptedText == null || encryptedText.isEmpty()) {
            throw new IllegalArgumentException("Encrypted data cannot be null or empty");
        }
        
        byte[] encryptedDataWithIv = Base64.getDecoder().decode(encryptedText);
        
        byte[] iv = new byte[GCM_IV_LENGTH];
        System.arraycopy(encryptedDataWithIv, 0, iv, 0, iv.length);
        
        byte[] encryptedData = new byte[encryptedDataWithIv.length - GCM_IV_LENGTH];
        System.arraycopy(encryptedDataWithIv, GCM_IV_LENGTH, encryptedData, 0, encryptedData.length);
        
        Cipher cipher = Cipher.getInstance(ALGORITHM);
        GCMParameterSpec parameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        cipher.init(Cipher.DECRYPT_MODE, secretKey, parameterSpec);
        
        byte[] decryptedData = cipher.doFinal(encryptedData);
        return new String(decryptedData, "UTF-8");
    }
    
    public void storeSensitiveData(String key, String data) throws Exception {
        if (key == null || key.isEmpty()) {
            throw new IllegalArgumentException("Key cannot be null or empty");
        }
        String encryptedData = encryptData(data);
        encryptedStorage.put(key, encryptedData);
    }
    
    public String retrieveSensitiveData(String key) throws Exception {
        if (key == null || key.isEmpty()) {
            throw new IllegalArgumentException("Key cannot be null or empty");
        }
        String encryptedData = encryptedStorage.get(key);
        if (encryptedData == null) {
            return null;
        }
        return decryptData(encryptedData);
    }
    
    public static void main(String[] args) {
        try {
            Task124 secureStorage = new Task124();
            
            // Test Case 1: Store and retrieve credit card number
            System.out.println("Test Case 1: Credit Card Storage");
            secureStorage.storeSensitiveData("creditCard1", "4532-1234-5678-9010");
            String retrievedCC = secureStorage.retrieveSensitiveData("creditCard1");
            System.out.println("Retrieved: " + retrievedCC);
            System.out.println();
            
            // Test Case 2: Store and retrieve personal information
            System.out.println("Test Case 2: Personal Information");
            secureStorage.storeSensitiveData("ssn1", "123-45-6789");
            String retrievedSSN = secureStorage.retrieveSensitiveData("ssn1");
            System.out.println("Retrieved: " + retrievedSSN);
            System.out.println();
            
            // Test Case 3: Store and retrieve email and password
            System.out.println("Test Case 3: Email Storage");
            secureStorage.storeSensitiveData("email1", "user@example.com");
            String retrievedEmail = secureStorage.retrieveSensitiveData("email1");
            System.out.println("Retrieved: " + retrievedEmail);
            System.out.println();
            
            // Test Case 4: Store and retrieve medical records
            System.out.println("Test Case 4: Medical Records");
            secureStorage.storeSensitiveData("medical1", "Patient: John Doe, Condition: Confidential");
            String retrievedMedical = secureStorage.retrieveSensitiveData("medical1");
            System.out.println("Retrieved: " + retrievedMedical);
            System.out.println();
            
            // Test Case 5: Retrieve non-existent data
            System.out.println("Test Case 5: Non-existent Key");
            String nonExistent = secureStorage.retrieveSensitiveData("nonExistent");
            System.out.println("Retrieved: " + nonExistent);
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
