import javax.crypto.Cipher;
import java.security.*;
import java.util.Base64;

/**
 * Note: RSA is typically used to encrypt small amounts of data, such as a
 * symmetric key for another algorithm (like AES). It is not efficient for
 * encrypting large files. The maximum data size is limited by the key size
 * and the padding scheme used.
 */
public class Task106 {

    private static final String ALGORITHM = "RSA";
    // Using OAEP padding is more secure than the older PKCS1Padding
    private static final String PADDING = "RSA/ECB/OAEPWithSHA-256AndMGF1Padding";
    private static final int KEY_SIZE = 2048; // Minimum recommended key size

    /**
     * Generates an RSA KeyPair.
     * @return The generated KeyPair.
     * @throws NoSuchAlgorithmException if the RSA algorithm is not available.
     */
    public static KeyPair generateKeyPair() throws NoSuchAlgorithmException {
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance(ALGORITHM);
        keyGen.initialize(KEY_SIZE);
        return keyGen.generateKeyPair();
    }

    /**
     * Encrypts data using the RSA public key.
     * @param data The plaintext string to encrypt.
     * @param publicKey The public key to use for encryption.
     * @return The encrypted data as a byte array.
     * @throws Exception if encryption fails.
     */
    public static byte[] encrypt(String data, PublicKey publicKey) throws Exception {
        Cipher cipher = Cipher.getInstance(PADDING);
        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        return cipher.doFinal(data.getBytes("UTF-8"));
    }

    /**
     * Decrypts data using the RSA private key.
     * @param data The encrypted byte array.
     * @param privateKey The private key to use for decryption.
     * @return The decrypted plaintext string.
     * @throws Exception if decryption fails.
     */
    public static String decrypt(byte[] data, PrivateKey privateKey) throws Exception {
        Cipher cipher = Cipher.getInstance(PADDING);
        cipher.init(Cipher.DECRYPT_MODE, privateKey);
        return new String(cipher.doFinal(data), "UTF-8");
    }

    public static void main(String[] args) {
        try {
            // 1. Generate Key Pair
            KeyPair keyPair = generateKeyPair();

            String[] testCases = {
                "This is a test message.",
                "RSA is a public-key cryptosystem.",
                "Security is important.",
                "12345!@#$%^&*()",
                "A short message. Real-world applications use RSA to encrypt a symmetric key, not the data itself."
            };

            for (int i = 0; i < testCases.length; i++) {
                System.out.println("--- Test Case " + (i + 1) + " ---");
                String originalData = testCases[i];
                System.out.println("Original: " + originalData);

                // 2. Encrypt using the public key
                byte[] encryptedData = encrypt(originalData, keyPair.getPublic());
                System.out.println("Encrypted (Base64): " + Base64.getEncoder().encodeToString(encryptedData));

                // 3. Decrypt using the private key
                String decryptedData = decrypt(encryptedData, keyPair.getPrivate());
                System.out.println("Decrypted: " + decryptedData);
                System.out.println("Status: " + (originalData.equals(decryptedData) ? "SUCCESS" : "FAILURE"));
                System.out.println();
            }

        } catch (Exception e) {
            System.err.println("An error occurred during the RSA process:");
            e.printStackTrace();
        }
    }
}