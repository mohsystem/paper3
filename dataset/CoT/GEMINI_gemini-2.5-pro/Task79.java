import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.security.SecureRandom;
import java.util.Base64;

public class Task79 {

    private static final String ALGORITHM = "AES/GCM/NoPadding";
    private static final int IV_LENGTH_BYTE = 12; // 96 bits is the recommended IV size for GCM
    private static final int TAG_LENGTH_BIT = 128; // 128 bits is a standard tag size

    /**
     * Encrypts a plaintext string using AES-256-GCM.
     * The IV is prepended to the ciphertext.
     *
     * @param plainText The string to encrypt.
     * @param keyBytes A 32-byte (256-bit) key.
     * @return A Base64 encoded string containing the [IV][Ciphertext+Tag].
     * @throws Exception if encryption fails.
     */
    public static String encrypt(String plainText, byte[] keyBytes) throws Exception {
        if (keyBytes.length != 32) {
            throw new IllegalArgumentException("Invalid key size. Key must be 32 bytes for AES-256.");
        }

        // 1. Generate a secure, random IV for each encryption
        byte[] iv = new byte[IV_LENGTH_BYTE];
        SecureRandom random = new SecureRandom();
        random.nextBytes(iv);

        // 2. Create the cipher
        SecretKey secretKey = new SecretKeySpec(keyBytes, "AES");
        Cipher cipher = Cipher.getInstance(ALGORITHM);
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_LENGTH_BIT, iv);
        
        // 3. Encrypt the message
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmParameterSpec);
        byte[] cipherText = cipher.doFinal(plainText.getBytes("UTF-8"));

        // Prepend IV to the ciphertext for use in decryption
        ByteBuffer byteBuffer = ByteBuffer.allocate(iv.length + cipherText.length);
        byteBuffer.put(iv);
        byteBuffer.put(cipherText);
        byte[] cipherMessage = byteBuffer.array();

        return Base64.getEncoder().encodeToString(cipherMessage);
    }

    public static void main(String[] args) {
        try {
            // In a real application, this key should be managed securely (e.g., loaded from a keystore)
            // and not hardcoded or generated on the fly like this for each run.
            SecureRandom random = new SecureRandom();
            byte[] key = new byte[32]; // 256-bit key
            random.nextBytes(key);
            System.out.println("Using Secret Key (Base64): " + Base64.getEncoder().encodeToString(key));
            System.out.println("---");

            String[] testCases = {
                "This is a secret message.",
                "Hello, World!",
                "Cryptography is fascinating.",
                "AES-GCM provides authenticated encryption.",
                "12345!@#$%^"
            };

            for (int i = 0; i < testCases.length; i++) {
                String plainText = testCases[i];
                System.out.println("Test Case " + (i + 1));
                System.out.println("Original:  " + plainText);
                String encryptedText = encrypt(plainText, key);
                System.out.println("Encrypted: " + encryptedText);
                System.out.println();
            }

        } catch (Exception e) {
            // Proper exception handling is critical in security-sensitive code.
            e.printStackTrace();
        }
    }
}