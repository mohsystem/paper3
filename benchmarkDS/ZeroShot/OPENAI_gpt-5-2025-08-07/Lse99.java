import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.SecureRandom;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Base64;
import java.util.regex.Pattern;

public class Lse99 {

    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    private static final int SALT_LENGTH = 16;
    private static final int ITERATIONS = 100_000;
    private static final int KEY_LENGTH_BITS = 256; // 32 bytes

    // Simple in-memory "database"
    static class UserRecord {
        final String username;
        final byte[] salt;
        final int iterations;
        final byte[] hash;

        UserRecord(String username, byte[] salt, int iterations, byte[] hash) {
            this.username = username;
            this.salt = salt.clone();
            this.iterations = iterations;
            this.hash = hash.clone();
        }
    }

    static class Database {
        private final Map<String, UserRecord> users = new ConcurrentHashMap<>();

        public boolean insertUser(UserRecord record) {
            return users.putIfAbsent(record.username, record) == null;
        }

        public UserRecord getUser(String username) {
            return users.get(username);
        }
    }

    public static boolean registerUser(Database db, String username, char[] password) {
        if (db == null || username == null || password == null) {
            wipe(password);
            return false;
        }
        if (!isValidUsername(username) || !isValidPassword(password)) {
            wipe(password);
            return false;
        }

        byte[] salt = new byte[SALT_LENGTH];
        SECURE_RANDOM.nextBytes(salt);

        byte[] hash = null;
        try {
            hash = hashPassword(password, salt, ITERATIONS, KEY_LENGTH_BITS);
            UserRecord rec = new UserRecord(username, salt, ITERATIONS, hash);
            boolean ok = db.insertUser(rec);
            return ok;
        } catch (Exception e) {
            return false;
        } finally {
            wipe(password);
            if (hash != null) {
                wipe(hash);
            }
            wipe(salt);
        }
    }

    private static boolean isValidUsername(String username) {
        if (username.length() < 3 || username.length() > 64) return false;
        Pattern p = Pattern.compile("^[A-Za-z0-9_.-]+$");
        return p.matcher(username).matches();
    }

    private static boolean isValidPassword(char[] password) {
        return password.length >= 8;
    }

    private static byte[] hashPassword(char[] password, byte[] salt, int iterations, int keyLengthBits) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLengthBits);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } finally {
            spec.clearPassword();
        }
    }

    private static void wipe(char[] data) {
        if (data != null) java.util.Arrays.fill(data, '\0');
    }

    private static void wipe(byte[] data) {
        if (data != null) java.util.Arrays.fill(data, (byte) 0);
    }

    // For demonstration: encode record as a safe string (not storing passwords in plaintext)
    public static String describeUserRecord(Database db, String username) {
        UserRecord r = db.getUser(username);
        if (r == null) return "not found";
        return "User=" + r.username
                + ", salt=" + Base64.getEncoder().encodeToString(r.salt)
                + ", iterations=" + r.iterations
                + ", hash=" + Base64.getEncoder().encodeToString(r.hash);
    }

    public static void main(String[] args) {
        Database db = new Database();

        String[] usernames = {
                "alice_01",
                "bob-02",
                "charlie.03",
                "diana04",
                "eve05"
        };
        String[] passwords = {
                "S3cureP@ss1",
                "Another$trong2",
                "Y3tAn0therStrong!",
                "Password#WithLength123",
                "Complex&Pass99"
        };

        for (int i = 0; i < usernames.length; i++) {
            char[] pw = passwords[i].toCharArray();
            boolean ok = registerUser(db, usernames[i], pw);
            System.out.println("Register " + usernames[i] + ": " + ok);
        }

        // Show stored records for 5 test users
        for (String u : usernames) {
            System.out.println(describeUserRecord(db, u));
        }

        // Attempt duplicate insert as an additional test
        char[] dupPw = "S3cureP@ss1".toCharArray();
        boolean dup = registerUser(db, "alice_01", dupPw);
        System.out.println("Duplicate register alice_01: " + dup);
    }
}