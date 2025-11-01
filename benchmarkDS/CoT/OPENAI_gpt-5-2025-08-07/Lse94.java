import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse94 {
    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32;
    private static final int ITERATIONS = 100_000;

    public static final class UserRecord {
        public final String username;
        public final byte[] salt;
        public final byte[] hash;

        public UserRecord(String username, byte[] salt, byte[] hash) {
            this.username = username;
            this.salt = salt;
            this.hash = hash;
        }
    }

    public static final class InMemoryDB {
        private final Map<String, UserRecord> users = new HashMap<>();

        public synchronized boolean insert(UserRecord rec) {
            if (users.containsKey(rec.username)) return false;
            users.put(rec.username, rec);
            return true;
        }

        public synchronized UserRecord get(String username) {
            return users.get(username);
        }

        public synchronized int size() {
            return users.size();
        }
    }

    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() < 3 || username.length() > 32) return false;
        return username.matches("^[A-Za-z0-9_]+$");
    }

    public static boolean isValidPassword(char[] password) {
        if (password == null) return false;
        if (password.length < 8) return false;
        return true;
    }

    public static byte[] generateSalt() {
        byte[] salt = new byte[SALT_LEN];
        new SecureRandom().nextBytes(salt);
        return salt;
    }

    public static byte[] hashPassword(char[] password, byte[] salt, int iterations, int keyLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        try {
            return skf.generateSecret(spec).getEncoded();
        } finally {
            spec.clearPassword();
        }
    }

    public static UserRecord createUserRecord(String username, char[] password) throws Exception {
        byte[] salt = generateSalt();
        byte[] hash = hashPassword(password, salt, ITERATIONS, HASH_LEN);
        return new UserRecord(username, salt, hash);
    }

    public static boolean registerUser(InMemoryDB db, String username, char[] password) {
        if (!isValidUsername(username) || !isValidPassword(password)) {
            Arrays.fill(password, '\0');
            return false;
        }
        try {
            UserRecord rec = createUserRecord(username, password);
            boolean ok = db.insert(rec);
            return ok;
        } catch (Exception e) {
            return false;
        } finally {
            Arrays.fill(password, '\0');
        }
    }

    private static String toB64(byte[] data) {
        return Base64.getEncoder().encodeToString(data);
    }

    public static void main(String[] args) {
        InMemoryDB db = new InMemoryDB();

        String[] usernames = {
            "alice_01",
            "bob_user",
            "charlie_3",
            "bob_user",    // duplicate
            "eve_user"
        };
        String[] passwords = {
            "S3cur3P@ssw0rd!",
            "Another$trong1",
            "passw0rdLongEnough",
            "Another$trong1", // same as bob
            "Y3t@An0th3rP@ss"
        };

        for (int i = 0; i < 5; i++) {
            String u = usernames[i];
            char[] p = passwords[i].toCharArray();
            boolean ok = registerUser(db, u, p);
            System.out.println("Register '" + u + "': " + ok);
            UserRecord rec = db.get(u);
            if (rec != null) {
                System.out.println("  salt(b64)=" + toB64(rec.salt));
                System.out.println("  hash(b64)=" + toB64(rec.hash));
            }
        }
        System.out.println("Total users stored: " + db.size());
    }
}