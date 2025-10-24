import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task136 {

    // In-memory user store with secure password hashing (PBKDF2-HMAC-SHA256)
    static final class UserRecord {
        final byte[] salt;
        final int iterations;
        byte[] hash;
        int failedAttempts;
        boolean locked;

        UserRecord(byte[] salt, int iterations, byte[] hash) {
            this.salt = salt;
            this.iterations = iterations;
            this.hash = hash;
            this.failedAttempts = 0;
            this.locked = false;
        }
    }

    private static final Map<String, UserRecord> USERS = new ConcurrentHashMap<>();
    private static final int ITERATIONS = 120_000;
    private static final int SALT_LENGTH = 16;
    private static final int KEY_LENGTH_BITS = 256;
    private static final int MAX_FAILED_ATTEMPTS = 5;
    private static final byte[] DUMMY_SALT = secureRandomBytes(SALT_LENGTH);
    private static final int DUMMY_ITER = ITERATIONS;

    // Register a user with username and password (returns true if successful)
    public static boolean registerUser(String username, char[] password) {
        if (!isValidUsername(username) || !isValidPassword(password)) {
            wipe(password);
            return false;
        }
        byte[] salt = secureRandomBytes(SALT_LENGTH);
        try {
            byte[] hash = pbkdf2(password, salt, ITERATIONS, KEY_LENGTH_BITS);
            // Ensure we don't overwrite existing user
            UserRecord rec = new UserRecord(salt, ITERATIONS, hash);
            UserRecord existing = USERS.putIfAbsent(username, rec);
            wipe(password);
            if (existing != null) {
                // Username already exists
                return false;
            }
            return true;
        } catch (Exception e) {
            wipe(password);
            return false;
        }
    }

    // Authenticate a user (constant-time verification). Returns true if authenticated.
    public static boolean authenticateUser(String username, char[] password) {
        try {
            UserRecord rec = USERS.get(username);
            if (rec == null) {
                // Perform dummy work to mitigate username enumeration and timing attacks
                byte[] dummy = pbkdf2(password, DUMMY_SALT, DUMMY_ITER, KEY_LENGTH_BITS);
                boolean _ = constantTimeEquals(dummy, DUMMY_SALT); // no-op compare
                Arrays.fill(dummy, (byte) 0);
                wipe(password);
                return false;
            }
            if (rec.locked) {
                // Do some work anyway
                byte[] dummy = pbkdf2(password, rec.salt, rec.iterations, KEY_LENGTH_BITS);
                boolean _ = constantTimeEquals(dummy, rec.hash);
                Arrays.fill(dummy, (byte) 0);
                wipe(password);
                return false;
            }
            byte[] computed = pbkdf2(password, rec.salt, rec.iterations, KEY_LENGTH_BITS);
            boolean ok = constantTimeEquals(computed, rec.hash);
            Arrays.fill(computed, (byte) 0);
            wipe(password);
            synchronized (rec) {
                if (ok) {
                    rec.failedAttempts = 0;
                    return true;
                } else {
                    rec.failedAttempts++;
                    if (rec.failedAttempts >= MAX_FAILED_ATTEMPTS) {
                        rec.locked = true;
                    }
                    return false;
                }
            }
        } catch (Exception e) {
            wipe(password);
            return false;
        }
    }

    // Change user password (requires correct current password). Returns true if changed.
    public static boolean changePassword(String username, char[] currentPassword, char[] newPassword) {
        try {
            UserRecord rec = USERS.get(username);
            if (rec == null || rec.locked) {
                wipe(currentPassword);
                wipe(newPassword);
                return false;
            }
            byte[] computed = pbkdf2(currentPassword, rec.salt, rec.iterations, KEY_LENGTH_BITS);
            boolean ok = constantTimeEquals(computed, rec.hash);
            Arrays.fill(computed, (byte) 0);
            wipe(currentPassword);
            if (!ok || !isValidPassword(newPassword)) {
                wipe(newPassword);
                return false;
            }
            // Create new salt and hash for the new password
            byte[] newSalt = secureRandomBytes(SALT_LENGTH);
            byte[] newHash = pbkdf2(newPassword, newSalt, ITERATIONS, KEY_LENGTH_BITS);
            wipe(newPassword);
            synchronized (rec) {
                rec.failedAttempts = 0;
                rec.locked = false;
                System.arraycopy(newSalt, 0, rec.salt, 0, rec.salt.length);
                rec.hash = newHash; // replace hash
            }
            return true;
        } catch (Exception e) {
            wipe(currentPassword);
            wipe(newPassword);
            return false;
        }
    }

    // Query helpers
    public static boolean isLocked(String username) {
        UserRecord rec = USERS.get(username);
        return rec != null && rec.locked;
    }

    public static int getFailedAttempts(String username) {
        UserRecord rec = USERS.get(username);
        return rec == null ? -1 : rec.failedAttempts;
    }

    // Secure helpers
    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() < 3 || username.length() > 32) return false;
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            boolean ok = (c >= 'a' && c <= 'z') ||
                         (c >= 'A' && c <= 'Z') ||
                         (c >= '0' && c <= '9') ||
                         (c == '_');
            if (!ok) return false;
        }
        return true;
    }

    private static boolean isValidPassword(char[] password) {
        if (password == null || password.length < 8 || password.length > 128) return false;
        // basic checks: at least one letter and one digit
        boolean letter = false, digit = false;
        for (char c : password) {
            if (Character.isLetter(c)) letter = true;
            if (Character.isDigit(c)) digit = true;
        }
        return letter && digit;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLengthBits)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLengthBits);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } finally {
            spec.clearPassword();
        }
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        int len = Math.max(a.length, b.length);
        int result = 0;
        for (int i = 0; i < len; i++) {
            byte ba = i < a.length ? a[i] : 0;
            byte bb = i < b.length ? b[i] : 0;
            result |= (ba ^ bb);
        }
        return result == 0 && a.length == b.length;
    }

    private static byte[] secureRandomBytes(int len) {
        byte[] out = new byte[len];
        SecureRandom rng;
        try {
            rng = SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            rng = new SecureRandom();
        }
        rng.nextBytes(out);
        return out;
    }

    private static void wipe(char[] arr) {
        if (arr != null) Arrays.fill(arr, '\0');
    }

    public static void main(String[] args) {
        // 5 test cases

        // 1) Register and authenticate successfully
        boolean reg1 = registerUser("alice", "CorrectHorseBatteryStaple1!".toCharArray());
        boolean auth1 = authenticateUser("alice", "CorrectHorseBatteryStaple1!".toCharArray());
        System.out.println("Test1 register alice: " + reg1 + ", auth success: " + auth1);

        // 2) Wrong password attempt
        boolean authWrong = authenticateUser("alice", "WrongPassword123".toCharArray());
        System.out.println("Test2 auth with wrong password: " + authWrong + ", failed attempts: " + getFailedAttempts("alice"));

        // 3) Lock account after repeated failures
        for (int i = 0; i < 4; i++) {
            authenticateUser("alice", ("WrongPassword" + i + "1").toCharArray());
        }
        System.out.println("Test3 after failures, locked: " + isLocked("alice"));
        boolean authAfterLock = authenticateUser("alice", "CorrectHorseBatteryStaple1!".toCharArray());
        System.out.println("Test3 auth after lock (should be false): " + authAfterLock);

        // 4) Register bob and change password successfully
        boolean reg2 = registerUser("bob_01", "InitPassw0rd!".toCharArray());
        boolean change = changePassword("bob_01", "InitPassw0rd!".toCharArray(), "N3wSecur3Pass!".toCharArray());
        boolean authOld = authenticateUser("bob_01", "InitPassw0rd!".toCharArray());
        boolean authNew = authenticateUser("bob_01", "N3wSecur3Pass!".toCharArray());
        System.out.println("Test4 bob register: " + reg2 + ", change: " + change + ", old auth: " + authOld + ", new auth: " + authNew);

        // 5) Duplicate registration should fail
        boolean dup = registerUser("alice", "AnotherPass1!".toCharArray());
        System.out.println("Test5 duplicate register alice: " + dup);
    }
}