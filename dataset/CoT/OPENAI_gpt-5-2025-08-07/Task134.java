import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.GCMParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.*;

/*
High-level notes:
- Implements secure in-memory key management with generation, retrieval (copy), rotation, deletion, and listing.
- Uses strong RNG (SecureRandom).
- AES-GCM helpers provided for sample usage; IV is 12 bytes, tag is 128 bits.
- Thread-safe via synchronized methods. Keys are zeroized on deletion. Returned keys are copies to avoid exposure.
*/
public class Task134 {

    private static final class KeyRecord {
        final String label;
        int version;
        byte[] key; // AES-256 key bytes
        final long createdAt;

        KeyRecord(String label, int version, byte[] key) {
            this.label = label;
            this.version = version;
            this.key = key;
            this.createdAt = System.currentTimeMillis();
        }
    }

    public static final class KeyManager {
        private final Map<String, KeyRecord> store = new HashMap<>();
        private final SecureRandom rng = new SecureRandom();

        // Generate a new key with given label. If label exists, returns existing version without modifying.
        public synchronized int createKey(String label, int sizeBytes) throws GeneralSecurityException {
            Objects.requireNonNull(label, "label");
            if (label.isEmpty()) throw new GeneralSecurityException("Label must not be empty");
            if (sizeBytes < 16) throw new GeneralSecurityException("Key size too small");
            if (store.containsKey(label)) {
                return store.get(label).version;
            }
            byte[] key = new byte[sizeBytes];
            rng.nextBytes(key);
            store.put(label, new KeyRecord(label, 1, key));
            return 1;
        }

        // Retrieves a copy of the key for the label, or null if not found
        public synchronized byte[] getKeyCopy(String label) {
            KeyRecord rec = store.get(label);
            if (rec == null || rec.key == null) return null;
            return Arrays.copyOf(rec.key, rec.key.length);
        }

        // Rotates the key: generates a new random key, increments version
        public synchronized int rotateKey(String label, int sizeBytes) throws GeneralSecurityException {
            Objects.requireNonNull(label, "label");
            if (sizeBytes < 16) throw new GeneralSecurityException("Key size too small");
            KeyRecord rec = store.get(label);
            if (rec == null) throw new GeneralSecurityException("Label not found");
            byte[] newKey = new byte[sizeBytes];
            rng.nextBytes(newKey);
            zeroize(rec.key);
            rec.key = newKey;
            rec.version += 1;
            return rec.version;
        }

        // Deletes the key and zeroizes memory
        public synchronized boolean deleteKey(String label) {
            KeyRecord rec = store.remove(label);
            if (rec == null) return false;
            if (rec.key != null) zeroize(rec.key);
            return true;
        }

        // Lists labels with versions
        public synchronized List<String> listKeys() {
            List<String> out = new ArrayList<>(store.size());
            for (KeyRecord rec : store.values()) {
                out.add(rec.label + ":v" + rec.version);
            }
            out.sort(Comparator.naturalOrder());
            return out;
        }

        // AES-GCM encrypt using key of label; returns iv || ciphertext+tag
        public synchronized byte[] encrypt(String label, byte[] plaintext, byte[] aad) throws GeneralSecurityException {
            KeyRecord rec = store.get(label);
            if (rec == null || rec.key == null) throw new GeneralSecurityException("Key not found");
            byte[] iv = new byte[12];
            rng.nextBytes(iv);
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec spec = new GCMParameterSpec(128, iv);
            SecretKey key = new SecretKeySpec(rec.key, "AES");
            cipher.init(Cipher.ENCRYPT_MODE, key, spec);
            if (aad != null && aad.length > 0) cipher.updateAAD(aad);
            byte[] ct = cipher.doFinal(plaintext);
            byte[] out = new byte[iv.length + ct.length];
            System.arraycopy(iv, 0, out, 0, iv.length);
            System.arraycopy(ct, 0, out, iv.length, ct.length);
            return out;
        }

        // AES-GCM decrypt given iv||ciphertext+tag
        public synchronized byte[] decrypt(String label, byte[] ivAndCiphertext, byte[] aad) throws GeneralSecurityException {
            KeyRecord rec = store.get(label);
            if (rec == null || rec.key == null) throw new GeneralSecurityException("Key not found");
            if (ivAndCiphertext == null || ivAndCiphertext.length < 12 + 16) {
                throw new GeneralSecurityException("Ciphertext too short");
            }
            byte[] iv = Arrays.copyOfRange(ivAndCiphertext, 0, 12);
            byte[] ct = Arrays.copyOfRange(ivAndCiphertext, 12, ivAndCiphertext.length);
            Cipher cipher = Cipher.getInstance("AES/GCM/NoPadding");
            GCMParameterSpec spec = new GCMParameterSpec(128, iv);
            SecretKey key = new SecretKeySpec(rec.key, "AES");
            cipher.init(Cipher.DECRYPT_MODE, key, spec);
            if (aad != null && aad.length > 0) cipher.updateAAD(aad);
            return cipher.doFinal(ct);
        }

        private static void zeroize(byte[] data) {
            if (data == null) return;
            Arrays.fill(data, (byte) 0);
        }
    }

    // Utility for hex printing in tests
    private static String toHex(byte[] data) {
        if (data == null) return "null";
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) sb.append(String.format("%02x", b));
        return sb.toString();
    }

    public static void main(String[] args) throws Exception {
        KeyManager km = new KeyManager();

        // Test 1: Create key 'alpha' and list
        int v1 = km.createKey("alpha", 32);
        System.out.println("Created alpha v" + v1 + " keys=" + km.listKeys());

        // Test 2: Encrypt/Decrypt with 'alpha'
        byte[] ct1 = km.encrypt("alpha", "hello world".getBytes(StandardCharsets.UTF_8), "aad".getBytes(StandardCharsets.UTF_8));
        byte[] pt1 = km.decrypt("alpha", ct1, "aad".getBytes(StandardCharsets.UTF_8));
        System.out.println("ct1=" + toHex(ct1) + " pt1=" + new String(pt1, StandardCharsets.UTF_8));

        // Test 3: Rotate 'alpha' and encrypt again, old ct should fail with new key if AAD differs
        int v2 = km.rotateKey("alpha", 32);
        System.out.println("Rotated alpha to v" + v2);
        byte[] ct2 = km.encrypt("alpha", "secure msg".getBytes(StandardCharsets.UTF_8), null);
        byte[] pt2 = km.decrypt("alpha", ct2, null);
        System.out.println("ct2=" + toHex(ct2) + " pt2=" + new String(pt2, StandardCharsets.UTF_8));
        try {
            // Attempt to decrypt old ciphertext with rotated key and wrong AAD to demonstrate auth failure
            km.decrypt("alpha", ct1, "wrong".getBytes(StandardCharsets.UTF_8));
            System.out.println("Unexpected: old ct decrypted");
        } catch (GeneralSecurityException e) {
            System.out.println("Expected auth failure on old ct with rotated key/AAD");
        }

        // Test 4: Create 'beta' and list
        km.createKey("beta", 32);
        System.out.println("Keys now: " + km.listKeys());

        // Test 5: Delete 'alpha' and verify missing
        boolean del = km.deleteKey("alpha");
        System.out.println("Deleted alpha? " + del + " getKey=" + Arrays.toString(km.getKeyCopy("alpha")));
    }
}