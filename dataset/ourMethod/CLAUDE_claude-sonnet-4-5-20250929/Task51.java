
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

public class Task51 {
    private static final String MAGIC = "ENC1";
    private static final int VERSION = 1;
    private static final int SALT_LENGTH = 16;
    private static final int IV_LENGTH = 12;
    private static final int TAG_LENGTH = 128;
    private static final int ITERATIONS = 210000;
    private static final String ALGORITHM = "AES/GCM/NoPadding";
    private static final String KDF_ALGORITHM = "PBKDF2WithHmacSHA256";

    public static String encrypt(String plaintext, String passphrase) throws Exception {
        if (plaintext == null || passphrase == null) {
            throw new IllegalArgumentException("Plaintext and passphrase must not be null");
        }
        if (plaintext.isEmpty() || passphrase.isEmpty()) {
            throw new IllegalArgumentException("Plaintext and passphrase must not be empty");
        }
        if (plaintext.length() > 1048576) {
            throw new IllegalArgumentException("Plaintext too long");
        }

        SecureRandom secureRandom = new SecureRandom();
        
        byte[] salt = new byte[SALT_LENGTH];
        secureRandom.nextBytes(salt);
        
        byte[] iv = new byte[IV_LENGTH];
        secureRandom.nextBytes(iv);
        
        SecretKey key = deriveKey(passphrase, salt);
        
        Cipher cipher = Cipher.getInstance(ALGORITHM);
        GCMParameterSpec parameterSpec = new GCMParameterSpec(TAG_LENGTH, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, parameterSpec);
        
        byte[] plaintextBytes = plaintext.getBytes(StandardCharsets.UTF_8);
        byte[] ciphertext = cipher.doFinal(plaintextBytes);
        
        byte[] magicBytes = MAGIC.getBytes(StandardCharsets.UTF_8);
        ByteBuffer buffer = ByteBuffer.allocate(magicBytes.length + 1 + salt.length + iv.length + ciphertext.length);
        buffer.put(magicBytes);
        buffer.put((byte) VERSION);
        buffer.put(salt);
        buffer.put(iv);
        buffer.put(ciphertext);
        
        return Base64.getEncoder().encodeToString(buffer.array());
    }

    public static String decrypt(String encryptedData, String passphrase) throws Exception {
        if (encryptedData == null || passphrase == null) {
            throw new IllegalArgumentException("Encrypted data and passphrase must not be null");
        }
        if (encryptedData.isEmpty() || passphrase.isEmpty()) {
            throw new IllegalArgumentException("Encrypted data and passphrase must not be empty");
        }

        byte[] data = Base64.getDecoder().decode(encryptedData);
        
        byte[] magicBytes = MAGIC.getBytes(StandardCharsets.UTF_8);
        int minLength = magicBytes.length + 1 + SALT_LENGTH + IV_LENGTH + TAG_LENGTH / 8;
        if (data.length < minLength) {
            throw new IllegalArgumentException("Invalid encrypted data format");
        }
        
        ByteBuffer buffer = ByteBuffer.wrap(data);
        
        byte[] magic = new byte[magicBytes.length];
        buffer.get(magic);
        if (!Arrays.equals(magic, magicBytes)) {
            throw new IllegalArgumentException("Invalid magic bytes");
        }
        
        byte version = buffer.get();
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
        GCMParameterSpec parameterSpec = new GCMParameterSpec(TAG_LENGTH, iv);
        cipher.init(Cipher.DECRYPT_MODE, key, parameterSpec);
        
        byte[] plaintext = cipher.doFinal(ciphertext);
        
        return new String(plaintext, StandardCharsets.UTF_8);
    }

    private static SecretKey deriveKey(String passphrase, byte[] salt) throws Exception {
        KeySpec spec = new PBEKeySpec(passphrase.toCharArray(), salt, ITERATIONS, 256);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(KDF_ALGORITHM);
        byte[] keyBytes = factory.generateSecret(spec).getEncoded();
        return new SecretKeySpec(keyBytes, "AES");
    }

    public static void main(String[] args) {
        try {
            String passphrase = "SecurePassphrase123!";
            
            String test1 = "Hello, World!";
            String encrypted1 = encrypt(test1, passphrase);
            String decrypted1 = decrypt(encrypted1, passphrase);
            System.out.println("Test 1: " + (test1.equals(decrypted1) ? "PASS" : "FAIL"));
            
            String test2 = "Sensitive data 2024";
            String encrypted2 = encrypt(test2, passphrase);
            String decrypted2 = decrypt(encrypted2, passphrase);
            System.out.println("Test 2: " + (test2.equals(decrypted2) ? "PASS" : "FAIL"));
            
            String test3 = "Special chars: !@#$%^&*()";
            String encrypted3 = encrypt(test3, passphrase);
            String decrypted3 = decrypt(encrypted3, passphrase);
            System.out.println("Test 3: " + (test3.equals(decrypted3) ? "PASS" : "FAIL"));
            
            String test4 = "Unicode: 你好世界 🌍";
            String encrypted4 = encrypt(test4, passphrase);
            String decrypted4 = decrypt(encrypted4, passphrase);
            System.out.println("Test 4: " + (test4.equals(decrypted4) ? "PASS" : "FAIL"));
            
            String test5 = "Multiple lines\\nof text\\nfor testing";
            String encrypted5 = encrypt(test5, passphrase);
            String decrypted5 = decrypt(encrypted5, passphrase);
            System.out.println("Test 5: " + (test5.equals(decrypted5) ? "PASS" : "FAIL"));
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
}
