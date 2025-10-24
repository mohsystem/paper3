
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Base64;

public class Task79 {
    private static final int KEY_LENGTH = 256;
    private static final int ITERATION_COUNT = 65536;
    private static final int SALT_LENGTH = 16;
    private static final int IV_LENGTH = 16;

    public static String encryptMessage(String message, String secretKey) {
        try {
            // Generate random salt
            SecureRandom secureRandom = new SecureRandom();
            byte[] salt = new byte[SALT_LENGTH];
            secureRandom.nextBytes(salt);

            // Derive key from password using PBKDF2
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            KeySpec spec = new PBEKeySpec(secretKey.toCharArray(), salt, ITERATION_COUNT, KEY_LENGTH);
            SecretKey tmp = factory.generateSecret(spec);
            SecretKeySpec secretKeySpec = new SecretKeySpec(tmp.getEncoded(), "AES");

            // Generate random IV
            byte[] iv = new byte[IV_LENGTH];
            secureRandom.nextBytes(iv);
            IvParameterSpec ivSpec = new IvParameterSpec(iv);

            // Create cipher and encrypt
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec, ivSpec);
            byte[] encrypted = cipher.doFinal(message.getBytes("UTF-8"));

            // Combine salt + IV + encrypted data
            byte[] combined = new byte[salt.length + iv.length + encrypted.length];
            System.arraycopy(salt, 0, combined, 0, salt.length);
            System.arraycopy(iv, 0, combined, salt.length, iv.length);
            System.arraycopy(encrypted, 0, combined, salt.length + iv.length, encrypted.length);

            // Encode to Base64
            return Base64.getEncoder().encodeToString(combined);
        } catch (Exception e) {
            throw new RuntimeException("Encryption failed: " + e.getMessage(), e);
        }
    }

    public static void main(String[] args) {
        // Test cases
        String[] messages = {
            "Hello, World!",
            "Secure message 123",
            "This is a confidential document",
            "Test@#$%^&*()",
            "Another secret message"
        };
        
        String[] keys = {
            "MySecretKey123",
            "P@ssw0rd!Strong",
            "EncryptionKey456",
            "SecurePassword789",
            "TopSecret2024"
        };

        System.out.println("=== Encryption Test Cases ===\\n");
        for (int i = 0; i < messages.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Original Message: " + messages[i]);
            System.out.println("Secret Key: " + keys[i]);
            String encrypted = encryptMessage(messages[i], keys[i]);
            System.out.println("Encrypted Message: " + encrypted);
            System.out.println("Encrypted Length: " + encrypted.length() + " characters");
            System.out.println();
        }
    }
}
