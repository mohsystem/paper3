import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class Task83 {

    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/CBC/PKCS5Padding";
    private static final int KEY_SIZE = 256;
    private static final int IV_LENGTH_BYTES = 16; // AES block size is 128 bits

    /**
     * Creates a Cipher using AES/CBC/PKCS5Padding, encrypts the given plaintext,
     * and returns the IV prepended to the ciphertext.
     *
     * @param plainText The string to encrypt.
     * @param key       The SecretKey to use for encryption.
     * @return A byte array containing the IV followed by the ciphertext.
     * @throws Exception if an encryption error occurs.
     */
    public static byte[] encrypt(String plainText, SecretKey key) throws Exception {
        // 1. Generate a cryptographically strong random IV
        byte[] iv = new byte[IV_LENGTH_BYTES];
        SecureRandom random = new SecureRandom();
        random.nextBytes(iv);
        IvParameterSpec ivParameterSpec = new IvParameterSpec(iv);

        // 2. Initialize Cipher in ENCRYPT_MODE
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        cipher.init(Cipher.ENCRYPT_MODE, key, ivParameterSpec);

        // 3. Encrypt the plaintext
        byte[] cipherText = cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));

        // 4. Prepend the IV to the ciphertext for use during decryption
        byte[] encryptedData = new byte[iv.length + cipherText.length];
        System.arraycopy(iv, 0, encryptedData, 0, iv.length);
        System.arraycopy(cipherText, 0, encryptedData, iv.length, cipherText.length);

        return encryptedData;
    }

    /**
     * Main method with 5 test cases.
     */
    public static void main(String[] args) {
        try {
            // Generate a secure AES key. In a real application, this key would be
            // managed securely (e.g., in a keystore).
            KeyGenerator keyGen = KeyGenerator.getInstance(ALGORITHM);
            keyGen.init(KEY_SIZE, new SecureRandom());
            SecretKey secretKey = keyGen.generateKey();

            System.out.println("Using AES Key (Base64): " + Base64.getEncoder().encodeToString(secretKey.getEncoded()));
            System.out.println("--- Running 5 Test Cases ---");

            String[] testCases = {
                "This is the first test case.",
                "Encrypting another piece of data.",
                "AES in CBC mode is secure with a random IV.",
                "A short string.",
                "A much longer string to test padding and block chaining properly, ensuring everything works as expected for various lengths of input data."
            };

            for (int i = 0; i < testCases.length; i++) {
                String plainText = testCases[i];
                System.out.println("\n--- Test Case " + (i + 1) + " ---");
                System.out.println("Plaintext: " + plainText);

                byte[] encryptedData = encrypt(plainText, secretKey);

                System.out.println("Encrypted (Base64): " + Base64.getEncoder().encodeToString(encryptedData));
            }

        } catch (Exception e) {
            // In a real application, use a proper logging framework instead of printing to stderr
            System.err.println("An error occurred during encryption: " + e.getMessage());
            e.printStackTrace();
        }
    }
}