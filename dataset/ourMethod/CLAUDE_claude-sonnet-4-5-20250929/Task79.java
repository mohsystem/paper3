
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;

public class Task79 {
    private static final String MAGIC = "ENC1";
    private static final int VERSION = 1;
    private static final int SALT_LENGTH = 16;
    private static final int IV_LENGTH = 12;
    private static final int TAG_LENGTH = 128;
    private static final int ITERATIONS = 210000;
    private static final String ALGORITHM = "AES/GCM/NoPadding";
    private static final String KDF_ALGORITHM = "PBKDF2WithHmacSHA256";

    public static class EncryptionResult {
        public final byte[] encryptedData;
        
        public EncryptionResult(byte[] encryptedData) {
            this.encryptedData = encryptedData;
        }
        
        public String toBase64() {
            return Base64.getEncoder().encodeToString(encryptedData);
        }
    }

    public static EncryptionResult encrypt(String message, String passphrase) throws Exception {
        if (message == null || passphrase == null) {
            throw new IllegalArgumentException("Message and passphrase must not be null");
        }
        
        if (passphrase.length() < 8) {
            throw new IllegalArgumentException("Passphrase must be at least 8 characters");
        }

        SecureRandom secureRandom = new SecureRandom();
        
        byte[] salt = new byte[SALT_LENGTH];
        secureRandom.nextBytes(salt);
        
        byte[] iv = new byte[IV_LENGTH];
        secureRandom.nextBytes(iv);

        SecretKey key = deriveKey(passphrase, salt);
        
        Cipher cipher = Cipher.getInstance(ALGORITHM);
        GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_LENGTH, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, gcmSpec);
        
        byte[] plaintext = message.getBytes(StandardCharsets.UTF_8);
        byte[] ciphertext = cipher.doFinal(plaintext);
        
        byte[] magicBytes = MAGIC.getBytes(StandardCharsets.UTF_8);
        ByteBuffer buffer = ByteBuffer.allocate(
            magicBytes.length + 1 + SALT_LENGTH + IV_LENGTH + ciphertext.length
        );
        buffer.put(magicBytes);
        buffer.put((byte) VERSION);
        buffer.put(salt);
        buffer.put(iv);
        buffer.put(ciphertext);
        
        return new EncryptionResult(buffer.array());
    }

    public static String decrypt(byte[] encryptedData, String passphrase) throws Exception {
        if (encryptedData == null || passphrase == null) {
            throw new IllegalArgumentException("Encrypted data and passphrase must not be null");
        }

        ByteBuffer buffer = ByteBuffer.wrap(encryptedData);
        
        byte[] magicBytes = new byte[4];
        buffer.get(magicBytes);
        String magic = new String(magicBytes, StandardCharsets.UTF_8);
        if (!MAGIC.equals(magic)) {
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
        
        Cipher cipher = Cipher.getInstance(ALGORITHM);
        GCMParameterSpec gcmSpec = new GCMParameterSpec(TAG_LENGTH, iv);
        cipher.init(Cipher.DECRYPT_MODE, key, gcmSpec);
        
        byte[] plaintext = cipher.doFinal(ciphertext);
        
        return new String(plaintext, StandardCharsets.UTF_8);
    }

    private static SecretKey deriveKey(String passphrase, byte[] salt) throws Exception {
        KeySpec spec = new PBEKeySpec(
            passphrase.toCharArray(),
            salt,
            ITERATIONS,
            256
        );
        SecretKeyFactory factory = SecretKeyFactory.getInstance(KDF_ALGORITHM);
        byte[] keyBytes = factory.generateSecret(spec).getEncoded();
        return new SecretKeySpec(keyBytes, "AES");
    }

    public static void main(String[] args) {
        try {
            System.out.println("Test Case 1: Basic encryption/decryption");
            String message1 = "Hello, World!";
            String passphrase1 = "mySecurePassword123";
            EncryptionResult result1 = encrypt(message1, passphrase1);
            String decrypted1 = decrypt(result1.encryptedData, passphrase1);
            System.out.println("Original: " + message1);
            System.out.println("Decrypted: " + decrypted1);
            System.out.println("Match: " + message1.equals(decrypted1));
            System.out.println();

            System.out.println("Test Case 2: Long message");
            String message2 = "This is a much longer message that contains multiple sentences. It should be encrypted and decrypted successfully.";
            String passphrase2 = "anotherSecurePass456";
            EncryptionResult result2 = encrypt(message2, passphrase2);
            String decrypted2 = decrypt(result2.encryptedData, passphrase2);
            System.out.println("Original: " + message2);
            System.out.println("Decrypted: " + decrypted2);
            System.out.println("Match: " + message2.equals(decrypted2));
            System.out.println();

            System.out.println("Test Case 3: Special characters");
            String message3 = "Special chars: @#$%^&*()_+-={}[]|:;<>?,./";
            String passphrase3 = "specialCharsPass789";
            EncryptionResult result3 = encrypt(message3, passphrase3);
            String decrypted3 = decrypt(result3.encryptedData, passphrase3);
            System.out.println("Original: " + message3);
            System.out.println("Decrypted: " + decrypted3);
            System.out.println("Match: " + message3.equals(decrypted3));
            System.out.println();

            System.out.println("Test Case 4: Empty string");
            String message4 = "";
            String passphrase4 = "emptyMessagePass000";
            EncryptionResult result4 = encrypt(message4, passphrase4);
            String decrypted4 = decrypt(result4.encryptedData, passphrase4);
            System.out.println("Original: '" + message4 + "'");
            System.out.println("Decrypted: '" + decrypted4 + "'");
            System.out.println("Match: " + message4.equals(decrypted4));
            System.out.println();

            System.out.println("Test Case 5: Wrong passphrase (should fail)");
            String message5 = "Secret message";
            String passphrase5 = "correctPassphrase";
            String wrongPassphrase = "wrongPassphrase123";
            EncryptionResult result5 = encrypt(message5, passphrase5);
            try {
                String decrypted5 = decrypt(result5.encryptedData, wrongPassphrase);
                System.out.println("ERROR: Should have failed with wrong passphrase");
            } catch (Exception e) {
                System.out.println("Correctly failed with wrong passphrase: " + e.getClass().getSimpleName());
            }

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
