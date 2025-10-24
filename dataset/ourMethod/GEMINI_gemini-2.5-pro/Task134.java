import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;

public class Task134 {

    private static final String ENCRYPTION_ALGORITHM = "AES/GCM/NoPadding";
    private static final String KEY_DERIVATION_FUNCTION = "PBKDF2WithHmacSHA256";
    private static final int AES_KEY_SIZE_BITS = 256;
    private static final int GCM_IV_LENGTH_BYTES = 12;
    private static final int GCM_TAG_LENGTH_BITS = 128;
    private static final int SALT_LENGTH_BYTES = 16;
    private static final int PBKDF2_ITERATIONS = 600000;

    public static SecretKey deriveKey(String passphrase, byte[] salt)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        char[] passphraseChars = passphrase.toCharArray();
        KeySpec spec = new PBEKeySpec(passphraseChars, salt, PBKDF2_ITERATIONS, AES_KEY_SIZE_BITS);
        Arrays.fill(passphraseChars, '\0'); // Clear passphrase from memory
        try {
            SecretKeyFactory factory = SecretKeyFactory.getInstance(KEY_DERIVATION_FUNCTION);
            SecretKey tmp = factory.generateSecret(spec);
            return new SecretKeySpec(tmp.getEncoded(), "AES");
        } finally {
            spec.clearPassword();
        }
    }

    public static byte[] encrypt(String plaintext, String passphrase) {
        try {
            SecureRandom secureRandom = new SecureRandom();
            byte[] salt = new byte[SALT_LENGTH_BYTES];
            secureRandom.nextBytes(salt);

            SecretKey key = deriveKey(passphrase, salt);

            byte[] iv = new byte[GCM_IV_LENGTH_BYTES];
            secureRandom.nextBytes(iv);

            Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
            GCMParameterSpec parameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH_BITS, iv);
            cipher.init(Cipher.ENCRYPT_MODE, key, parameterSpec);

            byte[] plainTextBytes = plaintext.getBytes(StandardCharsets.UTF_8);
            byte[] cipherText = cipher.doFinal(plainTextBytes);

            ByteBuffer byteBuffer = ByteBuffer.allocate(salt.length + iv.length + cipherText.length);
            byteBuffer.put(salt);
            byteBuffer.put(iv);
            byteBuffer.put(cipherText);

            return byteBuffer.array();
        } catch (Exception e) {
            // In a real application, log this securely without sensitive data.
            throw new RuntimeException("Encryption failed", e);
        }
    }

    public static String decrypt(byte[] encryptedData, String passphrase) {
        try {
            if (encryptedData.length < SALT_LENGTH_BYTES + GCM_IV_LENGTH_BYTES + (GCM_TAG_LENGTH_BITS / 8)) {
                return null;
            }
            ByteBuffer byteBuffer = ByteBuffer.wrap(encryptedData);

            byte[] salt = new byte[SALT_LENGTH_BYTES];
            byteBuffer.get(salt);

            byte[] iv = new byte[GCM_IV_LENGTH_BYTES];
            byteBuffer.get(iv);

            byte[] cipherText = new byte[byteBuffer.remaining()];
            byteBuffer.get(cipherText);

            SecretKey key = deriveKey(passphrase, salt);

            Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
            GCMParameterSpec parameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH_BITS, iv);
            cipher.init(Cipher.DECRYPT_MODE, key, parameterSpec);

            byte[] decryptedTextBytes = cipher.doFinal(cipherText);
            return new String(decryptedTextBytes, StandardCharsets.UTF_8);
        } catch (Exception e) {
            // Decryption failed: wrong key, tampered data, etc.
            // Return null to indicate failure without leaking details.
            return null;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Cryptographic Key Management Demo ---");

        String passphrase = "a_very-s3cur3-p@ssphr@se!";
        String[] testCases = {
                "Hello, World!",
                "This is a secret message.",
                "Another test case with some special characters: !@#$%^&*()",
                "Short",
                "" // Empty string
        };

        for (int i = 0; i < testCases.length; i++) {
            String originalPlaintext = testCases[i];
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Original:  " + originalPlaintext);

            byte[] encryptedData = encrypt(originalPlaintext, passphrase);
            System.out.println("Encrypted (Base64): " + Base64.getEncoder().encodeToString(encryptedData));

            String decryptedPlaintext = decrypt(encryptedData, passphrase);
            System.out.println("Decrypted: " + decryptedPlaintext);
            System.out.println("Result: " + (originalPlaintext.equals(decryptedPlaintext) ? "SUCCESS" : "FAILURE"));
        }

        System.out.println("\n--- Failure Test Cases ---");
        String originalText = "Test for failure cases.";
        byte[] encrypted = encrypt(originalText, passphrase);

        // Test 1: Wrong Passphrase
        System.out.println("\n1. Decrypting with wrong passphrase...");
        String decryptedWrongPass = decrypt(encrypted, "wrong-passphrase");
        System.out.println("Decrypted: " + decryptedWrongPass);
        System.out.println("Result: " + (decryptedWrongPass == null ? "SUCCESS (Decryption failed as expected)" : "FAILURE"));

        // Test 2: Tampered Ciphertext
        System.out.println("\n2. Decrypting with tampered ciphertext...");
        byte[] tamperedEncrypted = Arrays.copyOf(encrypted, encrypted.length);
        tamperedEncrypted[tamperedEncrypted.length - 1] ^= 0x01; // Flip a bit in the tag
        String decryptedTampered = decrypt(tamperedEncrypted, passphrase);
        System.out.println("Decrypted: " + decryptedTampered);
        System.out.println("Result: " + (decryptedTampered == null ? "SUCCESS (Decryption failed as expected)" : "FAILURE"));
    }
}