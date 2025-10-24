import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Base64;

public class Task134 {

    // Secure in-memory key manager for symmetric keys (AES)
    public static final class KeyManager {
        private static final class KeyRecord {
            final String id;
            final String algorithm;
            final int bits;
            final boolean exportable;
            final long createdAtEpochSeconds;
            byte[] keyMaterial;

            KeyRecord(String id, String algorithm, int bits, boolean exportable, byte[] keyMaterial) {
                this.id = id;
                this.algorithm = algorithm;
                this.bits = bits;
                this.exportable = exportable;
                this.createdAtEpochSeconds = Instant.now().getEpochSecond();
                this.keyMaterial = keyMaterial;
            }
        }

        private final SecureRandom secureRandom;
        private final Map<String, KeyRecord> store;

        public KeyManager() {
            this.secureRandom = new SecureRandom();
            this.store = new ConcurrentHashMap<>();
        }

        private String generateId() {
            byte[] idBytes = new byte[16];
            secureRandom.nextBytes(idBytes);
            StringBuilder sb = new StringBuilder(idBytes.length * 2);
            for (byte b : idBytes) {
                sb.append(String.format("%02x", b));
            }
            return sb.toString();
        }

        private static void secureWipe(byte[] arr) {
            if (arr == null) return;
            Arrays.fill(arr, (byte) 0);
        }

        public String createSymmetricKey(String algorithm, int bits, boolean exportable) throws Exception {
            if (algorithm == null || !algorithm.equalsIgnoreCase("AES")) {
                throw new IllegalArgumentException("Unsupported algorithm");
            }
            if (bits != 128 && bits != 192 && bits != 256) {
                throw new IllegalArgumentException("Invalid key size");
            }
            KeyGenerator kg = KeyGenerator.getInstance("AES");
            try {
                kg.init(bits, secureRandom);
            } catch (Exception e) {
                // Fallback to 128 if 192/256 not supported
                if (bits > 128) {
                    kg.init(128, secureRandom);
                    bits = 128;
                } else {
                    throw e;
                }
            }
            SecretKey sk = kg.generateKey();
            byte[] material = sk.getEncoded();
            if (material == null || material.length * 8 != bits) {
                // ensure the reported bits match encoded length
                bits = material.length * 8;
            }
            String id = generateId();
            KeyRecord rec = new KeyRecord(id, "AES", bits, exportable, material);
            store.put(id, rec);
            return id;
        }

        public String getKeyInfo(String keyId) {
            if (keyId == null) return null;
            KeyRecord rec = store.get(keyId);
            if (rec == null) return null;
            // Return JSON-like metadata string without exposing key
            return String.format(Locale.ROOT,
                    "{\"id\":\"%s\",\"algorithm\":\"%s\",\"bits\":%d,\"exportable\":%b,\"createdAt\":%d}",
                    rec.id, rec.algorithm, rec.bits, rec.exportable, rec.createdAtEpochSeconds);
        }

        public List<String> listKeyIds() {
            return new ArrayList<>(store.keySet());
        }

        public boolean deleteKey(String keyId) {
            if (keyId == null) return false;
            KeyRecord rec = store.remove(keyId);
            if (rec == null) return false;
            secureWipe(rec.keyMaterial);
            return true;
        }

        public boolean rotateKey(String keyId) throws Exception {
            if (keyId == null) return false;
            KeyRecord rec = store.get(keyId);
            if (rec == null) return false;
            // Generate a new key with same algorithm and size, preserving exportability.
            byte[] old = rec.keyMaterial;
            try {
                KeyGenerator kg = KeyGenerator.getInstance(rec.algorithm);
                kg.init(rec.bits, secureRandom);
                SecretKey sk = kg.generateKey();
                byte[] material = sk.getEncoded();
                rec.keyMaterial = material;
                // Note: createdAt remains original to keep history simple; alternatively update timestamp.
                return true;
            } finally {
                secureWipe(old);
            }
        }

        public String exportKeyBase64(String keyId) {
            if (keyId == null) return null;
            KeyRecord rec = store.get(keyId);
            if (rec == null) return null;
            if (!rec.exportable) return null;
            return Base64.getEncoder().encodeToString(rec.keyMaterial);
        }
    }

    // Testable functions

    public static String createKey(KeyManager km, String algorithm, int bits, boolean exportable) throws Exception {
        return km.createSymmetricKey(algorithm, bits, exportable);
    }

    public static String getKeyInfo(KeyManager km, String keyId) {
        return km.getKeyInfo(keyId);
    }

    public static boolean rotateKey(KeyManager km, String keyId) throws Exception {
        return km.rotateKey(keyId);
    }

    public static boolean deleteKey(KeyManager km, String keyId) {
        return km.deleteKey(keyId);
    }

    public static String exportKey(KeyManager km, String keyId) {
        return km.exportKeyBase64(keyId);
    }

    public static List<String> listKeys(KeyManager km) {
        return km.listKeyIds();
    }

    public static void main(String[] args) throws Exception {
        KeyManager km = new KeyManager();

        // Test case 1: Create exportable AES-256 key
        String id1 = createKey(km, "AES", 256, true);
        System.out.println("Test1: Created key id1=" + id1);
        System.out.println("Info1: " + getKeyInfo(km, id1));

        // Test case 2: Create non-exportable AES-128 key
        String id2 = createKey(km, "AES", 128, false);
        System.out.println("Test2: Created key id2=" + id2);
        System.out.println("Info2: " + getKeyInfo(km, id2));

        // Test case 3: List keys
        List<String> ids = listKeys(km);
        System.out.println("Test3: List keys -> " + ids);

        // Test case 4: Export exportable key and attempt export non-exportable
        String exported1 = exportKey(km, id1);
        System.out.println("Test4a: Export id1 length=" + (exported1 != null ? exported1.length() : -1));
        String exported2 = exportKey(km, id2);
        System.out.println("Test4b: Export id2 should be null -> " + exported2);

        // Test case 5: Rotate id1 and delete id2
        boolean rotated = rotateKey(km, id1);
        System.out.println("Test5a: Rotated id1 -> " + rotated);
        boolean deleted = deleteKey(km, id2);
        System.out.println("Test5b: Deleted id2 -> " + deleted);
        System.out.println("Info2 after delete (should be null): " + getKeyInfo(km, id2));
    }
}