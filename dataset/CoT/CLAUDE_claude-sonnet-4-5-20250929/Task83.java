
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Arrays;

public class Task83 {
    private static final String ALGORITHM = "AES";
    private static final String TRANSFORMATION = "AES/CBC/PKCS5Padding";
    private static final int KEY_SIZE = 256;
    private static final int IV_SIZE = 16;

    public static class EncryptionResult {
        private final byte[] encryptedData;
        private final byte[] iv;

        public EncryptionResult(byte[] encryptedData, byte[] iv) {
            this.encryptedData = encryptedData;
            this.iv = iv;
        }

        public byte[] getEncryptedData() {
            return encryptedData;
        }

        public byte[] getIv() {
            return iv;
        }

        public String toBase64() {
            return Base64.getEncoder().encodeToString(encryptedData);
        }

        public String getIvBase64() {
            return Base64.getEncoder().encodeToString(iv);
        }
    }

    public static EncryptionResult encryptData(byte[] plaintext, byte[] keyBytes) throws Exception {
        if (plaintext == null || plaintext.length == 0) {
            throw new IllegalArgumentException("Plaintext cannot be null or empty");
        }
        if (keyBytes == null || (keyBytes.length != 16 && keyBytes.length != 24 && keyBytes.length != 32)) {
            throw new IllegalArgumentException("Key must be 16, 24, or 32 bytes");
        }

        // Generate random IV using SecureRandom
        byte[] iv = new byte[IV_SIZE];
        SecureRandom secureRandom = new SecureRandom();
        secureRandom.nextBytes(iv);

        SecretKeySpec secretKey = new SecretKeySpec(keyBytes, ALGORITHM);
        IvParameterSpec ivSpec = new IvParameterSpec(iv);

        Cipher cipher = Cipher.getInstance(TRANSFORMATION);
        cipher.init(Cipher.ENCRYPT_MODE, secretKey, ivSpec);

        byte[] encryptedData = cipher.doFinal(plaintext);

        return new EncryptionResult(encryptedData, iv);
    }

    public static byte[] generateSecureKey(int keySize) throws Exception {
        KeyGenerator keyGen = KeyGenerator.getInstance(ALGORITHM);
        keyGen.init(keySize, new SecureRandom());
        SecretKey secretKey = keyGen.generateKey();
        return secretKey.getEncoded();
    }

    public static void main(String[] args) {
        try {
            System.out.println("AES CBC Encryption Test Cases\\n");

            // Test Case 1: Standard encryption with 256-bit key
            System.out.println("Test Case 1: 256-bit key encryption");
            byte[] key1 = generateSecureKey(256);
            String plaintext1 = "Hello, Secure World!";
            EncryptionResult result1 = encryptData(plaintext1.getBytes("UTF-8"), key1);
            System.out.println("Plaintext: " + plaintext1);
            System.out.println("Encrypted (Base64): " + result1.toBase64());
            System.out.println("IV (Base64): " + result1.getIvBase64());
            System.out.println();

            // Test Case 2: Encryption with 128-bit key
            System.out.println("Test Case 2: 128-bit key encryption");
            byte[] key2 = generateSecureKey(128);
            String plaintext2 = "Sensitive Data 123";
            EncryptionResult result2 = encryptData(plaintext2.getBytes("UTF-8"), key2);
            System.out.println("Plaintext: " + plaintext2);
            System.out.println("Encrypted (Base64): " + result2.toBase64());
            System.out.println("IV (Base64): " + result2.getIvBase64());
            System.out.println();

            // Test Case 3: Large data encryption
            System.out.println("Test Case 3: Large data encryption");
            byte[] key3 = generateSecureKey(256);
            String plaintext3 = "This is a much longer message that contains multiple sentences. " +
                               "It demonstrates the encryption of larger data blocks using AES CBC mode.";
            EncryptionResult result3 = encryptData(plaintext3.getBytes("UTF-8"), key3);
            System.out.println("Plaintext length: " + plaintext3.length() + " bytes");
            System.out.println("Encrypted (Base64): " + result3.toBase64());
            System.out.println("IV (Base64): " + result3.getIvBase64());
            System.out.println();

            // Test Case 4: Special characters encryption
            System.out.println("Test Case 4: Special characters encryption");
            byte[] key4 = generateSecureKey(192);
            String plaintext4 = "Special chars: @#$%^&*()_+-={}[]|:;<>?,./~`";
            EncryptionResult result4 = encryptData(plaintext4.getBytes("UTF-8"), key4);
            System.out.println("Plaintext: " + plaintext4);
            System.out.println("Encrypted (Base64): " + result4.toBase64());
            System.out.println("IV (Base64): " + result4.getIvBase64());
            System.out.println();

            // Test Case 5: Verify different IVs produce different ciphertexts
            System.out.println("Test Case 5: Same plaintext, different IVs");
            byte[] key5 = generateSecureKey(256);
            String plaintext5 = "Same message";
            EncryptionResult result5a = encryptData(plaintext5.getBytes("UTF-8"), key5);
            EncryptionResult result5b = encryptData(plaintext5.getBytes("UTF-8"), key5);
            System.out.println("Plaintext: " + plaintext5);
            System.out.println("Encrypted 1 (Base64): " + result5a.toBase64());
            System.out.println("Encrypted 2 (Base64): " + result5b.toBase64());
            System.out.println("Are ciphertexts different? " + 
                             !Arrays.equals(result5a.getEncryptedData(), result5b.getEncryptedData()));

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
