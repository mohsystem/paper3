import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Arrays;

public class Task83 {

    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/CBC/PKCS5Padding";
    private static final int KEY_SIZE = 256; // bits
    private static final int IV_SIZE = 16; // bytes

    /**
     * Creates a Cipher using AES in CBC mode and encrypts the given plaintext.
     * The IV is prepended to the ciphertext.
     *
     * @param plainText The plaintext string to encrypt.
     * @param key       The SecretKey for encryption.
     * @param iv        The IvParameterSpec (Initialization Vector).
     * @return A byte array containing the IV prepended to the ciphertext.
     * @throws Exception if encryption fails.
     */
    public static byte[] encrypt(String plainText, SecretKey key, IvParameterSpec iv) throws Exception {
        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        cipher.init(Cipher.ENCRYPT_MODE, key, iv);
        byte[] cipherText = cipher.doFinal(plainText.getBytes("UTF-8"));

        // Prepend IV to ciphertext for use in decryption
        byte[] encryptedOutput = new byte[iv.getIV().length + cipherText.length];
        System.arraycopy(iv.getIV(), 0, encryptedOutput, 0, iv.getIV().length);
        System.arraycopy(cipherText, 0, encryptedOutput, iv.getIV().length, cipherText.length);

        return encryptedOutput;
    }

    // Helper to print byte arrays as Hex
    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        try {
            // Generate a single, reusable key for the test cases
            KeyGenerator keyGen = KeyGenerator.getInstance(ALGORITHM);
            keyGen.init(KEY_SIZE);
            SecretKey secretKey = keyGen.generateKey();
            System.out.println("Generated AES Key: " + bytesToHex(secretKey.getEncoded()));
            System.out.println("----------------------------------------");


            String[] testCases = {
                "This is a secret message.",
                "The quick brown fox jumps over the lazy dog.",
                "AES encryption is secure!",
                "1234567890",
                "Another test case with some special characters !@#$%^&*()"
            };

            for (int i = 0; i < testCases.length; i++) {
                String plainText = testCases[i];
                System.out.println("Test Case " + (i + 1));
                System.out.println("Plaintext: " + plainText);

                // Generate a new, random IV for each encryption
                byte[] ivBytes = new byte[IV_SIZE];
                new SecureRandom().nextBytes(ivBytes);
                IvParameterSpec ivSpec = new IvParameterSpec(ivBytes);
                System.out.println("IV: " + bytesToHex(ivSpec.getIV()));

                // Encrypt the data
                byte[] encryptedData = encrypt(plainText, secretKey, ivSpec);

                // Print the Base64 encoded result
                String encodedResult = Base64.getEncoder().encodeToString(encryptedData);
                System.out.println("Encrypted (Base64): " + encodedResult);
                System.out.println("----------------------------------------");
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}