
import javax.crypto.*;
import javax.crypto.spec.*;
import java.io.*;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.security.*;
import java.util.Arrays;

public class Task52 {
    private static final String MAGIC = "ENC1";
    private static final byte VERSION = 1;
    private static final int SALT_LENGTH = 16;
    private static final int IV_LENGTH = 12;
    private static final int TAG_LENGTH = 16;
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final int KEY_LENGTH = 256;
    
    public static byte[] encryptFile(String inputPath, String passphrase, String baseDir) throws Exception {
        if (passphrase == null || passphrase.isEmpty()) {
            throw new IllegalArgumentException("Passphrase cannot be empty");
        }
        
        Path base = Paths.get(baseDir).toRealPath();
        Path input = Paths.get(inputPath).toRealPath();
        
        if (!input.startsWith(base)) {
            throw new SecurityException("Path outside base directory");
        }
        
        if (!Files.isRegularFile(input)) {
            throw new IllegalArgumentException("Input must be a regular file");
        }
        
        byte[] plaintext = Files.readAllBytes(input);
        
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        
        byte[] iv = new byte[IV_LENGTH];
        random.nextBytes(iv);
        
        SecretKey key = deriveKey(passphrase, salt);
        
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(TAG_LENGTH * 8, iv);
        cipher.init(Cipher.ENCRYPT_MODE, key, spec);
        
        byte[] ciphertextWithTag = cipher.doFinal(plaintext);
        
        ByteBuffer buffer = ByteBuffer.allocate(
            MAGIC.length() + 1 + SALT_LENGTH + IV_LENGTH + ciphertextWithTag.length
        );
        buffer.put(MAGIC.getBytes(StandardCharsets.UTF_8));
        buffer.put(VERSION);
        buffer.put(salt);
        buffer.put(iv);
        buffer.put(ciphertextWithTag);
        
        return buffer.array();
    }
    
    public static byte[] decryptFile(byte[] encryptedData, String passphrase) throws Exception {
        if (passphrase == null || passphrase.isEmpty()) {
            throw new IllegalArgumentException("Passphrase cannot be empty");
        }
        
        if (encryptedData == null || encryptedData.length < MAGIC.length() + 1 + SALT_LENGTH + IV_LENGTH + TAG_LENGTH) {
            throw new IllegalArgumentException("Invalid encrypted data");
        }
        
        ByteBuffer buffer = ByteBuffer.wrap(encryptedData);
        
        byte[] magicBytes = new byte[MAGIC.length()];
        buffer.get(magicBytes);
        String magic = new String(magicBytes, StandardCharsets.UTF_8);
        
        if (!MessageDigest.isEqual(magic.getBytes(StandardCharsets.UTF_8), MAGIC.getBytes(StandardCharsets.UTF_8))) {
            throw new SecurityException("Invalid file format");
        }
        
        byte version = buffer.get();
        if (version != VERSION) {
            throw new SecurityException("Unsupported version");
        }
        
        byte[] salt = new byte[SALT_LENGTH];
        buffer.get(salt);
        
        byte[] iv = new byte[IV_LENGTH];
        buffer.get(iv);
        
        byte[] ciphertextWithTag = new byte[buffer.remaining()];
        buffer.get(ciphertextWithTag);
        
        SecretKey key = deriveKey(passphrase, salt);
        
        Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
        GCMParameterSpec spec = new GCMParameterSpec(TAG_LENGTH * 8, iv);
        cipher.init(Cipher.DECRYPT_MODE, key, spec);
        
        try {
            return cipher.doFinal(ciphertextWithTag);
        } catch (AEADBadTagException e) {
            throw new SecurityException("Authentication failed: data may be corrupted or tampered");
        }
    }
    
    private static SecretKey deriveKey(String passphrase, byte[] salt) throws Exception {
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        KeySpec spec = new PBEKeySpec(passphrase.toCharArray(), salt, PBKDF2_ITERATIONS, KEY_LENGTH);
        SecretKey tmp = factory.generateSecret(spec);
        return new SecretKeySpec(tmp.getEncoded(), "AES");
    }
    
    public static void main(String[] args) {
        try {
            Path tempDir = Files.createTempDirectory("test_encrypt");
            String baseDir = tempDir.toString();
            
            System.out.println("Test 1: Basic encryption and decryption");
            Path testFile1 = tempDir.resolve("test1.txt");
            Files.write(testFile1, "Hello World".getBytes(StandardCharsets.UTF_8));
            byte[] encrypted1 = encryptFile(testFile1.toString(), "password123", baseDir);
            byte[] decrypted1 = decryptFile(encrypted1, "password123");
            System.out.println("Result: " + new String(decrypted1, StandardCharsets.UTF_8));
            
            System.out.println("\\nTest 2: Different content");
            Path testFile2 = tempDir.resolve("test2.txt");
            Files.write(testFile2, "Secret Message 42".getBytes(StandardCharsets.UTF_8));
            byte[] encrypted2 = encryptFile(testFile2.toString(), "strongpass", baseDir);
            byte[] decrypted2 = decryptFile(encrypted2, "strongpass");
            System.out.println("Result: " + new String(decrypted2, StandardCharsets.UTF_8));
            
            System.out.println("\\nTest 3: Wrong password");
            try {
                decryptFile(encrypted1, "wrongpassword");
                System.out.println("ERROR: Should have failed");
            } catch (SecurityException e) {
                System.out.println("Correctly rejected: " + e.getMessage());
            }
            
            System.out.println("\\nTest 4: Path traversal prevention");
            try {
                encryptFile("../outside.txt", "pass", baseDir);
                System.out.println("ERROR: Should have rejected path");
            } catch (Exception e) {
                System.out.println("Correctly rejected: " + e.getMessage());
            }
            
            System.out.println("\\nTest 5: Large content");
            Path testFile5 = tempDir.resolve("test5.txt");
            byte[] largeContent = new byte[10000];
            Arrays.fill(largeContent, (byte)'A');
            Files.write(testFile5, largeContent);
            byte[] encrypted5 = encryptFile(testFile5.toString(), "largepass", baseDir);
            byte[] decrypted5 = decryptFile(encrypted5, "largepass");
            System.out.println("Result: " + decrypted5.length + " bytes decrypted");
            
            for (File f : tempDir.toFile().listFiles()) {
                f.delete();
            }
            tempDir.toFile().delete();
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
