import java.nio.charset.StandardCharsets;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task116 {

    // In-memory credential store (simulation only; not writing to OS)
    private static final Map<String, String> HASH_STORE = new LinkedHashMap<>();
    private static volatile boolean PRIVILEGES_DROPPED = false;

    // Validate username: 1..32, [A-Za-z0-9_-]+
    public static boolean validateUsername(String username) {
        if (username == null) return false;
        if (username.length() < 1 || username.length() > 32) return false;
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            boolean ok = (c >= 'a' && c <= 'z') ||
                         (c >= 'A' && c <= 'Z') ||
                         (c >= '0' && c <= '9') ||
                         c == '_' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    // Validate password: 12..128, have upper, lower, digit, special
    public static boolean validatePassword(String password) {
        if (password == null) return false;
        int len = password.length();
        if (len < 12 || len > 128) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        String specials = "!@#$%^&*()-_=+[]{}|;:',.<>/?`~\"\\";
        for (int i = 0; i < len; i++) {
            char c = password.charAt(i);
            if (c >= 'A' && c <= 'Z') hasUpper = true;
            else if (c >= 'a' && c <= 'z') hasLower = true;
            else if (c >= '0' && c <= '9') hasDigit = true;
            else if (specials.indexOf(c) >= 0) hasSpecial = true;
        }
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }

    // Simulated "is root" check based on supplied current user string
    public static boolean isRootUser(String currentUser) {
        if (currentUser == null) return false;
        return "root".equalsIgnoreCase(currentUser.trim());
    }

    // Secure random salt
    public static byte[] genSalt(int length) {
        if (length < 16 || length > 64) length = 16;
        byte[] salt = new byte[length];
        SecureRandom sr = new SecureRandom();
        sr.nextBytes(salt);
        return salt;
    }

    // PBKDF2-HMAC-SHA256 hashing
    public static String hashPasswordPBKDF2(String password, byte[] salt, int iterations, int keyLenBytes) {
        if (iterations < 60000) iterations = 120000;
        if (keyLenBytes < 32) keyLenBytes = 32;
        try {
            PBEKeySpec spec = new PBEKeySpec(password.toCharArray(), salt, iterations, keyLenBytes * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] key = skf.generateSecret(spec).getEncoded();
            String hexSalt = toHex(salt);
            String hexKey = toHex(key);
            Arrays.fill(key, (byte) 0);
            return "pbkdf2_sha256$" + iterations + "$" + hexSalt + "$" + hexKey;
        } catch (NoSuchAlgorithmException | java.security.spec.InvalidKeySpecException e) {
            return null;
        }
    }

    private static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) {
            sb.append(Character.forDigit((b >>> 4) & 0xF, 16));
            sb.append(Character.forDigit(b & 0xF, 16));
        }
        return sb.toString();
    }

    // Simulated privileged password change using provided "currentUser" to decide if privileged
    public static boolean performPrivilegedPasswordChange(String currentUser, String username, String newPassword) {
        if (PRIVILEGES_DROPPED) return false; // cannot perform privileged ops after dropping
        if (!isRootUser(currentUser)) return false;
        if (!validateUsername(username)) return false;
        if (!validatePassword(newPassword)) return false;
        byte[] salt = genSalt(16);
        String encoded = hashPasswordPBKDF2(newPassword, salt, 120000, 32);
        if (encoded == null) return false;
        HASH_STORE.put(username, encoded);
        return true;
    }

    // Simulated privilege drop (Java cannot setuid without native code). After this, privileged ops are blocked.
    public static boolean dropPrivilegesSimulated(String targetUser) {
        if (targetUser == null || targetUser.isEmpty()) return false;
        PRIVILEGES_DROPPED = true;
        return true;
    }

    // Orchestrator: change password then drop privileges
    public static boolean changePasswordThenDrop(String currentUser, String targetUser, String username, String newPassword) {
        boolean changed = performPrivilegedPasswordChange(currentUser, username, newPassword);
        if (!changed) return false;
        return dropPrivilegesSimulated(targetUser);
    }

    // For demonstration: check whether we can still perform privileged operation
    public static boolean canPerformPrivilegedOps(String currentUser) {
        return isRootUser(currentUser) && !PRIVILEGES_DROPPED;
    }

    // Print a safe summary without secrets
    public static String summarize(String label, boolean changedAndDropped, boolean canPrivNow) {
        StringBuilder sb = new StringBuilder();
        sb.append(label).append(": changedAndDropped=").append(changedAndDropped)
          .append(", canPrivNow=").append(canPrivNow)
          .append(", storedUsers=").append(HASH_STORE.keySet().toString());
        return sb.toString();
    }

    public static void main(String[] args) {
        // 5 test cases
        String[] users = {"root", "root", "user", "root", "root"};
        String[] targetUsers = {"nobody", "daemon", "nobody", "nobody", "nobody"};
        String[] testUsernames = {"alice", "bob-01", "bad!name", "charlie", "d"};
        String[] testPasswords = {
            "StrongPassw0rd!",
            "Another$trongP4ss",
            "WeakPass1!",
            "ValidP@ssw0rd2025",
            "Too$hort1"
        };

        for (int i = 0; i < 5; i++) {
            // Reset simulation state per test
            PRIVILEGES_DROPPED = false;
            HASH_STORE.clear();

            boolean result = changePasswordThenDrop(users[i], targetUsers[i], testUsernames[i], testPasswords[i]);
            boolean canPriv = canPerformPrivilegedOps(users[i]);
            System.out.println(summarize("Test" + (i + 1), result, canPriv));
        }
    }
}