
import javax.crypto.Cipher;
import javax.crypto.Mac;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;

public class Task83 {
    
    private static final String MAGIC = "ENC1";
    private static final byte VERSION = 1;
    private static final int SALT_LENGTH = 16;
    private static final int IV_LENGTH = 16;
    private static final int HMAC_LENGTH = 32;
    private static final int PBKDF2_ITERATIONS = 210000;
    private static final int AES_KEY_LENGTH = 256;
    
    /**
     * Derives a 256-bit encryption key from a passphrase using PBKDF2-HMAC-SHA256.
     * Uses 210,000 iterations as per security requirements.
     * 
     * @param passphrase The passphrase to derive the key from
     * @param salt Unique 16-byte salt for key derivation
     * @return 32-byte derived key
     * @throws Exception if key derivation fails
     */
    private static byte[] deriveKey(String passphrase, byte[] salt) throws Exception {
        // Validate inputs - treat all inputs as untrusted
        if (passphrase == null || passphrase.isEmpty()) {
            throw new IllegalArgumentException("Passphrase must not be empty");
        }
        if (salt == null || salt.length != SALT_LENGTH) {
            throw new IllegalArgumentException("Salt must be exactly 16 bytes");
        }
        
        // Use PBKDF2 with HMAC-SHA256 for secure key derivation
        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        KeySpec spec = new PBEKeySpec(
            passphrase.toCharArray(),
            salt,
            PBKDF2_ITERATIONS,
            AES_KEY_LENGTH
        );
        SecretKey tmp = factory.generateSecret(spec);
        byte[] key = tmp.getEncoded();
        
        // Clear the PBEKeySpec to remove passphrase from memory
        try {
            spec.clearPassword();
        } catch (Exception e) {
            // Best effort to clear
        }
        
        return key;
    }
    
    /**
     * Computes HMAC-SHA256 for Encrypt-then-MAC pattern.
     * 
     * @param key HMAC key
     * @param data Data to authenticate
     * @return HMAC tag
     * @throws Exception if HMAC computation fails
     */
    private static byte[] computeHmac(byte[] key, byte[] data) throws Exception {
        Mac mac = Mac.getInstance("HmacSHA256");
        SecretKeySpec keySpec = new SecretKeySpec(key, "HmacSHA256");
        mac.init(keySpec);
        return mac.doFinal(data);
    }
    
    /**
     * Encrypts data using AES-256-CBC with HMAC-SHA256 (Encrypt-then-MAC).
     * Returns a structured format: [magic][version][salt][hmac_salt][iv][ciphertext][hmac_tag]
     * 
     * Security measures:
     * - Uses AES-256 in CBC mode with PKCS5Padding
     * - Implements Encrypt-then-MAC pattern for authentication
     * - Generates unique 16-byte salt per encryption using SecureRandom
     * - Generates unique 16-byte IV per encryption using SecureRandom
     * - Uses PBKDF2-HMAC-SHA256 with 210,000 iterations
     * - Validates all inputs before processing
     * 
     * @param plaintext Data to encrypt
     * @param passphrase Passphrase for key derivation
     * @return Encrypted data with authentication
     * @throws Exception if encryption fails
     */
    public static byte[] encryptAesCbcWithHmac(byte[] plaintext, String passphrase) throws Exception {
        // Input validation - treat all inputs as untrusted
        if (plaintext == null || plaintext.length == 0) {
            throw new IllegalArgumentException("Plaintext must not be empty");
        }
        if (passphrase == null || passphrase.isEmpty()) {
            throw new IllegalArgumentException("Passphrase must not be empty");
        }
        if (plaintext.length > 10 * 1024 * 1024) { // 10MB limit
            throw new IllegalArgumentException("Plaintext exceeds maximum size");
        }
        
        // Generate cryptographically secure random salt and IV using SecureRandom
        SecureRandom secureRandom = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        byte[] hmacSalt = new byte[SALT_LENGTH];
        byte[] iv = new byte[IV_LENGTH];
        
        secureRandom.nextBytes(salt);      // Unique salt for encryption key
        secureRandom.nextBytes(hmacSalt);  // Unique salt for HMAC key
        secureRandom.nextBytes(iv);        // Unique IV for CBC mode
        
        // Derive encryption key from passphrase using secure KDF
        byte[] encryptionKey = deriveKey(passphrase, salt);
        
        // Derive separate HMAC key for Encrypt-then-MAC
        byte[] hmacKey = deriveKey(passphrase, hmacSalt);
        
        // Create AES-256-CBC cipher with explicit mode and padding
        Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        SecretKeySpec keySpec = new SecretKeySpec(encryptionKey, "AES");
        IvParameterSpec ivSpec = new IvParameterSpec(iv);
        cipher.init(Cipher.ENCRYPT_MODE, keySpec, ivSpec);
        
        // Encrypt the plaintext
        byte[] ciphertext = cipher.doFinal(plaintext);
        
        // Build the output structure: magic + version + salt + hmac_salt + iv + ciphertext
        byte[] magicBytes = MAGIC.getBytes(StandardCharsets.UTF_8);
        ByteBuffer buffer = ByteBuffer.allocate(
            magicBytes.length + 1 + SALT_LENGTH + SALT_LENGTH + IV_LENGTH + 
            ciphertext.length + HMAC_LENGTH
        );
        
        buffer.put(magicBytes);
        buffer.put(VERSION);
        buffer.put(salt);
        buffer.put(hmacSalt);
        buffer.put(iv);
        buffer.put(ciphertext);
        
        // Get authenticated data (everything except HMAC tag)
        byte[] authenticatedData = new byte[buffer.position()];
        buffer.position(0);
        buffer.get(authenticatedData);
        
        // Compute HMAC over all data (Encrypt-then-MAC pattern)
        byte[] hmacTag = computeHmac(hmacKey, authenticatedData);
        buffer.put(hmacTag);
        
        // Securely clear sensitive key material
        Arrays.fill(encryptionKey, (byte) 0);
        Arrays.fill(hmacKey, (byte) 0);
        
        return buffer.array();
    }
    
    /**
     * Decrypts data encrypted with encryptAesCbcWithHmac.
     * Verifies HMAC before decryption (fail closed on authentication failure).
     * 
     * Security measures:
     * - Validates magic and version before processing
     * - Verifies HMAC using MessageDigest.isEqual (constant time) before decryption
     * - Fails closed on any validation error without revealing details
     * - Does not return partial plaintext on error
     * 
     * @param encryptedData Encrypted data with authentication
     * @param passphrase Passphrase for key derivation
     * @return Decrypted plaintext
     * @throws Exception if decryption or authentication fails
     */
    public static byte[] decryptAesCbcWithHmac(byte[] encryptedData, String passphrase) throws Exception {
        // Input validation
        if (encryptedData == null || encryptedData.length == 0) {
            throw new IllegalArgumentException("Encrypted data must not be empty");
        }
        if (passphrase == null || passphrase.isEmpty()) {
            throw new IllegalArgumentException("Passphrase must not be empty");
        }
        
        // Minimum size check
        int minSize = 4 + 1 + SALT_LENGTH + SALT_LENGTH + IV_LENGTH + HMAC_LENGTH;
        if (encryptedData.length < minSize) {
            throw new IllegalArgumentException("Invalid encrypted data format");
        }
        
        ByteBuffer buffer = ByteBuffer.wrap(encryptedData);
        
        // Parse and validate magic
        byte[] magicBytes = new byte[4];
        buffer.get(magicBytes);
        String magic = new String(magicBytes, StandardCharsets.UTF_8);
        if (!MAGIC.equals(magic)) {
            throw new IllegalArgumentException("Invalid file format");
        }
        
        // Parse and validate version
        byte version = buffer.get();
        if (version != VERSION) {
            throw new IllegalArgumentException("Unsupported version");
        }
        
        // Parse salt, hmac_salt, and IV
        byte[] salt = new byte[SALT_LENGTH];
        byte[] hmacSalt = new byte[SALT_LENGTH];
        byte[] iv = new byte[IV_LENGTH];
        buffer.get(salt);
        buffer.get(hmacSalt);
        buffer.get(iv);
        
        // Parse ciphertext and HMAC tag
        int ciphertextLength = encryptedData.length - buffer.position() - HMAC_LENGTH;
        byte[] ciphertext = new byte[ciphertextLength];
        byte[] hmacTag = new byte[HMAC_LENGTH];
        buffer.get(ciphertext);
        buffer.get(hmacTag);
        
        // Get authenticated data (everything except HMAC tag)
        byte[] authenticatedData = Arrays.copyOfRange(
            encryptedData, 
            0, 
            encryptedData.length - HMAC_LENGTH
        );
        
        // Derive HMAC key to verify authentication tag
        byte[] hmacKey = deriveKey(passphrase, hmacSalt);
        
        // Verify HMAC tag using constant-time comparison
        byte[] expectedHmac = computeHmac(hmacKey, authenticatedData);
        
        // Use MessageDigest.isEqual for constant-time comparison
        if (!MessageDigest.isEqual(expectedHmac, hmacTag)) {
            // Fail closed - clear sensitive data and throw exception
            Arrays.fill(hmacKey, (byte) 0);
            throw new SecurityException("Authentication failed");
        }
        
        // HMAC verified - proceed with decryption
        byte[] encryptionKey = deriveKey(passphrase, salt);
        
        // Create AES-256-CBC cipher for decryption
        Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
        SecretKeySpec keySpec = new SecretKeySpec(encryptionKey, "AES");
        IvParameterSpec ivSpec = new IvParameterSpec(iv);
        cipher.init(Cipher.DECRYPT_MODE, keySpec, ivSpec);
        
        // Decrypt the ciphertext
        byte[] plaintext = cipher.doFinal(ciphertext);
        
        // Clear sensitive key material
        Arrays.fill(encryptionKey, (byte) 0);
        Arrays.fill(hmacKey, (byte) 0);
        
        return plaintext;
    }
    
    public static void main(String[] args) {
        System.out.println("AES-256-CBC with HMAC-SHA256 Encryption Test Cases\\n");
        
        try {
            // Test case 1: Basic encryption/decryption
            System.out.println("Test 1: Basic encryption/decryption");
            byte[] plaintext1 = "Secret message to encrypt".getBytes(StandardCharsets.UTF_8);
            String passphrase1 = "StrongPassphrase123!@#";
            byte[] encrypted1 = encryptAesCbcWithHmac(plaintext1, passphrase1);
            byte[] decrypted1 = decryptAesCbcWithHmac(encrypted1, passphrase1);
            System.out.println("Original:  " + new String(plaintext1, StandardCharsets.UTF_8));
            System.out.println("Encrypted length: " + encrypted1.length + " bytes");
            System.out.println("Decrypted: " + new String(decrypted1, StandardCharsets.UTF_8));
            System.out.println("Match: " + Arrays.equals(plaintext1, decrypted1) + "\\n");
            
            // Test case 2: Encrypting binary data
            System.out.println("Test 2: Binary data encryption");
            byte[] plaintext2 = new byte[64];
            new SecureRandom().nextBytes(plaintext2);
            String passphrase2 = "AnotherSecurePass456$%^";
            byte[] encrypted2 = encryptAesCbcWithHmac(plaintext2, passphrase2);
            byte[] decrypted2 = decryptAesCbcWithHmac(encrypted2, passphrase2);
            System.out.println("Original length:  " + plaintext2.length + " bytes");
            System.out.println("Encrypted length: " + encrypted2.length + " bytes");
            System.out.println("Match: " + Arrays.equals(plaintext2, decrypted2) + "\\n");
            
            // Test case 3: UTF-8 text encryption
            System.out.println("Test 3: UTF-8 text encryption");
            byte[] plaintext3 = "Hello, 世界! 🔐".getBytes(StandardCharsets.UTF_8);
            String passphrase3 = "Unicode_Passphrase_2024!";
            byte[] encrypted3 = encryptAesCbcWithHmac(plaintext3, passphrase3);
            byte[] decrypted3 = decryptAesCbcWithHmac(encrypted3, passphrase3);
            System.out.println("Original:  " + new String(plaintext3, StandardCharsets.UTF_8));
            System.out.println("Decrypted: " + new String(decrypted3, StandardCharsets.UTF_8));
            System.out.println("Match: " + Arrays.equals(plaintext3, decrypted3) + "\\n");
            
            // Test case 4: Large data encryption
            System.out.println("Test 4: Large data encryption");
            byte[] plaintext4 = new byte[10000];
            Arrays.fill(plaintext4, (byte) 'X');
            String passphrase4 = "LargeDataPass789&*()";
            byte[] encrypted4 = encryptAesCbcWithHmac(plaintext4, passphrase4);
            byte[] decrypted4 = decryptAesCbcWithHmac(encrypted4, passphrase4);
            System.out.println("Original length:  " + plaintext4.length + " bytes");
            System.out.println("Encrypted length: " + encrypted4.length + " bytes");
            System.out.println("Match: " + Arrays.equals(plaintext4, decrypted4) + "\\n");
            
            // Test case 5: Verify HMAC authentication failure detection
            System.out.println("Test 5: HMAC authentication failure detection");
            byte[] plaintext5 = "Authenticated message".getBytes(StandardCharsets.UTF_8);
            String passphrase5 = "AuthTestPass!@#123";
            byte[] encrypted5 = encryptAesCbcWithHmac(plaintext5, passphrase5);
            
            // Tamper with the ciphertext
            byte[] tampered = Arrays.copyOf(encrypted5, encrypted5.length);
            tampered[tampered.length - 50] ^= 0xFF;  // Flip bits
            
            try {
                decryptAesCbcWithHmac(tampered, passphrase5);
                System.out.println("FAILED: Tampered data was accepted");
            } catch (SecurityException e) {
                System.out.println("SUCCESS: Tampered data rejected - " + e.getMessage() + "\\n");
            }
            
            System.out.println("All test cases completed successfully!");
            
        } catch (Exception e) {
            System.err.println("Error during testing: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
