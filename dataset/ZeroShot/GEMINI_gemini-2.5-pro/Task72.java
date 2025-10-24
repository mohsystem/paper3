import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.util.Base64;
import java.nio.charset.StandardCharsets;

public class Task72 {

    // Static Initialization Vector (IV). For AES, this is 16 bytes.
    // In a real-world application, IV should be random and unique for each encryption.
    private static final byte[] STATIC_IV = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
    };

    /**
     * Encrypts data using AES/CBC/PKCS5Padding with a static IV.
     *
     * @param keyBytes The encryption key (must be 16, 24, or 32 bytes for AES-128, AES-192, or AES-256).
     * @param plainText The data to encrypt.
     * @return The encrypted data as a byte array, or null on error.
     */
    public static byte[] encrypt(byte[] keyBytes, String plainText) {
        if (keyBytes == null || (keyBytes.length != 16 && keyBytes.length != 24 && keyBytes.length != 32)) {
            System.err.println("Error: Invalid key size. Key must be 16, 24, or 32 bytes long.");
            return null;
        }
        try {
            // The input data is not padded manually; PKCS5Padding handles it.
            byte[] plainTextBytes = plainText.getBytes(StandardCharsets.UTF_8);

            // Initialize SecretKeySpec and IvParameterSpec
            SecretKeySpec secretKeySpec = new SecretKeySpec(keyBytes, "AES");
            IvParameterSpec ivParameterSpec = new IvParameterSpec(STATIC_IV);

            // Initialize Cipher for AES/CBC/PKCS5Padding
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivParameterSpec);
            
            // Encrypt the data
            return cipher.doFinal(plainTextBytes);
        } catch (Exception e) {
            // In a real application, more specific exception handling is needed.
            e.printStackTrace();
            return null;
        }
    }

    public static void main(String[] args) {
        // A 16-byte (128-bit) key for demonstration.
        // In a real application, use a secure key management system.
        byte[] key = {
            0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
            (byte)0x88, (byte)0x99, (byte)0xAA, (byte)0xBB, (byte)0xCC, (byte)0xDD, (byte)0xEE, (byte)0xFF
        };

        String[] testCases = {
            "Hello, World!",
            "16-byte message!", // Exactly one block
            "This is a longer test message for AES CBC encryption.",
            "", // Empty string
            "123!@#$%^&*()"
        };
        
        System.out.println("Java AES/CBC Encryption with Static IV");
        System.out.println("---------------------------------------");
        
        int i = 1;
        for (String testCase : testCases) {
            System.out.println("Test Case #" + (i++));
            System.out.println("Plaintext: " + testCase);
            byte[] encryptedData = encrypt(key, testCase);
            if (encryptedData != null) {
                System.out.println("Encrypted (Base64): " + Base64.getEncoder().encodeToString(encryptedData));
            } else {
                System.out.println("Encryption failed.");
            }
            System.out.println();
        }
    }
}