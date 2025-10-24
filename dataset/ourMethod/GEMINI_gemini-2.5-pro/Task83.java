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
import java.util.Arrays;
import java.util.Base64;

public class Task83 {

    private static final String ALGORITHM = "AES";
    private static final String CIPHER_TRANSFORMATION = "AES/GCM/NoPadding";
    private static final int SALT_LENGTH_BYTES = 16;
    private static final int IV_LENGTH_BYTES = 12;
    private static final int TAG_LENGTH_BITS = 128;
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final int AES_KEY_BITS = 256;

    /**
     * Encrypts plaintext using AES-256-GCM, deriving the key from a password.
     * The security rules mandate using AES-GCM over the requested AES-CBC.
     *
     * @param plaintext The data to encrypt.
     * @param password  The password to derive the encryption key from.
     * @return A byte array containing salt, IV, and ciphertext with authentication tag.
     * @throws Exception if any cryptographic error occurs.
     */
    public static byte[] encrypt(byte[] plaintext, char[] password) throws Exception {
        // 1. Generate a random salt
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH_BYTES];
        random.nextBytes(salt);

        // 2. Derive the key from the password and salt
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        KeySpec spec = new PBEKeySpec(password, salt, PBKDF2_ITERATIONS, AES_KEY_BITS);
        SecretKey tmp = factory.generateSecret(spec);
        SecretKey secretKey = new SecretKeySpec(tmp.getEncoded(), ALGORITHM);

        // 3. Generate a random IV
        byte[] iv = new byte[IV_LENGTH_BYTES];
        random.nextBytes(iv);

        // 4. Encrypt the plaintext
        Cipher cipher = Cipher.getInstance(CIPHER_TRANSFORMATION);
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_LENGTH_BITS, iv);
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmParameterSpec);

        byte[] cipherTextWithTag = cipher.doFinal(plaintext);

        // 5. Concatenate salt, IV, and ciphertext with tag
        ByteBuffer byteBuffer = ByteBuffer.allocate(salt.length + iv.length + cipherTextWithTag.length);
        byteBuffer.put(salt);
        byteBuffer.put(iv);
        byteBuffer.put(cipherTextWithTag);

        return byteBuffer.array();
    }

    public static void main(String[] args) {
        // NOTE: Hardcoding secrets is insecure and violates security rules.
        // This is for demonstration purposes only. In a real application,
        // use a secure mechanism like environment variables or a secrets vault.
        char[] password = "very-strong-password-123".toCharArray();
        String[] testKeysToEncrypt = {
            "0123456789abcdef0123456789abcdef", // 32 bytes hex-like string
            "short key",
            "a-longer-key-with-some-symbols!@#$",
            "", // Empty string
            "another key for testing purposes 12345"
        };

        System.out.println("Running 5 test cases for AES-GCM encryption...");

        for (int i = 0; i < testKeysToEncrypt.length; i++) {
            try {
                System.out.println("\n--- Test Case " + (i + 1) + " ---");
                String originalKey = testKeysToEncrypt[i];
                byte[] plaintext = originalKey.getBytes(StandardCharsets.UTF_8);
                System.out.println("Original data: " + originalKey);
                
                byte[] encryptedData = encrypt(plaintext, password);
                System.out.println("Encrypted (Base64): " + Base64.getEncoder().encodeToString(encryptedData));
                System.out.println("Encrypted data length: " + encryptedData.length + " bytes");

            } catch (Exception e) {
                System.err.println("Test Case " + (i + 1) + " failed: " + e.getMessage());
                e.printStackTrace();
            }
        }
        
        // Securely clear password from memory after use
        Arrays.fill(password, '\0');
    }
}