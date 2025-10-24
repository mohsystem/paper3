import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class Task72 {

    // 1. Create a static initialization vector (IV). AES block size is 16 bytes.
    private static final byte[] STATIC_IV = "abcdefghijklmnop".getBytes(StandardCharsets.UTF_8);
    // A secret key is also required for AES. Using a 128-bit key (16 bytes).
    private static final byte[] SECRET_KEY = "0123456789abcdef".getBytes(StandardCharsets.UTF_8);

    /**
     * Encrypts data using AES/CBC/PKCS5Padding.
     *
     * @param plainText The string to encrypt.
     * @param key The encryption key (16 bytes for AES-128).
     * @param iv The initialization vector (16 bytes).
     * @return The encrypted data as a byte array.
     * @throws Exception if encryption fails.
     */
    public static byte[] encrypt(String plainText, byte[] key, byte[] iv) throws Exception {
        // 2. Initialize a cipher using AES in CBC mode with the static IV.
        Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        SecretKeySpec secretKeySpec = new SecretKeySpec(key, "AES");
        IvParameterSpec ivParameterSpec = new IvParameterSpec(iv);
        cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivParameterSpec);

        // 3. Pad the input data to match the block size required by the cipher.
        // PKCS5Padding handles this automatically.
        byte[] plainTextBytes = plainText.getBytes(StandardCharsets.UTF_8);

        // 4. Encrypt the padded data using the cipher.
        byte[] encryptedData = cipher.doFinal(plainTextBytes);

        // 5. Return the encrypted data.
        return encryptedData;
    }

    public static void main(String[] args) {
        String[] testCases = {
            "",
            "Hello",
            "16-byte message!", // Exactly 16 bytes, will add a full padding block
            "This is a longer test message for AES encryption.",
            "!@#$%^&*()_+"
        };

        System.out.println("Java AES/CBC Encryption Test Cases:");
        for (int i = 0; i < testCases.length; i++) {
            try {
                byte[] encrypted = encrypt(testCases[i], SECRET_KEY, STATIC_IV);
                System.out.println("Test Case " + (i + 1) + ":");
                System.out.println("  Plaintext:  \"" + testCases[i] + "\"");
                System.out.println("  Encrypted (Base64): " + Base64.getEncoder().encodeToString(encrypted));
                System.out.println();
            } catch (Exception e) {
                System.err.println("Error during encryption for test case " + (i + 1) + ": " + e.getMessage());
            }
        }
    }
}