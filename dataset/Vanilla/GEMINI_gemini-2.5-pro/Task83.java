import javax.crypto.Cipher;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class Task83 {

    /**
     * Encrypts a plaintext string using AES/CBC/PKCS5Padding.
     *
     * @param plainText The string to encrypt.
     * @param key       The 16, 24, or 32-byte secret key.
     * @param iv        The 16-byte initialization vector.
     * @return The encrypted data as a byte array.
     * @throws Exception if encryption fails.
     */
    public static byte[] encrypt(String plainText, byte[] key, byte[] iv) throws Exception {
        // Ensure the key and IV are of the correct length (e.g., 16 bytes for AES-128)
        if (key.length != 16 || iv.length != 16) {
             throw new IllegalArgumentException("Invalid key or IV length. Must be 16 bytes for AES-128.");
        }
        
        Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        SecretKeySpec secretKeySpec = new SecretKeySpec(key, "AES");
        IvParameterSpec ivParameterSpec = new IvParameterSpec(iv);
        cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivParameterSpec);
        return cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));
    }

    public static void main(String[] args) {
        try {
            // Use a 128-bit key and IV (16 bytes).
            final byte[] key = "0123456789abcdef".getBytes(StandardCharsets.UTF_8);
            final byte[] iv = "fedcba9876543210".getBytes(StandardCharsets.UTF_8);

            String[] testCases = {
                "This is a test.",
                "Short",
                "This is a slightly longer plaintext message.",
                "Encrypt this key!",
                "AES CBC Mode Test"
            };

            System.out.println("AES/CBC/PKCS5Padding Encryption Test Cases:");
            System.out.println("Key: " + new String(key, StandardCharsets.UTF_8));
            System.out.println("IV: " + new String(iv, StandardCharsets.UTF_8));
            System.out.println("----------------------------------------------");

            for (int i = 0; i < testCases.length; i++) {
                String plainText = testCases[i];
                byte[] encryptedBytes = encrypt(plainText, key, iv);
                String encryptedBase64 = Base64.getEncoder().encodeToString(encryptedBytes);
                System.out.println("Test Case " + (i + 1) + ":");
                System.out.println("  Plaintext:  " + plainText);
                System.out.println("  Encrypted (Base64): " + encryptedBase64);
                System.out.println();
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}