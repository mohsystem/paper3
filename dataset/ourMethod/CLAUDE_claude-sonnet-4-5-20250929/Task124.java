
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.PosixFilePermission;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

public class Task124 {
    private static final String MAGIC = "ENC1";
    private static final int VERSION = 1;
    private static final int SALT_LENGTH = 16;
    private static final int IV_LENGTH = 12;
    private static final int TAG_LENGTH = 16;
    private static final int ITERATIONS = 210000;
    private static final String CIPHER_ALGORITHM = "AES/GCM/NoPadding";
    
    public static byte[] encryptData(String plaintext, char[] passphrase) throws Exception {
        if (plaintext == null || plaintext.isEmpty()) {
            throw new IllegalArgumentException("Plaintext cannot be null or empty");
        }
        if (passphrase == null || passphrase.length == 0) {
            throw new IllegalArgumentException("Passphrase cannot be null or empty");
        }
        
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        
        byte[] iv = new byte[IV_LENGTH];
        random.nextBytes(iv);
        
        SecretKey key = deriveKey(passphrase, salt);
        
        Cipher cipher = Cipher.getInstance(CIPHER_ALGORITHM);
        GCMParameterSpec spec = new GCMParameterSpec(TAG_LENGTH * 8, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, spec);
        
        byte[] plaintextBytes = plaintext.getBytes(StandardCharsets.UTF_8);
        byte[] ciphertextWithTag = cipher.doFinal(plaintextBytes);
        
        ByteBuffer buffer = ByteBuffer.allocate(
            MAGIC.length() + 4 + SALT_LENGTH + IV_LENGTH + ciphertextWithTag.length
        );
        buffer.put(MAGIC.getBytes(StandardCharsets.UTF_8));
        buffer.putInt(VERSION);
        buffer.put(salt);
        buffer.put(iv);
        buffer.put(ciphertextWithTag);
        
        Arrays.fill(plaintextBytes, (byte) 0);
        
        return buffer.array();
    }
    
    public static String decryptData(byte[] encryptedData, char[] passphrase) throws Exception {
        if (encryptedData == null || encryptedData.length < MAGIC.length() + 4 + SALT_LENGTH + IV_LENGTH + TAG_LENGTH) {
            throw new IllegalArgumentException("Invalid encrypted data");
        }
        if (passphrase == null || passphrase.length == 0) {
            throw new IllegalArgumentException("Passphrase cannot be null or empty");
        }
        
        ByteBuffer buffer = ByteBuffer.wrap(encryptedData);
        
        byte[] magic = new byte[MAGIC.length()];
        buffer.get(magic);
        if (!Arrays.equals(magic, MAGIC.getBytes(StandardCharsets.UTF_8))) {
            throw new IllegalArgumentException("Invalid file format: magic mismatch");
        }
        
        int version = buffer.getInt();
        if (version != VERSION) {
            throw new IllegalArgumentException("Unsupported version: " + version);
        }
        
        byte[] salt = new byte[SALT_LENGTH];
        buffer.get(salt);
        
        byte[] iv = new byte[IV_LENGTH];
        buffer.get(iv);
        
        byte[] ciphertextWithTag = new byte[buffer.remaining()];
        buffer.get(ciphertextWithTag);
        
        SecretKey key = deriveKey(passphrase, salt);
        
        Cipher cipher = Cipher.getInstance(CIPHER_ALGORITHM);
        GCMParameterSpec spec = new GCMParameterSpec(TAG_LENGTH * 8, iv);
        cipher.init(Cipher.DECRYPT_MODE, key, spec);
        
        byte[] plaintext = cipher.doFinal(ciphertextWithTag);
        String result = new String(plaintext, StandardCharsets.UTF_8);
        
        Arrays.fill(plaintext, (byte) 0);
        
        return result;
    }
    
    private static SecretKey deriveKey(char[] passphrase, byte[] salt) throws Exception {
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        KeySpec spec = new PBEKeySpec(passphrase, salt, ITERATIONS, 256);
        SecretKey tmp = factory.generateSecret(spec);
        return new SecretKeySpec(tmp.getEncoded(), "AES");
    }
    
    public static void storeEncryptedData(String filepath, String data, char[] passphrase) throws Exception {
        if (filepath == null || filepath.isEmpty()) {
            throw new IllegalArgumentException("Filepath cannot be null or empty");
        }
        
        Path path = Paths.get(filepath).toAbsolutePath().normalize();
        Path parent = path.getParent();
        if (parent == null) {
            throw new IllegalArgumentException("Invalid file path");
        }
        
        byte[] encrypted = encryptData(data, passphrase);
        
        Path tempFile = Files.createTempFile(parent, ".tmp", ".enc");
        try {
            Files.write(tempFile, encrypted, StandardOpenOption.WRITE);
            
            try {
                Set<PosixFilePermission> perms = new HashSet<>();
                perms.add(PosixFilePermission.OWNER_READ);
                perms.add(PosixFilePermission.OWNER_WRITE);
                Files.setPosixFilePermissions(tempFile, perms);
            } catch (UnsupportedOperationException e) {
                // Not a POSIX system, skip permission setting
            }
            
            Files.move(tempFile, path, java.nio.file.StandardCopyOption.ATOMIC_MOVE, 
                      java.nio.file.StandardCopyOption.REPLACE_EXISTING);
        } catch (Exception e) {
            try {
                Files.deleteIfExists(tempFile);
            } catch (IOException ignored) {
            }
            throw e;
        }
    }
    
    public static String retrieveEncryptedData(String filepath, char[] passphrase) throws Exception {
        if (filepath == null || filepath.isEmpty()) {
            throw new IllegalArgumentException("Filepath cannot be null or empty");
        }
        
        Path path = Paths.get(filepath).toAbsolutePath().normalize();
        
        if (!Files.exists(path)) {
            throw new IOException("File does not exist: " + filepath);
        }
        if (!Files.isRegularFile(path)) {
            throw new IOException("Path is not a regular file: " + filepath);
        }
        
        byte[] encrypted = Files.readAllBytes(path);
        return decryptData(encrypted, passphrase);
    }
    
    public static void main(String[] args) {
        try {
            // Test Case 1: Basic encryption and decryption
            System.out.println("Test Case 1: Basic encryption/decryption");
            String creditCard = "4532-1234-5678-9010";
            char[] pass1 = "SecurePassphrase123!".toCharArray();
            byte[] encrypted1 = encryptData(creditCard, pass1);
            String decrypted1 = decryptData(encrypted1, pass1);
            System.out.println("Success: " + constantTimeEquals(creditCard, decrypted1));
            Arrays.fill(pass1, '\\0');
            
            // Test Case 2: Personal information
            System.out.println("\\nTest Case 2: Personal information");
            String personalInfo = "SSN:123-45-6789|Name:John Doe";
            char[] pass2 = "AnotherSecurePass456$".toCharArray();
            byte[] encrypted2 = encryptData(personalInfo, pass2);
            String decrypted2 = decryptData(encrypted2, pass2);
            System.out.println("Success: " + constantTimeEquals(personalInfo, decrypted2));
            Arrays.fill(pass2, '\\0');
            
            // Test Case 3: File storage and retrieval
            System.out.println("\\nTest Case 3: File storage/retrieval");
            String testFile = "test_sensitive_data.enc";
            String testData = "Account:9876543210|PIN:5432";
            char[] pass3 = "FileTestPass789&".toCharArray();
            storeEncryptedData(testFile, testData, pass3);
            String retrieved = retrieveEncryptedData(testFile, pass3);
            System.out.println("Success: " + constantTimeEquals(testData, retrieved));
            Files.deleteIfExists(Paths.get(testFile));
            Arrays.fill(pass3, '\\0');
            
            // Test Case 4: Wrong passphrase handling
            System.out.println("\\nTest Case 4: Wrong passphrase");
            String data4 = "Sensitive:Data123";
            char[] correctPass = "CorrectPassword!".toCharArray();
            char[] wrongPass = "WrongPassword!".toCharArray();
            byte[] encrypted4 = encryptData(data4, correctPass);
            try {
                decryptData(encrypted4, wrongPass);
                System.out.println("Failed: Should have thrown exception");
            } catch (Exception e) {
                System.out.println("Success: Correctly rejected wrong passphrase");
            }
            Arrays.fill(correctPass, '\\0');
            Arrays.fill(wrongPass, '\\0');
            
            // Test Case 5: Empty input validation
            System.out.println("\\nTest Case 5: Input validation");
            try {
                encryptData("", "pass".toCharArray());
                System.out.println("Failed: Should have rejected empty plaintext");
            } catch (IllegalArgumentException e) {
                System.out.println("Success: Correctly rejected empty plaintext");
            }
            
        } catch (Exception e) {
            System.err.println("Error: " + e.getMessage());
        }
    }
    
    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) {
            return false;
        }
        byte[] aBytes = a.getBytes(StandardCharsets.UTF_8);
        byte[] bBytes = b.getBytes(StandardCharsets.UTF_8);
        return MessageDigest.isEqual(aBytes, bBytes);
    }
}
