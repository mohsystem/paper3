import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Set;

/**
 * NOTE: This is a demonstration of cryptographic key management concepts.
 * Storing raw key material in application memory is NOT a secure practice
 * for production systems. Production systems should use Hardware Security
 * Modules (HSMs) or dedicated key management services (e.g., AWS KMS, Azure Key Vault).
 */
class KeyManager {
    private final Map<String, byte[]> keyStore = new ConcurrentHashMap<>();

    /**
     * Generates a new AES key using a cryptographically secure random number generator.
     *
     * @param keyName The alias to store the key under.
     * @param keySizeInBits The size of the key in bits (must be 128, 192, or 256 for AES).
     * @return true if key generation was successful, false otherwise.
     */
    public boolean generateKey(String keyName, int keySizeInBits) {
        if (keyName == null || keyName.isEmpty()) {
            System.err.println("Key name cannot be null or empty.");
            return false;
        }
        if (keySizeInBits != 128 && keySizeInBits != 192 && keySizeInBits != 256) {
            System.err.println("Invalid AES key size. Must be 128, 192, or 256 bits.");
            return false;
        }

        try {
            KeyGenerator keyGen = KeyGenerator.getInstance("AES");
            SecureRandom secureRandom = new SecureRandom();
            keyGen.init(keySizeInBits, secureRandom);
            SecretKey secretKey = keyGen.generateKey();
            keyStore.put(keyName, secretKey.getEncoded());
            return true;
        } catch (NoSuchAlgorithmException e) {
            System.err.println("Error generating key: " + e.getMessage());
            return false;
        }
    }

    /**
     * Retrieves a copy of the key material for the given key name.
     *
     * @param keyName The alias of the key to retrieve.
     * @return A copy of the key as a byte array, or null if not found.
     */
    public byte[] getKey(String keyName) {
        byte[] key = keyStore.get(keyName);
        if (key == null) {
            return null;
        }
        // Return a copy to prevent modification of the stored key
        return Arrays.copyOf(key, key.length);
    }

    /**
     * Deletes a key from the store and overwrites its memory.
     *
     * @param keyName The alias of the key to delete.
     * @return true if the key was found and deleted, false otherwise.
     */
    public boolean deleteKey(String keyName) {
        byte[] key = keyStore.get(keyName);
        if (key != null) {
            // Overwrite the key material for security before removing
            Arrays.fill(key, (byte) 0);
            keyStore.remove(keyName);
            return true;
        }
        return false;
    }

    /**
     * Lists all key names currently in the store.
     *
     * @return A set of key names.
     */
    public Set<String> listKeys() {
        return keyStore.keySet();
    }
}

public class Task134 {
    public static void main(String[] args) {
        KeyManager keyManager = new KeyManager();
        System.out.println("--- Java Key Manager Test ---");

        // Test Case 1: Generate a new key
        System.out.println("\n1. Generating a 256-bit key named 'api_key_1'...");
        boolean generated = keyManager.generateKey("api_key_1", 256);
        System.out.println("Key generated successfully: " + generated);
        System.out.println("Current keys: " + keyManager.listKeys());

        // Test Case 2: Retrieve the key
        System.out.println("\n2. Retrieving key 'api_key_1'...");
        byte[] retrievedKey = keyManager.getKey("api_key_1");
        if (retrievedKey != null) {
            System.out.println("Retrieved key (Base64): " + Base64.getEncoder().encodeToString(retrievedKey));
            System.out.println("Retrieved key length: " + retrievedKey.length * 8 + " bits");
        } else {
            System.out.println("Failed to retrieve key.");
        }

        // Test Case 3: Attempt to retrieve a non-existent key
        System.out.println("\n3. Attempting to retrieve non-existent key 'fake_key'...");
        byte[] fakeKey = keyManager.getKey("fake_key");
        System.out.println("Retrieved 'fake_key': " + (fakeKey == null ? "null (as expected)" : "FAIL"));

        // Test Case 4: Delete the key
        System.out.println("\n4. Deleting key 'api_key_1'...");
        boolean deleted = keyManager.deleteKey("api_key_1");
        System.out.println("Key deleted successfully: " + deleted);
        System.out.println("Current keys: " + keyManager.listKeys());

        // Test Case 5: Attempt to retrieve the deleted key
        System.out.println("\n5. Attempting to retrieve deleted key 'api_key_1'...");
        byte[] deletedKey = keyManager.getKey("api_key_1");
        System.out.println("Retrieved 'api_key_1' after deletion: " + (deletedKey == null ? "null (as expected)" : "FAIL"));

        // Extra test: Invalid key size
        System.out.println("\n6. Attempting to generate key with invalid size (100 bits)...");
        keyManager.generateKey("invalid_key", 100);
    }
}