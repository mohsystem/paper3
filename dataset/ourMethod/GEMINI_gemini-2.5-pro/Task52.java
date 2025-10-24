import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;

public class Task52 {

    private static final String ALGORITHM = "AES";
    private static final String CIPHER_TRANSFORMATION = "AES/GCM/NoPadding";
    private static final String KEY_DERIVATION_ALGORITHM = "PBKDF2WithHmacSHA256";
    
    private static final int KEY_SIZE_BITS = 256;
    private static final int SALT_SIZE_BYTES = 16;
    private static final int IV_SIZE_BYTES = 12; // GCM recommended
    private static final int TAG_SIZE_BITS = 128; // GCM recommended
    private static final int PBKDF2_ITERATIONS = 210000;

    private static final byte[] MAGIC_BYTES = "ENCF".getBytes(StandardCharsets.UTF_8);
    private static final byte VERSION = 1;

    public static boolean encryptFile(String inputFilePath, String outputFilePath, String password) throws IOException {
        Path inputPath = Paths.get(inputFilePath);
        Path outputPath = Paths.get(outputFilePath);
        Path tempOutputPath = null;

        try {
            byte[] salt = generateRandomBytes(SALT_SIZE_BYTES);
            byte[] iv = generateRandomBytes(IV_SIZE_BYTES);

            SecretKey secretKey = deriveKey(password, salt);

            Cipher cipher = Cipher.getInstance(CIPHER_TRANSFORMATION);
            GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_SIZE_BITS, iv);
            cipher.init(Cipher.ENCRYPT_MODE, secretKey, gcmParameterSpec);

            byte[] plaintext = Files.readAllBytes(inputPath);
            byte[] ciphertext = cipher.doFinal(plaintext);
            
            // In Java GCM, doFinal appends the tag to the ciphertext. We separate them for a consistent file format.
            int tagIndex = ciphertext.length - (TAG_SIZE_BITS / 8);
            byte[] actualCiphertext = Arrays.copyOfRange(ciphertext, 0, tagIndex);
            byte[] tag = Arrays.copyOfRange(ciphertext, tagIndex, ciphertext.length);


            tempOutputPath = Files.createTempFile(outputPath.getParent(), "enc", ".tmp");
            try (OutputStream os = Files.newOutputStream(tempOutputPath)) {
                os.write(MAGIC_BYTES);
                os.write(VERSION);
                os.write(salt);
                os.write(iv);
                os.write(actualCiphertext);
                os.write(tag);
            }
            
            Files.move(tempOutputPath, outputPath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            return true;
        } catch (Exception e) {
            // System.err.println("Encryption failed: " + e.getMessage());
            if (tempOutputPath != null) {
                Files.deleteIfExists(tempOutputPath);
            }
            return false;
        }
    }
    
    public static boolean decryptFile(String inputFilePath, String outputFilePath, String password) throws IOException {
        Path inputPath = Paths.get(inputFilePath);
        Path outputPath = Paths.get(outputFilePath);
        Path tempOutputPath = null;

        try (InputStream is = Files.newInputStream(inputPath)) {
            byte[] magic = new byte[MAGIC_BYTES.length];
            if (is.read(magic) != magic.length || !Arrays.equals(magic, MAGIC_BYTES)) {
                // System.err.println("Error: Invalid file format or not an encrypted file.");
                return false;
            }

            int version = is.read();
            if (version != VERSION) {
                // System.err.println("Error: Unsupported file version.");
                return false;
            }
            
            byte[] salt = new byte[SALT_SIZE_BYTES];
            if (is.read(salt) != SALT_SIZE_BYTES) throw new IOException("Could not read salt.");
            
            byte[] iv = new byte[IV_SIZE_BYTES];
            if (is.read(iv) != IV_SIZE_BYTES) throw new IOException("Could not read IV.");

            // The rest is ciphertext and tag
            byte[] ciphertextAndTag = is.readAllBytes();
            if (ciphertextAndTag.length < (TAG_SIZE_BITS / 8)) {
                // System.err.println("Error: Corrupted file, missing tag.");
                return false;
            }

            SecretKey secretKey = deriveKey(password, salt);
            
            Cipher cipher = Cipher.getInstance(CIPHER_TRANSFORMATION);
            GCMParameterSpec gcmParameterSpec = new GCMParameterSpec(TAG_SIZE_BITS, iv);
            cipher.init(Cipher.DECRYPT_MODE, secretKey, gcmParameterSpec);

            byte[] plaintext = cipher.doFinal(ciphertextAndTag);

            tempOutputPath = Files.createTempFile(outputPath.getParent(), "dec", ".tmp");
            Files.write(tempOutputPath, plaintext, StandardOpenOption.CREATE, StandardOpenOption.WRITE, StandardOpenOption.TRUNCATE_EXISTING);
            
            Files.move(tempOutputPath, outputPath, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            return true;
        } catch (Exception e) {
            // System.err.println("Decryption failed: " + e.getMessage());
            // This includes AEADBadTagException for wrong password/tampering
            if (tempOutputPath != null) {
                Files.deleteIfExists(tempOutputPath);
            }
            return false;
        }
    }

    private static SecretKey deriveKey(String password, byte[] salt) throws NoSuchAlgorithmException, InvalidKeySpecException {
        PBEKeySpec spec = new PBEKeySpec(password.toCharArray(), salt, PBKDF2_ITERATIONS, KEY_SIZE_BITS);
        SecretKeyFactory skf = SecretKeyFactory.getInstance(KEY_DERIVATION_ALGORITHM);
        SecretKey pbeKey = skf.generateSecret(spec);
        spec.clearPassword();
        return new SecretKeySpec(pbeKey.getEncoded(), ALGORITHM);
    }
    
    private static byte[] generateRandomBytes(int size) {
        byte[] bytes = new byte[size];
        new SecureRandom().nextBytes(bytes);
        return bytes;
    }

    public static void main(String[] args) {
        System.out.println("Running Java tests...");
        String plaintextFile = "plain.txt";
        String encryptedFile = "encrypted.dat";
        String decryptedFile = "decrypted.txt";
        String password = "securePassword123";
        String wrongPassword = "wrongPassword123";

        try {
            // Setup
            Files.write(Paths.get(plaintextFile), "This is a test file for encryption.".getBytes(StandardCharsets.UTF_8));
            
            // Test Case 1: Successful Encryption
            boolean encSuccess = encryptFile(plaintextFile, encryptedFile, password);
            System.out.println("Test Case 1 (Encryption): " + (encSuccess ? "PASSED" : "FAILED"));
            assert encSuccess;

            // Test Case 2: Successful Decryption
            boolean decSuccess = decryptFile(encryptedFile, decryptedFile, password);
            System.out.println("Test Case 2 (Decryption): " + (decSuccess ? "PASSED" : "FAILED"));
            assert decSuccess;

            // Test Case 3: Verify content
            byte[] original = Files.readAllBytes(Paths.get(plaintextFile));
            byte[] decrypted = Files.readAllBytes(Paths.get(decryptedFile));
            boolean contentMatch = Arrays.equals(original, decrypted);
            System.out.println("Test Case 3 (Content verification): " + (contentMatch ? "PASSED" : "FAILED"));
            assert contentMatch;
            
            // Test Case 4: Decryption with wrong password
            boolean wrongPassResult = decryptFile(encryptedFile, decryptedFile, wrongPassword);
            System.out.println("Test Case 4 (Wrong password): " + (!wrongPassResult ? "PASSED" : "FAILED"));
            assert !wrongPassResult;

            // Test Case 5: Decryption of tampered file
            byte[] tamperedData = Files.readAllBytes(Paths.get(encryptedFile));
            tamperedData[tamperedData.length - 20] ^= 0xFF; // Flip a bit in the ciphertext
            Files.write(Paths.get(encryptedFile), tamperedData);
            boolean tamperedResult = decryptFile(encryptedFile, decryptedFile, password);
            System.out.println("Test Case 5 (Tampered file): " + (!tamperedResult ? "PASSED" : "FAILED"));
            assert !tamperedResult;

        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            // Cleanup
            try {
                Files.deleteIfExists(Paths.get(plaintextFile));
                Files.deleteIfExists(Paths.get(encryptedFile));
                Files.deleteIfExists(Paths.get(decryptedFile));
            } catch (IOException e) {
                // Ignore cleanup errors
            }
        }
        System.out.println("Java tests finished.");
    }
}