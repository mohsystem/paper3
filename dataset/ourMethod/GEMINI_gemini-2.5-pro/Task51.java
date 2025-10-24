import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;

public class Task51 {

    private static final String ENCRYPTION_ALGORITHM = "AES/GCM/NoPadding";
    private static final String KDF_ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final int SALT_LENGTH_BYTES = 16;
    private static final int IV_LENGTH_BYTES = 12;
    private static final int TAG_LENGTH_BITS = 128;
    private static final int KEY_LENGTH_BITS = 256;
    private static final int PBKDF2_ITERATIONS = 600000;
    private static final byte[] MAGIC_BYTES = "ENC1".getBytes(StandardCharsets.UTF_8);
    private static final byte VERSION = 0x01;

    public static String encrypt(String plaintext, String password) throws Exception {
        SecureRandom random = new SecureRandom();

        // 1. Generate Salt
        byte[] salt = new byte[SALT_LENGTH_BYTES];
        random.nextBytes(salt);

        // 2. Derive Key
        SecretKeyFactory factory = SecretKeyFactory.getInstance(KDF_ALGORITHM);
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, PBKDF2_ITERATIONS, KEY_LENGTH_BITS);
        SecretKey tmp = factory.generateSecret(spec);
        SecretKey secretKey = new javax.crypto.spec.SecretKeySpec(tmp.getEncoded(), "AES");

        // 3. Generate IV
        byte[] iv = new byte[IV_LENGTH_BYTES];
        random.nextBytes(iv);

        // 4. Encrypt
        Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_LENGTH_BITS, iv);
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmParameterSpec);
        byte[] ciphertext = cipher.doFinal(plaintext.getBytes(StandardCharsets.UTF_8));

        // 5. Assemble Output: [magic][version][salt][iv][ciphertext+tag]
        ByteBuffer byteBuffer = ByteBuffer.allocate(MAGIC_BYTES.length + 1 + salt.length + iv.length + ciphertext.length);
        byteBuffer.put(MAGIC_BYTES);
        byteBuffer.put(VERSION);
        byteBuffer.put(salt);
        byteBuffer.put(iv);
        byteBuffer.put(ciphertext);

        return Base64.getEncoder().encodeToString(byteBuffer.array());
    }

    public static String decrypt(String base64Ciphertext, String password) throws Exception {
        byte[] decoded = Base64.getDecoder().decode(base64Ciphertext);
        ByteBuffer byteBuffer = ByteBuffer.wrap(decoded);

        // 1. Parse Input
        byte[] magic = new byte[MAGIC_BYTES.length];
        byteBuffer.get(magic);
        if (!Arrays.equals(MAGIC_BYTES, magic)) {
            throw new IllegalArgumentException("Invalid file format: magic bytes mismatch.");
        }

        byte version = byteBuffer.get();
        if (version != VERSION) {
            throw new IllegalArgumentException("Unsupported version.");
        }

        byte[] salt = new byte[SALT_LENGTH_BYTES];
        byteBuffer.get(salt);

        byte[] iv = new byte[IV_LENGTH_BYTES];
        byteBuffer.get(iv);

        byte[] ciphertext = new byte[byteBuffer.remaining()];
        byteBuffer.get(ciphertext);

        // 2. Derive Key
        SecretKeyFactory factory = SecretKeyFactory.getInstance(KDF_ALGORITHM);
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, PBKDF2_ITERATIONS, KEY_LENGTH_BITS);
        SecretKey tmp = factory.generateSecret(spec);
        SecretKey secretKey = new javax.crypto.spec.SecretKeySpec(tmp.getEncoded(), "AES");

        // 3. Decrypt
        Cipher cipher = Cipher.getInstance(ENCRYPTION_ALGORITHM);
        GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_LENGTH_BITS, iv);
        cipher.init(Cipher.DECRYPT_MODE, secretKey, gcmParameterSpec);
        byte[] plaintext = cipher.doFinal(ciphertext);

        return new String(plaintext, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        String password = "a_very_strong_password_!@#$"; // In a real app, use a secure way to get the password
        String[] testCases = {
                "Hello, World!",
                "",
                "This is a longer test sentence to see how it performs with more data.",
                "12345!@#$%^&*()_+-=`~",
                "こんにちは世界" // Unicode test
        };

        System.out.println("Running 5 test cases...");

        for (int i = 0; i < testCases.length; i++) {
            String originalPlaintext = testCases[i];
            System.out.println("\n--- Test Case " + (i + 1) + " ---");
            System.out.println("Original:    " + originalPlaintext);
            try {
                String encrypted = encrypt(originalPlaintext, password);
                System.out.println("Encrypted:   " + encrypted.substring(0, Math.min(encrypted.length(), 60)) + "...");

                String decrypted = decrypt(encrypted, password);
                System.out.println("Decrypted:   " + decrypted);

                if (originalPlaintext.equals(decrypted)) {
                    System.out.println("Status:      SUCCESS");
                } else {
                    System.out.println("Status:      FAILURE");
                }
            } catch (Exception e) {
                System.err.println("An error occurred during test case " + (i + 1) + ": " + e.getMessage());
                e.printStackTrace();
            }
        }
    }
}