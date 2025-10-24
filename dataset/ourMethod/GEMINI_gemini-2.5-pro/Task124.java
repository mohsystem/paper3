import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Base64;
import java.util.Optional;

public class Task124 {

    private static final int SALT_LENGTH_BYTES = 16;
    private static final int IV_LENGTH_BYTES = 12;
    private static final int TAG_LENGTH_BITS = 128;
    private static final int KEY_LENGTH_BITS = 256;
    private static final int PBKDF2_ITERATIONS = 600000;
    private static final String ENCRYPTION_ALGORITHM = "AES/GCM/NoPadding";
    private static final String KEY_DERIVATION_ALGORITHM = "PBKDF2WithHmacSHA256";

    public static byte[] encrypt(String plaintext, String passphrase) {
        if (plaintext == null || plaintext.isEmpty() || passphrase == null || passphrase.isEmpty()) {
            throw new IllegalArgumentException("Plaintext and passphrase must not be empty.");
        }

        try {
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[SALT_LENGTH_BYTES];
            random.nextBytes(salt);

            byte[] iv = new byte[IV_LENGTH_BYTES];
            random.nextBytes(iv);

            SecretKeyFactory factory = SecretKeyFactory.getInstance(KEY_DERIVATION_ALGORITHM);
            KeySpec spec = new PBEKeySpec(passphrase.toCharArray(), salt, PBKDF2_ITERATIONS, KEY_LENGTH_BITS);
            SecretKey tmp = factory.generateSecret(spec);
            SecretKey secretKey = new javax.crypto.spec.SecretKeySpec(tmp.getEncoded(), "AES");

            Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
            GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_LENGTH_BITS, iv);
            cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmParameterSpec);

            byte[] ciphertext = cipher.doFinal(plaintext.getBytes(StandardCharsets.UTF_8));

            ByteBuffer byteBuffer = ByteBuffer.allocate(salt.length + iv.length + ciphertext.length);
            byteBuffer.put(salt);
            byteBuffer.put(iv);
            byteBuffer.put(ciphertext);

            return byteBuffer.array();
        } catch (Exception e) {
            // In a real application, log this securely or handle it appropriately.
            // For this example, re-throwing as a runtime exception.
            throw new RuntimeException("Encryption failed", e);
        }
    }

    public static Optional<String> decrypt(byte[] ciphertextBlob, String passphrase) {
        if (ciphertextBlob == null || ciphertextBlob.length < SALT_LENGTH_BYTES + IV_LENGTH_BYTES + (TAG_LENGTH_BITS / 8)
                || passphrase == null || passphrase.isEmpty()) {
            return Optional.empty();
        }

        try {
            ByteBuffer byteBuffer = ByteBuffer.wrap(ciphertextBlob);

            byte[] salt = new byte[SALT_LENGTH_BYTES];
            byteBuffer.get(salt);

            byte[] iv = new byte[IV_LENGTH_BYTES];
            byteBuffer.get(iv);

            byte[] ciphertext = new byte[byteBuffer.remaining()];
            byteBuffer.get(ciphertext);

            SecretKeyFactory factory = SecretKeyFactory.getInstance(KEY_DERIVATION_ALGORITHM);
            KeySpec spec = new PBEKeySpec(passphrase.toCharArray(), salt, PBKDF2_ITERATIONS, KEY_LENGTH_BITS);
            SecretKey tmp = factory.generateSecret(spec);
            SecretKey secretKey = new javax.crypto.spec.SecretKeySpec(tmp.getEncoded(), "AES");

            Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
            GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_LENGTH_BITS, iv);
            cipher.init(Cipher.DECRYPT_MODE, secretKey, gcmParameterSpec);

            byte[] decryptedText = cipher.doFinal(ciphertext);
            return Optional.of(new String(decryptedText, StandardCharsets.UTF_8));
        } catch (Exception e) {
            // Decryption failed: wrong password, tampered data, etc.
            // Return empty Optional to indicate failure without leaking details.
            return Optional.empty();
        }
    }

    public static void main(String[] args) {
        String[] testPassphrases = {
                "correct-horse-battery-staple",
                "Tr0ub4dor&3",
                "!@#$%^&*()_+",
                "a long passphrase with spaces and punctuation",
                "short"
        };
        String[] testData = {
                "Credit Card: 1234-5678-9012-3456",
                "SSN: 000-00-0000",
                "{\"user\":\"admin\",\"password\":\"password123\"}",
                "This is a secret message.",
                "Another piece of sensitive info."
        };

        for (int i = 0; i < 5; i++) {
            System.out.println("--- Test Case " + (i + 1) + " ---");
            String originalData = testData[i];
            String passphrase = testPassphrases[i];
            System.out.println("Original: " + originalData);
            System.out.println("Passphrase: " + passphrase);

            // Encryption
            byte[] encryptedData = encrypt(originalData, passphrase);
            System.out.println("Encrypted (Base64): " + Base64.getEncoder().encodeToString(encryptedData));

            // Decryption (Success Case)
            Optional<String> decryptedData = decrypt(encryptedData, passphrase);
            decryptedData.ifPresent(s -> System.out.println("Decrypted (Success): " + s));
            if (decryptedData.isPresent() && decryptedData.get().equals(originalData)) {
                System.out.println("SUCCESS: Decrypted data matches original.");
            } else {
                System.out.println("FAILURE: Decrypted data does NOT match original.");
            }

            // Decryption (Failure Case)
            Optional<String> failedDecryption = decrypt(encryptedData, "wrong-password");
            if (failedDecryption.isEmpty()) {
                System.out.println("SUCCESS: Decryption failed as expected with wrong passphrase.");
            } else {
                System.out.println("FAILURE: Decryption succeeded with wrong passphrase.");
            }
            System.out.println();
        }
    }
}