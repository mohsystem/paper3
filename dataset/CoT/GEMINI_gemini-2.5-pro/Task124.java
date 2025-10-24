import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task124 {

    /**
     * A secure in-memory data store for sensitive information.
     * This class encrypts data using AES-GCM with a key derived from a password using PBKDF2.
     * WARNING: This is an example for demonstration purposes. Real-world secure storage
     * requires careful key management, platform-specific protections, and threat modeling.
     * Do not hardcode passwords in production applications.
     */
    static class SecureDataStore {
        // In-memory vault to store encrypted data. Key is an identifier, Value is a Base64 encoded
        // string containing: salt + IV + ciphertext.
        private final Map<String, String> vault = new HashMap<>();

        // Cryptographic parameters - use modern, strong standards.
        private static final String ENCRYPTION_ALGORITHM = "AES/GCM/NoPadding";
        private static final String KEY_DERIVATION_ALGORITHM = "PBKDF2WithHmacSHA256";
        private static final int AES_KEY_SIZE = 256; // bits
        private static final int GCM_IV_LENGTH = 12; // bytes
        private static final int GCM_TAG_LENGTH = 16; // bytes (128 bits)
        private static final int SALT_LENGTH = 16; // bytes
        private static final int PBKDF2_ITERATIONS = 65536; // A good baseline

        private final SecureRandom secureRandom = new SecureRandom();

        /**
         * Derives a secret key from a password and salt using PBKDF2.
         */
        private SecretKey deriveKey(char[] password, byte[] salt) throws NoSuchAlgorithmException, InvalidKeySpecException {
            SecretKeyFactory factory = SecretKeyFactory.getInstance(KEY_DERIVATION_ALGORITHM);
            KeySpec spec = new PBEKeySpec(password, salt, PBKDF2_ITERATIONS, AES_KEY_SIZE);
            SecretKey tmp = factory.generateSecret(spec);
            return new SecretKeySpec(tmp.getEncoded(), "AES");
        }

        /**
         * Encrypts and stores a piece of sensitive data.
         * @param id A unique identifier for the data.
         * @param data The sensitive data to store.
         * @param password The password to protect the data.
         */
        public void storeData(String id, String data, String password) {
            try {
                // 1. Generate a random salt for key derivation.
                byte[] salt = new byte[SALT_LENGTH];
                secureRandom.nextBytes(salt);

                // 2. Derive a strong encryption key from the password and salt.
                SecretKey key = deriveKey(password.toCharArray(), salt);

                // 3. Generate a random Initialization Vector (IV) for AES-GCM.
                byte[] iv = new byte[GCM_IV_LENGTH];
                secureRandom.nextBytes(iv);

                // 4. Encrypt the data.
                Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
                GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
                cipher.init(Cipher.ENCRYPT_MODE, key, gcmParameterSpec);
                byte[] cipherText = cipher.doFinal(data.getBytes(StandardCharsets.UTF_8));

                // 5. Combine salt, IV, and ciphertext into a single byte array for storage.
                ByteBuffer byteBuffer = ByteBuffer.allocate(salt.length + iv.length + cipherText.length);
                byteBuffer.put(salt);
                byteBuffer.put(iv);
                byteBuffer.put(cipherText);
                byte[] combined = byteBuffer.array();
                
                // 6. Store the Base64 encoded result.
                vault.put(id, Base64.getEncoder().encodeToString(combined));

            } catch (Exception e) {
                // In a real application, use a robust logging framework and specific exceptions.
                throw new RuntimeException("Could not store data", e);
            }
        }

        /**
         * Retrieves and decrypts a piece of sensitive data.
         * @param id The identifier of the data to retrieve.
         * @param password The password used to protect the data.
         * @return The decrypted sensitive data, or null if decryption fails (e.g., wrong password).
         */
        public String retrieveData(String id, String password) {
            String encryptedData = vault.get(id);
            if (encryptedData == null) {
                return null;
            }

            try {
                byte[] decoded = Base64.getDecoder().decode(encryptedData);
                ByteBuffer byteBuffer = ByteBuffer.wrap(decoded);

                // 1. Extract salt, IV, and ciphertext from the stored data.
                byte[] salt = new byte[SALT_LENGTH];
                byteBuffer.get(salt);

                byte[] iv = new byte[GCM_IV_LENGTH];
                byteBuffer.get(iv);

                byte[] cipherText = new byte[byteBuffer.remaining()];
                byteBuffer.get(cipherText);

                // 2. Re-derive the key using the same password and the extracted salt.
                SecretKey key = deriveKey(password.toCharArray(), salt);

                // 3. Decrypt the data. AES-GCM automatically handles integrity checking.
                // If the tag doesn't match (e.g., wrong password or tampered data), it will throw an exception.
                Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
                GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
                cipher.init(Cipher.DECRYPT_MODE, key, gcmParameterSpec);
                byte[] plainText = cipher.doFinal(cipherText);
                
                return new String(plainText, StandardCharsets.UTF_8);

            } catch (Exception e) {
                // An exception here usually means a wrong password or corrupted data.
                // Do not expose detailed error messages to the user.
                return null;
            }
        }
    }

    public static void main(String[] args) {
        SecureDataStore store = new SecureDataStore();
        String masterPassword = "aVery!Strong_Password123";

        System.out.println("--- Test Case 1: Storing multiple sensitive data entries ---");
        store.storeData("CreditCard_1", "1111-2222-3333-4444", masterPassword);
        store.storeData("SSN_PersonA", "987-65-4321", masterPassword);
        store.storeData("Email_Password", "S3cr3tP@ssw0rd!", masterPassword);
        store.storeData("API_Key_Prod", "xyz-abc-def-ghi-jkl-mno", masterPassword);
        store.storeData("MedicalRecord_ID", "MRN-555-01-1234", masterPassword);
        System.out.println("5 entries stored successfully.\n");

        System.out.println("--- Test Case 2: Retrieving one entry with the correct password ---");
        String creditCard = store.retrieveData("CreditCard_1", masterPassword);
        System.out.println("Retrieved Credit Card: " + creditCard);
        assert "1111-2222-3333-4444".equals(creditCard);
        System.out.println();

        System.out.println("--- Test Case 3: Attempting to retrieve an entry with the WRONG password ---");
        String failedRetrieval = store.retrieveData("SSN_PersonA", "wrongPassword");
        System.out.println("Result of retrieval with wrong password: " + failedRetrieval);
        assert failedRetrieval == null;
        System.out.println("As expected, retrieval failed (returned null).\n");
        
        System.out.println("--- Test Case 4: Attempting to retrieve a non-existent entry ---");
        String nonExistent = store.retrieveData("NonExistentKey", masterPassword);
        System.out.println("Result of retrieving non-existent key: " + nonExistent);
        assert nonExistent == null;
        System.out.println("As expected, retrieval failed (returned null).\n");
        
        System.out.println("--- Test Case 5: Retrieving all stored entries successfully ---");
        System.out.println("API Key: " + store.retrieveData("API_Key_Prod", masterPassword));
        System.out.println("Email Password: " + store.retrieveData("Email_Password", masterPassword));
        System.out.println("SSN: " + store.retrieveData("SSN_PersonA", masterPassword));
        System.out.println("Medical Record: " + store.retrieveData("MedicalRecord_ID", masterPassword));
        System.out.println("All tests completed.");
    }
}