import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.Formatter;

public class Task134 {

    private final Map<String, byte[]> keyStore;
    private final SecureRandom secureRandom;
    private static final int KEY_SIZE_BYTES = 32; // 256 bits

    public Task134() {
        this.keyStore = new HashMap<>();
        this.secureRandom = new SecureRandom();
    }

    /**
     * Generates a new cryptographic key and stores it with the given ID.
     * @param id The identifier for the key.
     * @return The generated key as a hex string.
     */
    public String generateKey(String id) {
        if (id == null || id.isEmpty()) {
            throw new IllegalArgumentException("Key ID cannot be null or empty.");
        }
        byte[] key = new byte[KEY_SIZE_BYTES];
        secureRandom.nextBytes(key);
        keyStore.put(id, key);
        return bytesToHexString(key);
    }

    /**
     * Retrieves a key by its ID.
     * @param id The identifier for the key.
     * @return The key as a byte array, or null if not found.
     */
    public byte[] getKey(String id) {
        return keyStore.get(id);
    }

    /**
     * Deletes a key by its ID.
     * @param id The identifier for the key.
     * @return true if the key was deleted, false otherwise.
     */
    public boolean deleteKey(String id) {
        return keyStore.remove(id) != null;
    }

    /**
     * Lists all key identifiers currently in the store.
     * @return A set of all key IDs.
     */
    public Set<String> listKeys() {
        return keyStore.keySet();
    }

    /**
     * Helper to convert a byte array to a hexadecimal string.
     * @param bytes The byte array to convert.
     * @return The hexadecimal string representation.
     */
    public static String bytesToHexString(byte[] bytes) {
        if (bytes == null) {
            return "null";
        }
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        try (Formatter formatter = new Formatter(sb)) {
            for (byte b : bytes) {
                formatter.format("%02x", b);
            }
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        Task134 keyManager = new Task134();
        System.out.println("Cryptographic Key Manager - Java");
        System.out.println("---------------------------------");

        // Test Case 1: Generate a key for "user1"
        System.out.println("1. Generating key for 'user1'...");
        String key1Hex = keyManager.generateKey("user1");
        System.out.println("   Generated key for 'user1': " + key1Hex);
        System.out.println();

        // Test Case 2: Generate a key for "user2"
        System.out.println("2. Generating key for 'user2'...");
        String key2Hex = keyManager.generateKey("user2");
        System.out.println("   Generated key for 'user2': " + key2Hex);
        System.out.println();

        // Test Case 3: Retrieve and print the key for "user1"
        System.out.println("3. Retrieving key for 'user1'...");
        byte[] retrievedKey1 = keyManager.getKey("user1");
        System.out.println("   Retrieved key for 'user1': " + bytesToHexString(retrievedKey1));
        System.out.println();

        // Test Case 4: Attempt to retrieve a key for "user3" (non-existent)
        System.out.println("4. Attempting to retrieve non-existent key 'user3'...");
        byte[] retrievedKey3 = keyManager.getKey("user3");
        System.out.println("   Retrieved key for 'user3': " + (retrievedKey3 == null ? "Not Found" : bytesToHexString(retrievedKey3)));
        System.out.println();

        // Test Case 5: Delete the key for "user2" and list remaining keys
        System.out.println("5. Deleting key for 'user2' and listing keys...");
        boolean deleted = keyManager.deleteKey("user2");
        System.out.println("   Key 'user2' deleted: " + deleted);
        System.out.println("   Current keys in store: " + keyManager.listKeys());
        System.out.println("---------------------------------");
    }
}