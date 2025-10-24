import java.security.*;
import javax.crypto.Cipher;
import java.nio.charset.StandardCharsets;
import java.util.Base64;

public class Task106 {

    /**
     * Generates an RSA key pair with a key size of 2048 bits.
     * @return The generated KeyPair.
     * @throws NoSuchAlgorithmException if the RSA algorithm is not available.
     */
    public static KeyPair generateRsaKeyPair() throws NoSuchAlgorithmException {
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(2048);
        return keyGen.generateKeyPair();
    }

    /**
     * Encrypts a plaintext string using the provided public key.
     * Uses RSA with OAEP padding for security.
     * @param plainText The text to encrypt.
     * @param publicKey The public key to use for encryption.
     * @return The encrypted ciphertext as a byte array.
     * @throws Exception if encryption fails.
     */
    public static byte[] encrypt(String plainText, PublicKey publicKey) throws Exception {
        // Rule #8: Always incorporate OAEP when using the RSA algorithm.
        Cipher cipher = Cipher.getInstance("RSA/ECB/OAEPWithSHA-256AndMGF1Padding");
        cipher.init(Cipher.ENCRYPT_MODE, publicKey);
        return cipher.doFinal(plainText.getBytes(StandardCharsets.UTF_8));
    }

    /**
     * Decrypts a ciphertext using the provided private key.
     * @param cipherText The ciphertext to decrypt.
     * @param privateKey The private key to use for decryption.
     * @return The original decrypted string.
     * @throws Exception if decryption fails.
     */
    public static String decrypt(byte[] cipherText, PrivateKey privateKey) throws Exception {
        Cipher cipher = Cipher.getInstance("RSA/ECB/OAEPWithSHA-256AndMGF1Padding");
        cipher.init(Cipher.DECRYPT_MODE, privateKey);
        byte[] decryptedBytes = cipher.doFinal(cipherText);
        return new String(decryptedBytes, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        try {
            KeyPair keyPair = generateRsaKeyPair();

            String[] testCases = {
                "This is a test message.",
                "RSA with OAEP padding is secure.",
                "Another message for testing purposes.",
                "Short",
                "A much longer message to see how the encryption handles different lengths of input data."
            };

            for (int i = 0; i < testCases.length; i++) {
                String originalMessage = testCases[i];
                System.out.println("--- Test Case " + (i + 1) + " ---");
                System.out.println("Original: " + originalMessage);

                byte[] encrypted = encrypt(originalMessage, keyPair.getPublic());
                System.out.println("Encrypted (Base64): " + Base64.getEncoder().encodeToString(encrypted));

                String decrypted = decrypt(encrypted, keyPair.getPrivate());
                System.out.println("Decrypted: " + decrypted);

                if (originalMessage.equals(decrypted)) {
                    System.out.println("Status: SUCCESS");
                } else {
                    System.out.println("Status: FAILED");
                }
                System.out.println();
            }

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}