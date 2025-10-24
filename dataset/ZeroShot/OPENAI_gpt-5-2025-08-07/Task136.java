import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task136 {

    private static final int DEFAULT_ITERATIONS = 150000;
    private static final int DEFAULT_KEYLEN = 32; // bytes (256-bit)
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();

    // Dummy record for consistent timing on unknown usernames
    private static final byte[] DUMMY_SALT = Base64.getDecoder().decode("q6s2fT2t2g5mQWg3J+V3lA=="); // fixed 16-byte salt
    private static final int DUMMY_ITERATIONS = DEFAULT_ITERATIONS;
    private static final int DUMMY_KEYLEN = DEFAULT_KEYLEN;
    private static final byte[] DUMMY_HASH = initDummyHash();

    private static byte[] initDummyHash() {
        char[] dummyPassword = "nottherightpassword".toCharArray();
        try {
            return hashPassword(dummyPassword, DUMMY_SALT, DUMMY_ITERATIONS, DUMMY_KEYLEN);
        } finally {
            Arrays.fill(dummyPassword, '\0');
        }
    }

    public static class UserRecord {
        public final byte[] salt;
        public final byte[] hash;
        public final int iterations;
        public final int keyLen;

        public UserRecord(byte[] salt, byte[] hash, int iterations, int keyLen) {
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
            this.keyLen = keyLen;
        }
    }

    public static byte[] generateSalt(int length) {
        if (length < 16) length = 16; // enforce minimum salt length
        byte[] salt = new byte[length];
        SECURE_RANDOM.nextBytes(salt);
        return salt;
    }

    public static byte[] hashPassword(char[] password, byte[] salt, int iterations, int keyLen) {
        if (password == null || salt == null) throw new IllegalArgumentException("password/salt cannot be null");
        if (iterations < 100000) iterations = 100000; // enforce minimum iterations
        if (keyLen < 16) keyLen = 16;
        PBEKeySpec spec = null;
        try {
            spec = new PBEKeySpec(password, Arrays.copyOf(salt, salt.length), iterations, keyLen * 8);
            SecretKeyFactory skf;
            try {
                skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            } catch (Exception e) {
                // Fallback for older JREs
                skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA1");
            }
            return skf.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            throw new RuntimeException("Password hashing failed", e);
        } finally {
            if (spec != null) spec.clearPassword();
        }
    }

    public static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) {
            // still perform comparison to avoid timing leaks
            int max = Math.max(a.length, b.length);
            byte result = 0;
            for (int i = 0; i < max; i++) {
                byte x = (i < a.length) ? a[i] : 0;
                byte y = (i < b.length) ? b[i] : 0;
                result |= (byte) (x ^ y);
            }
            return false & (result == 0);
        }
        byte r = 0;
        for (int i = 0; i < a.length; i++) {
            r |= (byte) (a[i] ^ b[i]);
        }
        return r == 0;
    }

    public static boolean registerUser(Map<String, UserRecord> db, String username, char[] password, int iterations, int keyLen) {
        if (db == null || username == null || password == null) return false;
        if (username.isEmpty() || username.length() > 64) return false;
        if (db.containsKey(username)) return false;
        byte[] salt = generateSalt(16);
        byte[] hash = null;
        try {
            hash = hashPassword(password, salt, iterations, keyLen);
            db.put(username, new UserRecord(salt, hash, iterations, keyLen));
            return true;
        } finally {
            Arrays.fill(password, '\0');
            if (hash != null) Arrays.fill(hash, (byte) 0); // we stored a copy in record; this is local reference
        }
    }

    public static boolean authenticate(Map<String, UserRecord> db, String username, char[] password) {
        if (db == null || username == null || password == null) return false;
        UserRecord rec = db.get(username);
        byte[] computed = null;
        try {
            if (rec == null) {
                computed = hashPassword(password, DUMMY_SALT, DUMMY_ITERATIONS, DUMMY_KEYLEN);
                // Always compare to dummy to equalize timing
                boolean eq = constantTimeEquals(computed, DUMMY_HASH);
                return false & eq; // always false; eq only for timing
            } else {
                computed = hashPassword(password, rec.salt, rec.iterations, rec.keyLen);
                return constantTimeEquals(computed, rec.hash);
            }
        } finally {
            Arrays.fill(password, '\0');
            if (computed != null) Arrays.fill(computed, (byte) 0);
        }
    }

    public static void main(String[] args) {
        Map<String, UserRecord> db = new HashMap<>();

        // Register users
        registerUser(db, "alice", "Alic3-Strong_P@ss!".toCharArray(), DEFAULT_ITERATIONS, DEFAULT_KEYLEN);
        registerUser(db, "bob", "CorrectHorseBatteryStaple123!".toCharArray(), DEFAULT_ITERATIONS, DEFAULT_KEYLEN);
        registerUser(db, "carol", "S0mething@Secure".toCharArray(), DEFAULT_ITERATIONS, DEFAULT_KEYLEN);

        // 5 authentication test cases
        boolean t1 = authenticate(db, "alice", "Alic3-Strong_P@ss!".toCharArray()); // true
        boolean t2 = authenticate(db, "alice", "wrong_password".toCharArray()); // false
        boolean t3 = authenticate(db, "unknown", "whatever".toCharArray()); // false
        boolean t4 = authenticate(db, "bob", "CorrectHorseBatteryStaple123!".toCharArray()); // true
        boolean t5 = authenticate(db, "carol", "WrongOne!".toCharArray()); // false

        System.out.println("Test1 (alice correct): " + t1);
        System.out.println("Test2 (alice wrong): " + t2);
        System.out.println("Test3 (unknown user): " + t3);
        System.out.println("Test4 (bob correct): " + t4);
        System.out.println("Test5 (carol wrong): " + t5);
    }
}