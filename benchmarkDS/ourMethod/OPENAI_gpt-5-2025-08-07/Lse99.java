import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse99 {

    // In-memory "database" of users
    static final class UserRecord {
        final String username;
        final byte[] salt;       // 16 bytes
        final byte[] hash;       // 32 bytes
        final int iterations;    // >=210000

        UserRecord(String username, byte[] salt, byte[] hash, int iterations) {
            this.username = username;
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
        }
    }

    static final class InMemoryDatabase {
        private final Map<String, UserRecord> users = new ConcurrentHashMap<>();
        public boolean insert(UserRecord record) {
            return users.putIfAbsent(record.username, record) == null;
        }
        public UserRecord get(String username) {
            return users.get(username);
        }
        public int count() {
            return users.size();
        }
    }

    private static final int MIN_PW_LEN = 12;
    private static final int MAX_PW_LEN = 128;
    private static final Pattern USERNAME_RE = Pattern.compile("^[A-Za-z0-9_]{3,50}$");
    private static final int PBKDF2_ITERS = 210_000;
    private static final int SALT_LEN = 16;
    private static final int KEY_LEN = 32; // 256-bit
    private static final SecureRandom RNG = new SecureRandom();

    // Hash password using PBKDF2-HMAC-SHA256 with a per-user salt
    public static byte[] hashPassword(char[] password, byte[] salt, int iterations) throws Exception {
        PBEKeySpec spec = null;
        try {
            spec = new PBEKeySpec(password, salt, iterations, KEY_LEN * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } finally {
            if (spec != null) {
                spec.clearPassword(); // zeroize password in memory where possible
            }
        }
    }

    // Validate input according to expected formats
    private static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_RE.matcher(username).matches();
    }

    private static boolean isValidPassword(char[] password) {
        if (password == null) return false;
        int len = password.length;
        if (len < MIN_PW_LEN || len > MAX_PW_LEN) return false;
        // basic complexity: at least one lower, upper, digit, and symbol
        boolean lower=false, upper=false, digit=false, other=false;
        for (char c : password) {
            if (Character.isLowerCase(c)) lower = true;
            else if (Character.isUpperCase(c)) upper = true;
            else if (Character.isDigit(c)) digit = true;
            else other = true;
        }
        return lower && upper && digit && other;
    }

    private static void secureWipe(char[] arr) {
        if (arr != null) {
            java.util.Arrays.fill(arr, '\0');
        }
    }

    // Register a user: validate inputs, hash password with unique salt, insert into "database"
    // Returns true on success, false on failure (e.g., invalid input or duplicate username)
    public static boolean registerUser(InMemoryDatabase db, String username, char[] password) {
        if (db == null) return false;
        try {
            if (!isValidUsername(username) || !isValidPassword(password)) {
                return false;
            }
            byte[] salt = new byte[SALT_LEN];
            RNG.nextBytes(salt);
            byte[] hash = hashPassword(password, salt, PBKDF2_ITERS);
            UserRecord record = new UserRecord(username, salt, hash, PBKDF2_ITERS);
            boolean inserted = db.insert(record);
            return inserted;
        } catch (Exception e) {
            return false;
        } finally {
            secureWipe(password);
        }
    }

    // Test helper to simulate registration without exposing secrets
    private static void testRegister(InMemoryDatabase db, String username, String password) {
        char[] pwChars = password.toCharArray();
        boolean ok = registerUser(db, username, pwChars);
        System.out.println("Register(" + username + "): " + (ok ? "ok" : "failed"));
    }

    public static void main(String[] args) {
        InMemoryDatabase db = new InMemoryDatabase();

        // 5 test cases
        testRegister(db, "alice_01", "StrongPass!234");
        testRegister(db, "bob", "Another$trongP4ss");
        testRegister(db, "alice_01", "DuplicateUser!234"); // duplicate username
        testRegister(db, "x", "TooShort!1");                // invalid username and password length
        testRegister(db, "charlie_3", "Val1d#PasswordThing");

        System.out.println("Total users stored: " + db.count());
    }
}