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

public class Task79 {

    private static final String ALGORITHM = "AES";
    private static final String CIPHER_TRANSFORMATION = "AES/GCM/NoPadding";
    private static final String KDF_ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final int SALT_LENGTH_BYTES = 16;
    private static final int IV_LENGTH_BYTES = 12;
    private static final int TAG_LENGTH_BITS = 128;
    private static final int KEY_LENGTH_BITS = 256;
    private static final int ITERATION_COUNT = 210000;

    /**
     * Encrypts a plaintext message using a password.
     *
     * @param plaintext The message to encrypt.
     * @param password  The password to derive the key from.
     * @return A byte array containing salt, IV, ciphertext, and tag, or null on failure.
     */
    public static byte[] encrypt(String plaintext, String password) {
        try {
            // 1. Generate a cryptographically secure random salt
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[SALT_LENGTH_BYTES];
            random.nextBytes(salt);

            // 2. Generate a cryptographically secure random IV
            byte[] iv = new byte[IV_LENGTH_BYTES];
            random.nextBytes(iv);

            // 3. Derive the encryption key from the password and salt
            SecretKeyFactory factory = SecretKeyFactory.getInstance(KDF_ALGORITHM);
            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATION_COUNT, KEY_LENGTH_BITS);
            SecretKey tmp = factory.generateSecret(spec);
            SecretKeySpec secretKey = new SecretKeySpec(tmp.getEncoded(), ALGORITHM);

            // 4. Initialize the cipher for encryption
            Cipher cipher = Cipher.getInstance(CIPHER_TRANSFORMATION);
            GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_LENGTH_BITS, iv);
            cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmParameterSpec);

            // 5. Encrypt the plaintext
            byte[] plaintextBytes = plaintext.getBytes(StandardCharsets.UTF_8);
            byte[] cipherText = cipher.doFinal(plaintextBytes);

            // 6. Concatenate salt, IV, and ciphertext with tag
            ByteBuffer byteBuffer = ByteBuffer.allocate(salt.length + iv.length + cipherText.length);
            byteBuffer.put(salt);
            byteBuffer.put(iv);
            byteBuffer.put(cipherText);

            return byteBuffer.array();
        } catch (Exception e) {
            // In a real application, log this error securely.
            // For this example, we print the stack trace and return null.
            e.printStackTrace();
            return null;
        }
    }

    public static void main(String[] args) {
        String password = "aVeryStrongPassword!123";

        String[] testCases = {
                "Hello, World!",
                "This is a secret message.",
                "", // Empty string
                "Short",
                "A much longer message to test the encryption process with more blocks of data to see how it handles it."
        };

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            String plaintext = testCases[i];
            System.out.println("Plaintext: " + plaintext);

            byte[] encryptedData = encrypt(plaintext, password);

            if (encryptedData != null) {
                System.out.println("Encrypted (Base64): " + Base64.getEncoder().encodeToString(encryptedData));
            } else {
                System.out.println("Encryption failed.");
            }
            System.out.println();
        }
    }
}