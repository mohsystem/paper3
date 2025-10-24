import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import java.security.SecureRandom;
import java.util.Base64;
import java.nio.ByteBuffer;

public class Task79 {

    private static final String ENCRYPT_ALGO = "AES/GCM/NoPadding";
    private static final int TAG_LENGTH_BIT = 128; // 128 bits for GCM authentication tag
    private static final int IV_LENGTH_BYTE = 12; // 12 bytes for GCM IV
    private static final int AES_KEY_SIZE = 256; // 256 bits for AES key

    /**
     * Encrypts a message using AES/GCM algorithm.
     * The IV is prepended to the ciphertext.
     *
     * @param message The plaintext message to encrypt.
     * @param secretKey The secret key for encryption.
     * @return A Base64 encoded string containing the IV and ciphertext.
     * @throws Exception if encryption fails.
     */
    public static String encrypt(String message, SecretKey secretKey) throws Exception {
        byte[] plainText = message.getBytes("UTF-8");

        // 1. Create a Cipher for encryption
        Cipher cipher = Cipher.getInstance(ENCRYPT_ALGO);

        // 2. Generate a secure random Initialization Vector (IV)
        byte[] iv = new byte[IV_LENGTH_BYTE];
        new SecureRandom().nextBytes(iv);

        // 3. Initialize the cipher for encryption
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_LENGTH_BIT, iv);
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmParameterSpec);

        // 4. Encrypt the message
        byte[] cipherText = cipher.doFinal(plainText);

        // 5. Concatenate IV and ciphertext
        ByteBuffer byteBuffer = ByteBuffer.allocate(iv.length + cipherText.length);
        byteBuffer.put(iv);
        byteBuffer.put(cipherText);
        byte[] cipherMessage = byteBuffer.array();

        // 6. Encode to Base64 to get a string representation
        return Base64.getEncoder().encodeToString(cipherMessage);
    }

    public static void main(String[] args) {
        try {
            // Generate a new AES secret key
            // In a real application, this key would be securely stored and retrieved.
            KeyGenerator keyGen = KeyGenerator.getInstance("AES");
            keyGen.init(AES_KEY_SIZE, new SecureRandom());
            SecretKey secretKey = keyGen.generateKey();
            
            System.out.println("Generated AES-256 Key (Base64): " + Base64.getEncoder().encodeToString(secretKey.getEncoded()));
            System.out.println("--- Running 5 Test Cases ---");

            String[] testMessages = {
                "This is the first secret message.",
                "Security is important for all applications.",
                "AES-GCM provides authenticated encryption.",
                "1234567890!@#$%^&*()_+",
                "A much longer message to test the encryption process to ensure it works correctly with varying lengths of input data. The quick brown fox jumps over the lazy dog."
            };

            for (int i = 0; i < testMessages.length; i++) {
                String message = testMessages[i];
                System.out.println("\n--- Test Case " + (i + 1) + " ---");
                System.out.println("Original Message: " + message);
                String encryptedMessage = encrypt(message, secretKey);
                System.out.println("Encrypted (Base64): " + encryptedMessage);
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}