import javax.crypto.Cipher;
import java.security.*;
import java.util.Base64;

public class Task106 {

    // Generates a 2048-bit RSA key pair
    public static KeyPair generateRsaKeyPair() throws NoSuchAlgorithmException {
        KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("RSA");
        keyPairGenerator.initialize(2048);
        return keyPairGenerator.generateKeyPair();
    }

    // Encrypts data using the RSA public key
    public static byte[] encrypt(String plainText, PublicKey publicKey) throws Exception {
        // Use OAEP padding for security, it's more secure than PKCS1Padding
        Cipher encryptCipher = Cipher.getInstance("RSA/ECB/OAEPWithSHA-256AndMGF1Padding");
        encryptCipher.init(Cipher.ENCRYPT_MODE, publicKey);
        return encryptCipher.doFinal(plainText.getBytes());
    }

    // Decrypts data using the RSA private key
    public static String decrypt(byte[] cipherText, PrivateKey privateKey) throws Exception {
        Cipher decryptCipher = Cipher.getInstance("RSA/ECB/OAEPWithSHA-256AndMGF1Padding");
        decryptCipher.init(Cipher.DECRYPT_MODE, privateKey);
        byte[] decryptedBytes = decryptCipher.doFinal(cipherText);
        return new String(decryptedBytes);
    }

    public static void main(String[] args) {
        String[] testCases = {
            "This is a secret message.",
            "RSA is a public-key cryptosystem.",
            "Security is important.",
            "Test case 4 with numbers 12345.",
            "Final test case with symbols !@#$%"
        };

        System.out.println("--- Java RSA Encryption/Decryption ---");

        for (int i = 0; i < testCases.length; i++) {
            String originalMessage = testCases[i];
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Original Message: " + originalMessage);

            try {
                // 1. Generate Key Pair
                KeyPair keyPair = generateRsaKeyPair();
                PublicKey publicKey = keyPair.getPublic();
                PrivateKey privateKey = keyPair.getPrivate();

                // 2. Encrypt the message
                byte[] encryptedMessage = encrypt(originalMessage, publicKey);
                String encodedMessage = Base64.getEncoder().encodeToString(encryptedMessage);
                System.out.println("Encrypted (Base64): " + encodedMessage);

                // 3. Decrypt the message
                String decryptedMessage = decrypt(encryptedMessage, privateKey);
                System.out.println("Decrypted Message: " + decryptedMessage);
                
                // Verification
                if (!originalMessage.equals(decryptedMessage)) {
                    System.out.println("Error: Decrypted message does not match original.");
                }

            } catch (Exception e) {
                System.err.println("An error occurred during cryptographic operations:");
                e.printStackTrace();
            }
        }
    }
}