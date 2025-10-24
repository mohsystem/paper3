
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.*;
import java.security.spec.MGF1ParameterSpec;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.X509EncodedKeySpec;
import java.util.Arrays;
import javax.crypto.spec.OAEPParameterSpec;
import javax.crypto.spec.PSource;

public class Task106 {
    private static final String RSA_ALGORITHM = "RSA/ECB/OAEPWithSHA-256AndMGF1Padding";
    private static final String AES_ALGORITHM = "AES/GCM/NoPadding";
    private static final int RSA_KEY_SIZE = 2048;
    private static final int AES_KEY_SIZE = 256;
    private static final int GCM_TAG_LENGTH = 128;
    private static final int GCM_IV_LENGTH = 12;
    private static final int SALT_LENGTH = 16;
    private static final byte[] MAGIC = "ENC1".getBytes(StandardCharsets.UTF_8);
    private static final byte VERSION = 1;

    public static class RSAKeyPairHolder {
        public final PublicKey publicKey;
        public final PrivateKey privateKey;

        public RSAKeyPairHolder(PublicKey publicKey, PrivateKey privateKey) {
            this.publicKey = publicKey;
            this.privateKey = privateKey;
        }
    }

    public static RSAKeyPairHolder generateRSAKeyPair() throws NoSuchAlgorithmException {
        KeyPairGenerator keyGen = KeyPairGenerator.getInstance("RSA");
        keyGen.initialize(RSA_KEY_SIZE, new SecureRandom());
        KeyPair pair = keyGen.generateKeyPair();
        return new RSAKeyPairHolder(pair.getPublic(), pair.getPrivate());
    }

    public static byte[] encryptWithRSA(byte[] data, PublicKey publicKey) throws Exception {
        if (data == null || data.length == 0) {
            throw new IllegalArgumentException("Data cannot be null or empty");
        }
        if (publicKey == null) {
            throw new IllegalArgumentException("Public key cannot be null");
        }
        
        Cipher cipher = Cipher.getInstance(RSA_ALGORITHM);
        OAEPParameterSpec oaepParams = new OAEPParameterSpec(
            "SHA-256", "MGF1", MGF1ParameterSpec.SHA256, PSource.PSpecified.DEFAULT
        );
        cipher.init(Cipher.ENCRYPT_MODE, publicKey, oaepParams, new SecureRandom());
        return cipher.doFinal(data);
    }

    public static byte[] decryptWithRSA(byte[] encryptedData, PrivateKey privateKey) throws Exception {
        if (encryptedData == null || encryptedData.length == 0) {
            throw new IllegalArgumentException("Encrypted data cannot be null or empty");
        }
        if (privateKey == null) {
            throw new IllegalArgumentException("Private key cannot be null");
        }
        
        Cipher cipher = Cipher.getInstance(RSA_ALGORITHM);
        OAEPParameterSpec oaepParams = new OAEPParameterSpec(
            "SHA-256", "MGF1", MGF1ParameterSpec.SHA256, PSource.PSpecified.DEFAULT
        );
        cipher.init(Cipher.DECRYPT_MODE, privateKey, oaepParams);
        return cipher.doFinal(encryptedData);
    }

    public static byte[] hybridEncrypt(String plaintext, PublicKey rsaPublicKey) throws Exception {
        if (plaintext == null || plaintext.isEmpty()) {
            throw new IllegalArgumentException("Plaintext cannot be null or empty");
        }
        if (rsaPublicKey == null) {
            throw new IllegalArgumentException("RSA public key cannot be null");
        }

        SecureRandom random = new SecureRandom();
        
        KeyGenerator keyGen = KeyGenerator.getInstance("AES");
        keyGen.init(AES_KEY_SIZE, random);
        SecretKey aesKey = keyGen.generateKey();
        
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        
        byte[] iv = new byte[GCM_IV_LENGTH];
        random.nextBytes(iv);
        
        Cipher aesCipher = Cipher.getInstance(AES_ALGORITHM);
        GCMParameterSpec gcmSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        aesCipher.init(Cipher.ENCRYPT_MODE, aesKey, gcmSpec, random);
        
        byte[] plaintextBytes = plaintext.getBytes(StandardCharsets.UTF_8);
        byte[] ciphertext = aesCipher.doFinal(plaintextBytes);
        
        byte[] encryptedAESKey = encryptWithRSA(aesKey.getEncoded(), rsaPublicKey);
        
        int encryptedKeyLength = encryptedAESKey.length;
        ByteBuffer buffer = ByteBuffer.allocate(
            MAGIC.length + 1 + SALT_LENGTH + GCM_IV_LENGTH + 4 + encryptedKeyLength + ciphertext.length
        );
        
        buffer.put(MAGIC);
        buffer.put(VERSION);
        buffer.put(salt);
        buffer.put(iv);
        buffer.putInt(encryptedKeyLength);
        buffer.put(encryptedAESKey);
        buffer.put(ciphertext);
        
        return buffer.array();
    }

    public static String hybridDecrypt(byte[] encryptedData, PrivateKey rsaPrivateKey) throws Exception {
        if (encryptedData == null || encryptedData.length < MAGIC.length + 1 + SALT_LENGTH + GCM_IV_LENGTH + 4) {
            throw new IllegalArgumentException("Invalid encrypted data format");
        }
        if (rsaPrivateKey == null) {
            throw new IllegalArgumentException("RSA private key cannot be null");
        }

        ByteBuffer buffer = ByteBuffer.wrap(encryptedData);
        
        byte[] magic = new byte[MAGIC.length];
        buffer.get(magic);
        if (!Arrays.equals(magic, MAGIC)) {
            throw new IllegalArgumentException("Invalid magic number");
        }
        
        byte version = buffer.get();
        if (version != VERSION) {
            throw new IllegalArgumentException("Unsupported version");
        }
        
        byte[] salt = new byte[SALT_LENGTH];
        buffer.get(salt);
        
        byte[] iv = new byte[GCM_IV_LENGTH];
        buffer.get(iv);
        
        int encryptedKeyLength = buffer.getInt();
        if (encryptedKeyLength <= 0 || encryptedKeyLength > 512) {
            throw new IllegalArgumentException("Invalid encrypted key length");
        }
        
        byte[] encryptedAESKey = new byte[encryptedKeyLength];
        buffer.get(encryptedAESKey);
        
        byte[] ciphertext = new byte[buffer.remaining()];
        buffer.get(ciphertext);
        
        byte[] aesKeyBytes = decryptWithRSA(encryptedAESKey, rsaPrivateKey);
        SecretKey aesKey = new SecretKeySpec(aesKeyBytes, "AES");
        
        Cipher aesCipher = Cipher.getInstance(AES_ALGORITHM);
        GCMParameterSpec gcmSpec = new GCMParameterSpec(GCM_TAG_LENGTH, iv);
        aesCipher.init(Cipher.DECRYPT_MODE, aesKey, gcmSpec);
        
        byte[] plaintext = aesCipher.doFinal(ciphertext);
        
        return new String(plaintext, StandardCharsets.UTF_8);
    }

    public static void main(String[] args) {
        try {
            System.out.println("RSA Encryption/Decryption with Hybrid Mode Test Cases:\\n");

            RSAKeyPairHolder keyPair = generateRSAKeyPair();

            String[] testCases = {
                "Hello, World!",
                "This is a test message for RSA encryption.",
                "Sensitive data: 123456789",
                "AES-256-GCM with RSA-OAEP provides strong security.",
                "Test case #5 with special characters: !@#$%^&*()"
            };

            for (int i = 0; i < testCases.length; i++) {
                System.out.println("Test Case " + (i + 1) + ":");
                System.out.println("Original: " + testCases[i]);

                byte[] encrypted = hybridEncrypt(testCases[i], keyPair.publicKey);
                System.out.println("Encrypted (length): " + encrypted.length + " bytes");

                String decrypted = hybridDecrypt(encrypted, keyPair.privateKey);
                System.out.println("Decrypted: " + decrypted);
                System.out.println("Match: " + testCases[i].equals(decrypted));
                System.out.println();
            }

        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
