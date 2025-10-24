// Chain-of-Through process in code generation:
//
// 1) Problem understanding:
//    - Build a routine that accepts username and a new password, hashes the password securely,
//      and updates an in-memory database record for that user.
//    - Inputs: username (String), newPassword (char[]).
//    - Output: boolean indicating success/failure; optional verify function for testing.
//    - Database: Simulated with an in-memory Map<String, UserRecord>.
//
// 2) Security requirements:
//    - Use strong password hashing (PBKDF2WithHmacSHA256), with a per-user random salt,
//      a high iteration count, and constant-time comparison for verification.
//    - Enforce basic password strength policy.
//    - Do not print sensitive data; clear password char[] buffers when possible.
//
// 3) Secure coding generation:
//    - Implement PBKDF2 with SecureRandom-generated salt.
//    - Store algorithm metadata: iterations, salt, hash.
//    - Use constant-time comparison for verification.
//    - Validate inputs and handle null or empty values.
//    - Minimal, safe logging for tests only.
//
// 4) Code review:
//    - Verify iteration count is sufficiently high.
//    - Ensure salts are random and unique.
//    - Ensure no secret printed; wipe char[] when possible.
//    - Defensive null checks, error handling, and return statuses.
//
// 5) Secure code output:
//    - Final code below mitigates common weaknesses (e.g., no plain hashing, no static salts,
//      constant-time compare, basic password policy).
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task102 {

    // Simulated "database" record
    static final class UserRecord {
        final byte[] salt;
        final byte[] hash;
        final int iterations;
        final int keyLength; // bytes
        final String algo;   // e.g., PBKDF2WithHmacSHA256

        UserRecord(byte[] salt, byte[] hash, int iterations, int keyLength, String algo) {
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
            this.keyLength = keyLength;
            this.algo = algo;
        }
    }

    // In-memory database
    static final class InMemoryDB {
        private final Map<String, UserRecord> store = new HashMap<>();
        public synchronized UserRecord get(String username) {
            return store.get(username);
        }
        public synchronized void put(String username, UserRecord record) {
            store.put(username, record);
        }
        public synchronized boolean contains(String username) {
            return store.containsKey(username);
        }
    }

    // Secure constants
    private static final int DEFAULT_ITERATIONS = 200_000;
    private static final int SALT_LENGTH_BYTES = 16;
    private static final int KEY_LENGTH_BYTES = 32; // 256-bit
    private static final String PBKDF2_ALGO = "PBKDF2WithHmacSHA256";

    // Generate cryptographically secure random bytes
    private static byte[] secureRandomBytes(int length) {
        byte[] out = new byte[length];
        SecureRandom sr = new SecureRandom();
        sr.nextBytes(out);
        return out;
    }

    // Derive a key from password+salt
    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLengthBytes)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLengthBytes * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance(PBKDF2_ALGO);
        return skf.generateSecret(spec).getEncoded();
    }

    // Constant-time equality check
    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }

    // Basic password policy
    public static boolean isStrongPassword(String pwd) {
        if (pwd == null) return false;
        if (pwd.length() < 10) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        String specials = "!@#$%^&*()_+-=[]{}|;:'\",.<>/?`~";
        for (char c : pwd.toCharArray()) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else if (specials.indexOf(c) >= 0) hasSpecial = true;
        }
        if (!(hasUpper && hasLower && hasDigit && hasSpecial)) return false;
        // rudimentary common/weak pattern checks
        String lower = pwd.toLowerCase();
        String[] banned = {"password", "qwerty", "12345", "abc123", "letmein", "admin", "welcome"};
        for (String b : banned) {
            if (lower.contains(b)) return false;
        }
        return true;
    }

    // Hash a password and produce a user record
    public static UserRecord hashPassword(char[] newPassword) throws NoSuchAlgorithmException, InvalidKeySpecException {
        if (newPassword == null || newPassword.length == 0) {
            throw new IllegalArgumentException("Password must not be empty");
        }
        byte[] salt = secureRandomBytes(SALT_LENGTH_BYTES);
        byte[] hash = pbkdf2(newPassword, salt, DEFAULT_ITERATIONS, KEY_LENGTH_BYTES);
        // wipe password memory where feasible
        Arrays.fill(newPassword, '\0');
        return new UserRecord(salt, hash, DEFAULT_ITERATIONS, KEY_LENGTH_BYTES, PBKDF2_ALGO);
    }

    // Update (or create) the user password in DB
    // Returns true if updated/created, false on failure (e.g., weak password)
    public static boolean updateUserPassword(InMemoryDB db, String username, char[] newPassword) {
        if (db == null || username == null || username.isEmpty()) {
            if (newPassword != null) Arrays.fill(newPassword, '\0');
            return false;
        }
        try {
            String pwdStr = newPassword == null ? null : new String(newPassword);
            if (!isStrongPassword(pwdStr)) {
                if (newPassword != null) Arrays.fill(newPassword, '\0');
                return false;
            }
            UserRecord rec = hashPassword(pwdStr.toCharArray());
            db.put(username, rec);
            return true;
        } catch (Exception e) {
            if (newPassword != null) Arrays.fill(newPassword, '\0');
            return false;
        }
    }

    // Verify provided password for a user (for testing)
    public static boolean verifyPassword(InMemoryDB db, String username, char[] attempt) {
        if (db == null || username == null || attempt == null) return false;
        try {
            UserRecord rec = db.get(username);
            if (rec == null) {
                Arrays.fill(attempt, '\0');
                return false;
            }
            byte[] derived = pbkdf2(attempt, rec.salt, rec.iterations, rec.keyLength);
            boolean ok = constantTimeEquals(derived, rec.hash);
            Arrays.fill(derived, (byte) 0);
            Arrays.fill(attempt, '\0');
            return ok;
        } catch (Exception e) {
            Arrays.fill(attempt, '\0');
            return false;
        }
    }

    // Demonstration with 5 test cases
    public static void main(String[] args) {
        InMemoryDB db = new InMemoryDB();

        // Test 1: Create Alice, update to a new strong password, verify old fails and new succeeds
        boolean createdAlice = updateUserPassword(db, "alice", "Str0ng!Pass".toCharArray());
        boolean updatedAlice = updateUserPassword(db, "alice", "EvenStr0nger@2025".toCharArray());
        boolean verifyOldAlice = verifyPassword(db, "alice", "Str0ng!Pass".toCharArray());
        boolean verifyNewAlice = verifyPassword(db, "alice", "EvenStr0nger@2025".toCharArray());
        System.out.println("Test1 created=" + createdAlice + " updated=" + updatedAlice +
                " verifyOld=" + verifyOldAlice + " verifyNew=" + verifyNewAlice);

        // Test 2: Reject weak password update
        boolean weakUpdate = updateUserPassword(db, "bob", "123456".toCharArray());
        System.out.println("Test2 weakUpdate=" + weakUpdate);

        // Test 3: Create Bob with strong password, then update to same password (allowed)
        boolean createdBob = updateUserPassword(db, "bob", "Bob$Pass1X".toCharArray());
        boolean updateSameBob = updateUserPassword(db, "bob", "Bob$Pass1X".toCharArray());
        boolean verifyBob = verifyPassword(db, "bob", "Bob$Pass1X".toCharArray());
        System.out.println("Test3 createdBob=" + createdBob + " updateSameBob=" + updateSameBob + " verifyBob=" + verifyBob);

        // Test 4: Create Charlie by updating non-existent user with strong password
        boolean upsertCharlie = updateUserPassword(db, "charlie", "Good#Pass2025".toCharArray());
        boolean verifyCharlie = verifyPassword(db, "charlie", "Good#Pass2025".toCharArray());
        System.out.println("Test4 upsertCharlie=" + upsertCharlie + " verifyCharlie=" + verifyCharlie);

        // Test 5: Multiple updates for Dave and verify only last works
        boolean d1 = updateUserPassword(db, "dave", "Alpha@2025X".toCharArray());
        boolean d2 = updateUserPassword(db, "dave", "Beta#2026Y".toCharArray());
        boolean d3 = updateUserPassword(db, "dave", "Gamma$2027Z".toCharArray());
        boolean vOld1 = verifyPassword(db, "dave", "Alpha@2025X".toCharArray());
        boolean vOld2 = verifyPassword(db, "dave", "Beta#2026Y".toCharArray());
        boolean vNew = verifyPassword(db, "dave", "Gamma$2027Z".toCharArray());
        System.out.println("Test5 updates=" + (d1 && d2 && d3) + " verifyOld1=" + vOld1 + " verifyOld2=" + vOld2 + " verifyNew=" + vNew);
    }
}