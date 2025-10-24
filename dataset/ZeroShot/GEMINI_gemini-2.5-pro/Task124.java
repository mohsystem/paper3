import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.Arrays;

/**
 * DISCLAIMER: This is a demonstration of cryptographic concepts.
 * It is NOT a secure system for storing sensitive data in a production environment.
 * Real-world security requires a much more comprehensive approach, including secure
 * key management, which is not possible in this single-file example.
 */
public class Task124 {

    private static final String ENCRYPTION_ALGORITHM = "AES/GCM/NoPadding";
    private static final String KEY_DERIVATION_ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final int AES_KEY_SIZE = 256;
    private static final int GCM_IV_LENGTH = 12; // 96 bits
    private static final int GCM_TAG_LENGTH = 16; // 128 bits
    private static final int SALT_LENGTH = 16;
    private static final int PBKDF2_ITERATIONS = 65536;

    /**
     * Encrypts a piece of data using a password.
     *
     * @param plainText The data to encrypt.
     * @param password  The password to derive the encryption key from.
     * @return A string containing the encrypted data, formatted for storage.
     * @throws Exception if encryption fails.
     */
    public static String encrypt(String plainText, String password) throws Exception {
        // 1. Generate a random salt
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);

        // 2. Derive a key from the password and salt
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, PBKDF2_ITERATIONS, AES_KEY_SIZE);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(KEY_DERIVATION_ALGORITHM);
        SecretKey secretKey = new SecretKeySpec(factory.generateSecret(spec).getEncoded(), "AES");

        // 3. Generate a random Initialization Vector (IV)
        byte[] iv = new byte[GCM_IV_LENGTH];
        random.nextBytes(iv);

        // 4. Encrypt the data
        Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmParameterSpec);
        byte[] cipherText = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));

        // 5. Combine salt, IV, and ciphertext for storage
        ByteBuffer byteBuffer = ByteBuffer.allocate(salt.length + iv.length + cipherText.length);
        byteBuffer.put(salt);
        byteBuffer.put(iv);
        byteBuffer.put(cipherText);
        
        // 6. Base64 encode the result for easy string storage
        return Base64.getEncoder().encodeToString(byteBuffer.array());
    }

    /**
     * Decrypts a piece of data using a password.
     *
     * @param encryptedData The encrypted data string from the encrypt function.
     * @param password      The password used for the original encryption.
     * @return The original plaintext data.
     * @throws Exception if decryption fails (e.g., wrong password, corrupted data).
     */
    public static String decrypt(String encryptedData, String password) throws Exception {
        // 1. Base64 decode and extract salt, IV, and ciphertext
        byte[] decodedBytes = Base64.getDecoder().decode(encryptedData);
        ByteBuffer byteBuffer = ByteBuffer.wrap(decodedBytes);

        byte[] salt = new byte[SALT_LENGTH];
        byteBuffer.get(salt);

        byte[] iv = new byte[GCM_IV_LENGTH];
        byteBuffer.get(iv);

        byte[] cipherText = new byte[byteBuffer.remaining()];
        byteBuffer.get(cipherText);

        // 2. Re-derive the key from the password and salt
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, PBKDF2_ITERATIONS, AES_KEY_SIZE);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(KEY_DERIVATION_ALGORITHM);
        SecretKey secretKey = new SecretKeySpec(factory.generateSecret(spec).getEncoded(), "AES");

        // 3. Decrypt the data
        Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
        cipher.init(Cipher.DECRYPT_MODE, secretKey, gcmParameterSpec);
        
        byte[] decryptedText = cipher.doFinal(cipherText);

        return new String(decryptedText, StandardCharsets.UTF_8);
    }
    
    public static void main(String[] args) {
        System.out.println("Running Java Tests...");
        String password = "a-very-strong-password-!@#$";

        String[] testData = {
            "1234-5678-9012-3456", // Test Case 1: Credit Card
            "{\"name\":\"John Doe\", \"ssn\":\"000-00-0000\"}", // Test Case 2: JSON data
            "My secret diary entry.", // Test Case 3: Simple text
            "", // Test Case 4: Empty string
            "Data with special characters: <>?`|~" // Test Case 5: Special characters
        };

        for (int i = 0; i < testData.length; i++) {
            String originalData = testData[i];
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Original:    " + originalData);
            try {
                // Encryption
                String encrypted = encrypt(originalData, password);
                System.out.println("Encrypted:   " + encrypted);

                // Decryption (Correct Password)
                String decrypted = decrypt(encrypted, password);
                System.out.println("Decrypted:   " + decrypted);
                
                // Verification
                if (!originalData.equals(decrypted)) {
                    System.out.println("FAILURE: Decrypted data does not match original data.");
                } else {
                    System.out.println("SUCCESS: Data integrity verified.");
                }

                // Decryption (Incorrect Password)
                System.out.println("Testing with incorrect password...");
                try {
                    decrypt(encrypted, "wrong-password");
                    System.out.println("FAILURE: Decryption with wrong password should have failed but didn't.");
                } catch (Exception e) {
                    System.out.println("SUCCESS: Decryption failed as expected with wrong password. (" + e.getClass().getSimpleName() + ")");
                }

            } catch (Exception e) {
                System.err.println("An error occurred during test case " + (i + 1) + ": " + e.getMessage());
                e.printStackTrace();
            }
        }
    }
}