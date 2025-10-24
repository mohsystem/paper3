import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Task42 {
    // Simple user authentication system with PBKDF2-HMAC-SHA256, per-user salt, and password policy.
    public static final class AuthSystem {
        private static final int SALT_LEN = 16;
        private static final int KEY_LEN = 32; // 256-bit
        private static final int ITERATIONS = 210_000;
        private static final long PASSWORD_MAX_AGE_SECONDS = 90L * 24 * 60 * 60; // 90 days

        private static final class UserRecord {
            final byte[] salt;
            final byte[] hash;
            final int iterations;
            final long createdAtEpochSeconds;

            UserRecord(byte[] salt, byte[] hash, int iterations, long createdAtEpochSeconds) {
                this.salt = Arrays.copyOf(salt, salt.length);
                this.hash = Arrays.copyOf(hash, hash.length);
                this.iterations = iterations;
                this.createdAtEpochSeconds = createdAtEpochSeconds;
            }
        }

        private final Map<String, UserRecord> users = new HashMap<>();
        private final SecureRandom rng = new SecureRandom();

        public boolean registerUser(String username, char[] password) {
            if (!isValidUsername(username)) {
                wipeCharArray(password);
                return false;
            }
            if (!isStrongPassword(password)) {
                wipeCharArray(password);
                return false;
            }
            if (users.containsKey(username)) {
                wipeCharArray(password);
                return false;
            }
            byte[] salt = new byte[SALT_LEN];
            rng.nextBytes(salt);
            byte[] hash = null;
            try {
                hash = pbkdf2(password, salt, ITERATIONS, KEY_LEN);
                long now = Instant.now().getEpochSecond();
                users.put(username, new UserRecord(salt, hash, ITERATIONS, now));
                return true;
            } catch (Exception e) {
                return false;
            } finally {
                if (hash != null) Arrays.fill(hash, (byte) 0);
                wipeCharArray(password);
                Arrays.fill(salt, (byte) 0);
            }
        }

        // Returns: 0=success, 1=invalid credentials, 2=password expired
        public int authenticate(String username, char[] password) {
            UserRecord rec = users.get(username);
            if (rec == null) {
                wipeCharArray(password);
                return 1;
            }
            byte[] derived = null;
            try {
                derived = pbkdf2(password, rec.salt, rec.iterations, rec.hash.length);
                boolean match = MessageDigest.isEqual(derived, rec.hash);
                if (!match) {
                    return 1;
                }
                long now = Instant.now().getEpochSecond();
                if ((now - rec.createdAtEpochSeconds) > PASSWORD_MAX_AGE_SECONDS) {
                    return 2;
                }
                return 0;
            } catch (Exception e) {
                return 1;
            } finally {
                wipeCharArray(password);
                if (derived != null) Arrays.fill(derived, (byte) 0);
            }
        }

        public boolean updatePassword(String username, char[] newPassword) {
            UserRecord old = users.get(username);
            if (old == null) {
                wipeCharArray(newPassword);
                return false;
            }
            if (!isStrongPassword(newPassword)) {
                wipeCharArray(newPassword);
                return false;
            }
            byte[] salt = new byte[SALT_LEN];
            rng.nextBytes(salt);
            byte[] hash = null;
            try {
                hash = pbkdf2(newPassword, salt, ITERATIONS, KEY_LEN);
                long now = Instant.now().getEpochSecond();
                users.put(username, new UserRecord(salt, hash, ITERATIONS, now));
                return true;
            } catch (Exception e) {
                return false;
            } finally {
                if (hash != null) Arrays.fill(hash, (byte) 0);
                Arrays.fill(salt, (byte) 0);
                wipeCharArray(newPassword);
            }
        }

        // Test helper: force a user's password to be expired
        public boolean forceExpireUser(String username, long secondsAgo) {
            UserRecord rec = users.get(username);
            if (rec == null) return false;
            long created = Instant.now().getEpochSecond() - Math.abs(secondsAgo);
            users.put(username, new UserRecord(rec.salt, rec.hash, rec.iterations, created));
            return true;
        }

        private static boolean isValidUsername(String username) {
            if (username == null) return false;
            if (username.length() < 1 || username.length() > 32) return false;
            for (int i = 0; i < username.length(); i++) {
                char c = username.charAt(i);
                if (!((c >= 'A' && c <= 'Z') ||
                      (c >= 'a' && c <= 'z') ||
                      (c >= '0' && c <= '9') ||
                      c == '_')) {
                    return false;
                }
            }
            return true;
        }

        private static boolean isStrongPassword(char[] pw) {
            if (pw == null || pw.length < 12) return false;
            boolean hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
            for (char c : pw) {
                if (c >= 'a' && c <= 'z') hasLower = true;
                else if (c >= 'A' && c <= 'Z') hasUpper = true;
                else if (c >= '0' && c <= '9') hasDigit = true;
                else if (c >= 33 && c <= 126) hasSpecial = true;
            }
            return hasLower && hasUpper && hasDigit && hasSpecial;
        }

        private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLen) throws Exception {
            PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] key = skf.generateSecret(spec).getEncoded();
            spec.clearPassword();
            return key;
        }

        private static void wipeCharArray(char[] arr) {
            if (arr != null) Arrays.fill(arr, '\0');
        }
    }

    public static void main(String[] args) {
        AuthSystem auth = new AuthSystem();

        // Test 1: Register and authenticate successfully
        boolean reg1 = auth.registerUser("alice", "Str0ng!Passw0rd".toCharArray());
        int auth1 = auth.authenticate("alice", "Str0ng!Passw0rd".toCharArray());
        System.out.println("Test1 register=" + reg1 + " auth=" + auth1);

        // Test 2: Weak password rejected
        boolean reg2 = auth.registerUser("bob", "password".toCharArray());
        System.out.println("Test2 registerWeak=" + reg2);

        // Test 3: Wrong password
        boolean reg3 = auth.registerUser("carol", "C0mpl3x#Pass123".toCharArray());
        int auth3 = auth.authenticate("carol", "wrongPASS!".toCharArray());
        System.out.println("Test3 register=" + reg3 + " authWrong=" + auth3);

        // Test 4: Expired password
        boolean reg4 = auth.registerUser("dave", "Secur3!Passw0rd".toCharArray());
        boolean forced = auth.forceExpireUser("dave", 200L * 24L * 60L * 60L); // 200 days ago
        int auth4 = auth.authenticate("dave", "Secur3!Passw0rd".toCharArray());
        System.out.println("Test4 register=" + reg4 + " forced=" + forced + " authExpired=" + auth4);

        // Test 5: Update password and authenticate
        boolean reg5 = auth.registerUser("erin", "Init!Passw0rd1".toCharArray());
        boolean up5 = auth.updatePassword("erin", "New!Passw0rd2".toCharArray());
        int auth5a = auth.authenticate("erin", "New!Passw0rd2".toCharArray());
        int auth5b = auth.authenticate("erin", "Init!Passw0rd1".toCharArray());
        System.out.println("Test5 register=" + reg5 + " update=" + up5 + " authNew=" + auth5a + " authOld=" + auth5b);
    }
}