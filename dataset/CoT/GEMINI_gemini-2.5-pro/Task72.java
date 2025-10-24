import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.Key;
import java.util.Base64;

public class Task72 {

    // IMPORTANT: Using a static, predictable IV is a major security vulnerability
    // in modes like CBC. The IV should be random and unique for each encryption
    // operation with the same key. This code is for demonstration purposes only,
    // following the prompt's specific requirements.
    private static final byte[] STATIC_IV = "1234567890123456".getBytes(StandardCharsets.UTF_8);

    /**
     * Encrypts data using AES in CBC mode with a static IV.
     * @param plainText The string to encrypt.
     * @param key The secret key for encryption.
     * @return The encrypted data as a byte array, or null on failure.
     */
    public static byte[] encrypt(String plainText, Key key) {
        try {
            // Initialize a cipher using AES in CBC mode with the static IV.
            // PKCS5Padding is used to pad the input data to the block size.
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            IvParameterSpec ivSpec = new IvParameterSpec(STATIC_IV);
            
            // Initialize the cipher for encryption
            cipher.init(Cipher.ENCRYPT_MODE, key, ivSpec);

            // Encrypt the padded data
            byte[] encryptedData = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));
            
            return encryptedData;
        } catch (Exception e) {
            System.err.println("Encryption failed: " + e.getMessage());
            e.printStackTrace();
            return null;
        }
    }

    public static void main(String[] args) {
        // IMPORTANT: Hardcoding keys is a security risk. In a real application,
        // use a secure key management system (e.g., a KeyStore).
        String keyString = "thisisakeyjklmno"; // Must be 16 bytes for AES-128
        Key aesKey = new SecretKeySpec(keyString.getBytes(StandardCharsets.UTF_8), "AES");

        String[] testCases = {
            "This is test case 1.",
            "Another test case with different data.",
            "Short",
            "A much longer string to test padding and multiple blocks of encryption.",
            "Test case 5 is the final one!"
        };
        
        System.out.println("Encrypting using AES/CBC with a STATIC IV (INSECURE PRACTICE)");
        System.out.println("==========================================================");

        for (int i = 0; i < testCases.length; i++) {
            String plainText = testCases[i];
            byte[] encryptedBytes = encrypt(plainText, aesKey);
            
            if (encryptedBytes != null) {
                String encryptedBase64 = Base64.getEncoder().encodeToString(encryptedBytes);
                System.out.println("Test Case " + (i + 1) + ":");
                System.out.println("  Plaintext:  " + plainText);
                System.out.println("  Encrypted (Base64): " + encryptedBase64);
                System.out.println();
            }
        }
    }
}