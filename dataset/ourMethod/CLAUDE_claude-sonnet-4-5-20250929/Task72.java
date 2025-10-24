
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Arrays;

public class Task72 {
    private static final String MAGIC = "ENC1";
    private static final int VERSION = 1;
    private static final int SALT_LENGTH = 16;
    private static final int IV_LENGTH = 12;
    private static final int TAG_LENGTH = 128;
    private static final int ITERATIONS = 210000;

    public static byte[] encryptData(String plaintext, String passphrase) throws Exception {
        if (plaintext == null || passphrase == null || passphrase.isEmpty()) {
            throw new IllegalArgumentException("Invalid input parameters");
        }

        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);

        byte[] iv = new byte[IV_LENGTH];
        random.nextBytes(iv);

        SecretKey key = deriveKey(passphrase, salt);

        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(TAG_LENGTH, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, spec);

        byte[] plaintextBytes = plaintext.getBytes(StandardCharsets.UTF_8);
        byte[] ciphertext = cipher.doFinal(plaintextBytes);

        ByteBuffer buffer = ByteBuffer.allocate(
            MAGIC.length() + 1 + SALT_LENGTH + IV_LENGTH + ciphertext.length
        );
        buffer.put(MAGIC.getBytes(StandardCharsets.UTF_8));
        buffer.put((byte) VERSION);
        buffer.put(salt);
        buffer.put(iv);
        buffer.put(ciphertext);

        return buffer.array();
    }

    public static String decryptData(byte[] encryptedData, String passphrase) throws Exception {
        if (encryptedData == null || passphrase == null || passphrase.isEmpty()) {
            throw new IllegalArgumentException("Invalid input parameters");
        }

        ByteBuffer buffer = ByteBuffer.wrap(encryptedData);

        byte[] magicBytes = new byte[MAGIC.length()];
        buffer.get(magicBytes);
        if (!MAGIC.equals(new String(magicBytes, StandardCharsets.UTF_8))) {
            throw new IllegalArgumentException("Invalid file format");
        }

        int version = buffer.get() & 0xFF;
        if (version != VERSION) {
            throw new IllegalArgumentException("Unsupported version");
        }

        byte[] salt = new byte[SALT_LENGTH];
        buffer.get(salt);

        byte[] iv = new byte[IV_LENGTH];
        buffer.get(iv);

        byte[] ciphertext = new byte[buffer.remaining()];
        buffer.get(ciphertext);

        SecretKey key = deriveKey(passphrase, salt);

        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(TAG_LENGTH, iv);
        cipher.init(Cipher.DECRYPT_MODE, key, spec);

        byte[] plaintext = cipher.doFinal(ciphertext);
        return new String(plaintext, StandardCharsets.UTF_8);
    }

    private static SecretKey deriveKey(String passphrase, byte[] salt) throws Exception {
        javax.crypto.SecretKeyFactory factory = javax.crypto.SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        javax.crypto.spec.PBEKeySpec spec = new javax.crypto.spec.PBEKeySpec(
            passphrase.toCharArray(), salt, ITERATIONS, 256
        );
        byte[] keyBytes = factory.generateSecret(spec).getEncoded();
        spec.clearPassword();
        return new SecretKeySpec(keyBytes, "AES");
    }

    public static void main(String[] args) {
        try {
            String passphrase = "SecurePassphrase123!";

            String test1 = "Hello, World!";
            byte[] encrypted1 = encryptData(test1, passphrase);
            String decrypted1 = decryptData(encrypted1, passphrase);
            System.out.println("Test 1: " + (test1.equals(decrypted1) ? "PASS" : "FAIL"));

            String test2 = "The quick brown fox jumps over the lazy dog";
            byte[] encrypted2 = encryptData(test2, passphrase);
            String decrypted2 = decryptData(encrypted2, passphrase);
            System.out.println("Test 2: " + (test2.equals(decrypted2) ? "PASS" : "FAIL"));

            String test3 = "1234567890";
            byte[] encrypted3 = encryptData(test3, passphrase);
            String decrypted3 = decryptData(encrypted3, passphrase);
            System.out.println("Test 3: " + (test3.equals(decrypted3) ? "PASS" : "FAIL"));

            String test4 = "Special chars: !@#$%^&*()_+-=[]{}|;:',.<>?/`~";
            byte[] encrypted4 = encryptData(test4, passphrase);
            String decrypted4 = decryptData(encrypted4, passphrase);
            System.out.println("Test 4: " + (test4.equals(decrypted4) ? "PASS" : "FAIL"));

            String test5 = "";
            byte[] encrypted5 = encryptData(test5, passphrase);
            String decrypted5 = decryptData(encrypted5, passphrase);
            System.out.println("Test 5: " + (test5.equals(decrypted5) ? "PASS" : "FAIL"));

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
