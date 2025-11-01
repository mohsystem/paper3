import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse94 {

    // Security parameters
    private static final int PBKDF2_ITERATIONS = 210_000;
    private static final int SALT_LENGTH_BYTES = 16;
    private static final int DERIVED_KEY_LENGTH_BYTES = 32;

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_]{3,32}$");

    public static final class UserRecord {
        public final String username;
        public final String saltB64;
        public final String hashB64;
        public final int iterations;

        public UserRecord(String username, String saltB64, String hashB64, int iterations) {
            this.username = username;
            this.saltB64 = saltB64;
            this.hashB64 = hashB64;
            this.iterations = iterations;
        }
    }

    // In-memory database simulation
    public static final class InMemoryDatabase {
        private final Map<String, UserRecord> users = Collections.synchronizedMap(new HashMap<>());

        public boolean insert(UserRecord rec) {
            Objects.requireNonNull(rec, "record");
            synchronized (users) {
                if (users.containsKey(rec.username)) {
                    return false;
                }
                users.put(rec.username, rec);
                return true;
            }
        }

        public UserRecord get(String username) {
            synchronized (users) {
                return users.get(username);
            }
        }

        public int size() {
            synchronized (users) {
                return users.size();
            }
        }
    }

    // Public API: register user (username + password). Returns stored record on success, else null.
    public static UserRecord registerUser(InMemoryDatabase db, String username, char[] password) {
        if (db == null) {
            return null;
        }
        if (!isValidUsername(username)) {
            wipe(password);
            return null;
        }
        if (!isStrongPassword(password)) {
            wipe(password);
            return null;
        }

        byte[] salt = null;
        byte[] dk = null;
        try {
            salt = generateSalt(SALT_LENGTH_BYTES);
            dk = hashPassword(password, salt, PBKDF2_ITERATIONS, DERIVED_KEY_LENGTH_BYTES);

            String saltB64 = Base64.getEncoder().encodeToString(salt);
            String hashB64 = Base64.getEncoder().encodeToString(dk);

            UserRecord rec = new UserRecord(username, saltB64, hashB64, PBKDF2_ITERATIONS);
            boolean ok = db.insert(rec);
            if (!ok) {
                return null;
            }
            return rec;
        } catch (GeneralSecurityException e) {
            return null;
        } finally {
            wipe(password);
            wipe(salt);
            wipe(dk);
        }
    }

    // Helpers

    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        if (username.length() < 3 || username.length() > 32) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    private static boolean isStrongPassword(char[] password) {
        if (password == null || password.length < 12) return false;
        boolean hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false, hasSpace = false;
        for (char c : password) {
            if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else if (Character.isWhitespace(c)) hasSpace = true;
            else hasSpecial = true;
        }
        if (hasSpace) return false;
        return hasLower && hasUpper && hasDigit && hasSpecial;
    }

    private static byte[] generateSalt(int len) {
        byte[] salt = new byte[len];
        new SecureRandom().nextBytes(salt);
        return salt;
    }

    private static byte[] hashPassword(char[] password, byte[] salt, int iterations, int dkLen)
            throws GeneralSecurityException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
        try {
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } finally {
            spec.clearPassword();
        }
    }

    private static void wipe(byte[] data) {
        if (data != null) {
            for (int i = 0; i < data.length; i++) data[i] = 0;
        }
    }

    private static void wipe(char[] data) {
        if (data != null) {
            for (int i = 0; i < data.length; i++) data[i] = 0;
        }
    }

    public static void main(String[] args) {
        InMemoryDatabase db = new InMemoryDatabase();

        // 5 test cases
        String[] usernames = {
            "alice_01",
            "bobUser_22",
            "x",             // invalid username (too short)
            "charlie_33",
            "alice_01"       // duplicate username
        };
        String[] passwords = {
            "Sup3rStr0ng!Pwd",     // strong
            "An0ther$trongPass!",  // strong
            "Short!1A",            // weak (too short)
            "NoSpacesAllowed!9",   // strong
            "Sup3rStr0ng!Pwd"      // strong but duplicate username
        };

        for (int i = 0; i < 5; i++) {
            char[] pwd = passwords[i].toCharArray();
            UserRecord rec = registerUser(db, usernames[i], pwd);
            System.out.println("Test " + (i + 1) + " - username=" + usernames[i] + " -> " + (rec != null ? "stored" : "rejected"));
        }

        // Show how many users stored
        System.out.println("Total stored users: " + db.size());

        // Optionally show a stored record without secrets (no plaintext, only metadata)
        UserRecord alice = db.get("alice_01");
        if (alice != null) {
            System.out.println("Stored record for alice_01: iterations=" + alice.iterations +
                    ", salt(len)=" + (alice.saltB64 != null ? alice.saltB64.length() : 0) +
                    ", hash(len)=" + (alice.hashB64 != null ? alice.hashB64.length() : 0));
        }
    }
}