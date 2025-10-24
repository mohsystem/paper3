import javax.crypto.*;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.security.*;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

/**
 * Manages cryptographic keys securely in memory.
 * This is a simplified example. In a real-world application:
 * - The master key should not be derived from a hardcoded password.
 * - The master key should be protected by a Hardware Security Module (HSM).
 * - Key material should be handled in byte arrays and zeroed out after use if possible.
 */
class KeyManager {
    // AES/GCM parameters for wrapping (encrypting) keys
    private static final String WRAP_ALGORITHM = "AES/GCM/NoPadding";
    private static final int GCM_IV_LENGTH = 12; // 96 bits
    private static final int GCM_TAG_LENGTH = 16; // 128 bits

    // Master key derivation parameters
    private static final String KDF_ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final int KDF_ITERATIONS = 65536;
    private static final int KEY_SIZE = 256;

    private final SecretKey masterKey;
    private final Map<String, byte[]> keyStore = new HashMap<>();
    private final SecureRandom secureRandom = new SecureRandom();

    public KeyManager(char[] password, byte[] salt) throws NoSuchAlgorithmException, InvalidKeySpecException {
        this.masterKey = deriveMasterKey(password, salt);
    }

    private SecretKey deriveMasterKey(char[] password, byte[] salt) throws NoSuchAlgorithmException, InvalidKeySpecException {
        SecretKeyFactory factory = SecretKeyFactory.getInstance(KDF_ALGORITHM);
        KeySpec spec = new PBEKeySpec(password, salt, KDF_ITERATIONS, KEY_SIZE);
        SecretKey tmp = factory.generateSecret(spec);
        return new SecretKeySpec(tmp.getEncoded(), "AES");
    }
    
    /**
     * Generates a new AES key, wraps it with the master key, and stores it.
     *
     * @param alias The name to associate with the key.
     * @param keySizeInBits The size of the key to generate (e.g., 128, 256).
     * @return true if key was generated and stored, false otherwise.
     */
    public boolean generateKey(String alias, int keySizeInBits) {
        try {
            KeyGenerator keyGen = KeyGenerator.getInstance("AES");
            keyGen.init(keySizeInBits, secureRandom);
            SecretKey newKey = keyGen.generateKey();

            // Wrap the key
            byte[] iv = new byte[GCM_IV_LENGTH];
            secureRandom.nextBytes(iv);
            GCMParameterSpec parameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);

            Cipher cipher = Cipher.getInstance(WRAP_ALGORITHM);
            cipher.init(Cipher.ENCRYPT_MODE, this.masterKey, parameterSpec);
            byte[] wrappedKey = cipher.doFinal(newKey.getEncoded());

            // Prepend IV to the wrapped key for storage
            byte[] ivAndWrappedKey = new byte[GCM_IV_LENGTH + wrappedKey.length];
            System.arraycopy(iv, 0, ivAndWrappedKey, 0, GCM_IV_LENGTH);
            System.arraycopy(wrappedKey, 0, ivAndWrappedKey, GCM_IV_LENGTH, wrappedKey.length);
            
            keyStore.put(alias, ivAndWrappedKey);

            // Securely clear the plaintext key from memory
            Arrays.fill(newKey.getEncoded(), (byte) 0);

            return true;
        } catch (Exception e) {
            System.err.println("Error generating key for alias '" + alias + "': " + e.getMessage());
            return false;
        }
    }

    /**
     * Retrieves and unwraps a stored key.
     *
     * @param alias The name of the key to retrieve.
     * @return The SecretKey, or null if not found or if unwrapping fails.
     */
    public SecretKey getKey(String alias) {
        byte[] ivAndWrappedKey = keyStore.get(alias);
        if (ivAndWrappedKey == null) {
            return null;
        }

        try {
            byte[] iv = Arrays.copyOfRange(ivAndWrappedKey, 0, GCM_IV_LENGTH);
            byte[] wrappedKey = Arrays.copyOfRange(ivAndWrappedKey, GCM_IV_LENGTH, ivAndWrappedKey.length);
            
            GCMParameterSpec parameterSpec = new GCMParameterSpec(GCM_TAG_LENGTH * 8, iv);
            Cipher cipher = Cipher.getInstance(WRAP_ALGORITHM);
            cipher.init(Cipher.DECRYPT_MODE, this.masterKey, parameterSpec);
            
            byte[] unwrappedKeyBytes = cipher.doFinal(wrappedKey);
            SecretKey key = new SecretKeySpec(unwrappedKeyBytes, "AES");
            
            // It is the caller's responsibility to clear the key after use.
            return key;
        } catch (Exception e) {
            System.err.println("Error retrieving key for alias '" + alias + "': " + e.getMessage());
            return null;
        }
    }

    /**
     * Deletes a key from the key store.
     *
     * @param alias The name of the key to delete.
     * @return true if the key existed and was deleted, false otherwise.
     */
    public boolean deleteKey(String alias) {
        return keyStore.remove(alias) != null;
    }
}


public class Task134 {
    public static void main(String[] args) {
        System.out.println("--- Java KeyManager Tests ---");
        try {
            // In a real application, the password would be obtained securely, 
            // and the salt would be unique and stored.
            char[] masterPassword = "a-very-strong-master-password".toCharArray();
            byte[] salt = new byte[] { (byte)0x1a, (byte)0x2b, (byte)0x3c, (byte)0x4d, (byte)0x5e, (byte)0x6f, (byte)0x7a, (byte)0x8b, 
                                      (byte)0x11, (byte)0x22, (byte)0x33, (byte)0x44, (byte)0x55, (byte)0x66, (byte)0x77, (byte)0x88 };

            KeyManager keyManager = new KeyManager(masterPassword, salt);
            
            // Test Case 1: Generate a new key
            System.out.println("1. Generating key 'api-key-1'...");
            boolean generated = keyManager.generateKey("api-key-1", 256);
            System.out.println("   Key generated: " + generated);
            assert generated;

            // Test Case 2: Retrieve the key
            System.out.println("\n2. Retrieving key 'api-key-1'...");
            SecretKey key1 = keyManager.getKey("api-key-1");
            System.out.println("   Key retrieved: " + (key1 != null));
            if (key1 != null) {
                System.out.println("   Key algorithm: " + key1.getAlgorithm());
                System.out.println("   Key length (bytes): " + key1.getEncoded().length);
                // Securely clear key after use
                Arrays.fill(key1.getEncoded(), (byte) 0);
            }
            assert key1 != null;
            
            // Test Case 3: Generate a second key
            System.out.println("\n3. Generating key 'db-key-2'...");
            boolean generated2 = keyManager.generateKey("db-key-2", 128);
            System.out.println("   Key generated: " + generated2);
            SecretKey key2 = keyManager.getKey("db-key-2");
            System.out.println("   Second key retrieved: " + (key2 != null));
            assert generated2 && key2 != null;

            // Test Case 4: Delete the first key
            System.out.println("\n4. Deleting key 'api-key-1'...");
            boolean deleted = keyManager.deleteKey("api-key-1");
            System.out.println("   Key deleted: " + deleted);
            assert deleted;

            // Test Case 5: Attempt to retrieve the deleted key
            System.out.println("\n5. Attempting to retrieve deleted key 'api-key-1'...");
            SecretKey deletedKey = keyManager.getKey("api-key-1");
            System.out.println("   Key retrieved: " + (deletedKey != null));
            assert deletedKey == null;

            System.out.println("\nAll tests passed.");

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}