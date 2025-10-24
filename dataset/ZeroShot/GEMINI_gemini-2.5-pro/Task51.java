import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Base64;

public class Task51 {

    // AES-GCM (Galois/Counter Mode) is a modern, authenticated encryption algorithm.
    // It's recommended for its performance and security properties (confidentiality and integrity).
    private static final String ALGORITHM = "AES/GCM/NoPadding";
    private static final int GCM_IV_LENGTH = 12; // 96 bits is the recommended IV size for GCM
    private static final int GCM_TAG_LENGTH = 16; // 128 bits is the recommended tag size for GCM
    private static final int AES_KEY_SIZE = 256; // bits

    /**
     * Encrypts a plaintext string using AES-GCM.
     * The output is a Base64 encoded string containing the IV, followed by the ciphertext and auth tag.
     *
     * @param plaintext The string to encrypt.
     * @param key The secret key for encryption.
     * @return A Base64 encoded string representing the encrypted data (IV + ciphertext).
     * @throws Exception if encryption fails.
     */
    public static String encrypt(String plaintext, SecretKey key) throws Exception {
        // Generate a random, non-repeating IV for each encryption
        byte[] iv = new byte[GCM_IV_LENGTH];
        SecureRandom random = new SecureRandom();
        random.nextBytes(iv);

        Cipher cipher = Cipher.getInstance(ALGORITHM);
        
        // GCMParameterSpec(tagLengthInBits, iv)
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
        
        cipher.init(Cipher.ENCRYPT_MODE, key, gcmParameterSpec);

        byte[] cipherText = cipher.doFinal(plaintext.getBytes(StandardCharsets.UTF_8));

        // Prepend the IV to the ciphertext. This is a standard way to transport the IV,
        // as it is not secret and is required for decryption.
        ByteBuffer byteBuffer = ByteBuffer.allocate(iv.length + cipherText.length);
        byteBuffer.put(iv);
        byteBuffer.put(cipherText);
        
        // Return the result as a Base64 encoded string for safe transport
        return Base64.getEncoder().encodeToString(byteBuffer.array());
    }

    /**
     * Generates a new AES key.
     * In a real application, you must manage keys securely (e.g., using a KeyStore or HSM).
     * @return A SecretKey for AES.
     * @throws Exception if key generation fails.
     */
    public static SecretKey generateKey() throws Exception {
        KeyGenerator keyGenerator = KeyGenerator.getInstance("AES");
        keyGenerator.init(AES_KEY_SIZE);
        return keyGenerator.generateKey();
    }

    public static void main(String[] args) {
        try {
            // IMPORTANT: In a real-world application, the key must be securely
            // managed and not hardcoded or generated on the fly like this.
            // This is for demonstration purposes only.
            SecretKey key = generateKey();
            
            String[] testCases = {
                "This is a secret message.",
                "1234567890",
                "!@#$%^&*()_+",
                "A very long message to test the encryption with a larger payload to see how it performs and handles bigger data chunks.",
                "" // Empty string
            };

            System.out.println("Using AES Key (Base64): " + Base64.getEncoder().encodeToString(key.getEncoded()));
            System.out.println("--- Running 5 Test Cases ---");

            for (int i = 0; i < testCases.length; i++) {
                String plaintext = testCases[i];
                System.out.println("\nTest Case " + (i + 1));
                System.out.println("Plaintext: " + plaintext);
                String encryptedText = encrypt(plaintext, key);
                System.out.println("Encrypted (Base64): " + encryptedText);
            }
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}