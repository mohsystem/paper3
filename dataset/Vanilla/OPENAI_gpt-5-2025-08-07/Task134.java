import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.StringJoiner;

public class Task134 {
    public static class KeyManager {
        private final Map<String, byte[]> store = new HashMap<>();
        private final SecureRandom rng = new SecureRandom();

        public boolean generateKey(String alias, int lengthBytes) {
            if (alias == null || alias.isEmpty() || lengthBytes <= 0 || store.containsKey(alias)) return false;
            byte[] key = new byte[lengthBytes];
            rng.nextBytes(key);
            store.put(alias, key);
            return true;
        }

        public String exportKeyBase64(String alias) {
            byte[] key = store.get(alias);
            if (key == null) return null;
            return Base64.getEncoder().encodeToString(key);
        }

        public boolean importKeyBase64(String alias, String base64, boolean overwrite) {
            if (alias == null || alias.isEmpty() || base64 == null) return false;
            if (!overwrite && store.containsKey(alias)) return false;
            try {
                byte[] key = Base64.getDecoder().decode(base64);
                if (key.length == 0) return false;
                store.put(alias, key);
                return true;
            } catch (IllegalArgumentException e) {
                return false;
            }
        }

        public String rotateKey(String alias, int newLengthBytes) {
            if (!store.containsKey(alias) || newLengthBytes <= 0) return null;
            byte[] key = new byte[newLengthBytes];
            rng.nextBytes(key);
            store.put(alias, key);
            return Base64.getEncoder().encodeToString(key);
        }

        public boolean deleteKey(String alias) {
            return store.remove(alias) != null;
        }

        public String listAliases() {
            StringJoiner sj = new StringJoiner(",");
            for (String k : store.keySet()) sj.add(k);
            return sj.toString();
        }

        public int getKeyLength(String alias) {
            byte[] key = store.get(alias);
            return key == null ? -1 : key.length;
        }
    }

    public static void main(String[] args) {
        KeyManager km = new KeyManager();

        // Test 1: Generate and export key
        boolean t1gen = km.generateKey("api", 32);
        String t1exp = km.exportKeyBase64("api");
        System.out.println("T1 gen=" + t1gen + " len=" + km.getKeyLength("api") + " b64prefix=" + (t1exp == null ? "null" : t1exp.substring(0, Math.min(10, t1exp.length()))));

        // Test 2: Generate another and list aliases
        boolean t2gen = km.generateKey("db", 16);
        System.out.println("T2 gen=" + t2gen + " aliases=" + km.listAliases());

        // Test 3: Import a key from exported base64 under a new alias
        String exported = km.exportKeyBase64("api");
        boolean t3imp = km.importKeyBase64("imported", exported, false);
        System.out.println("T3 import=" + t3imp + " importedLen=" + km.getKeyLength("imported"));

        // Test 4: Rotate a key and check new length
        String t4rot = km.rotateKey("api", 24);
        System.out.println("T4 rotatedLen=" + km.getKeyLength("api") + " rotB64prefix=" + (t4rot == null ? "null" : t4rot.substring(0, Math.min(10, t4rot.length()))));

        // Test 5: Delete a key and list again
        boolean t5del = km.deleteKey("db");
        System.out.println("T5 delete=" + t5del + " aliases=" + km.listAliases());
    }
}