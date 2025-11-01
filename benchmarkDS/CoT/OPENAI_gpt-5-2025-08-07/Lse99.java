import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse99 {
    // Secure password hashing using PBKDF2 with HmacSHA256
    static class PasswordHasher {
        private static final int SALT_LEN = 16;
        private static final int ITERATIONS = 150_000;
        private static final int KEY_LENGTH_BITS = 256; // 32 bytes

        public static String hashPassword(char[] password) throws Exception {
            if (password == null) {
                throw new IllegalArgumentException("Password must not be null");
            }
            byte[] salt = new byte[SALT_LEN];
            SecureRandom sr;
            try {
                sr = SecureRandom.getInstanceStrong();
            } catch (Exception e) {
                sr = new SecureRandom();
            }
            sr.nextBytes(salt);

            PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH_BITS);
            Arrays.fill(password, '\0'); // clear incoming secret as soon as possible
            byte[] hash = null;
            try {
                SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
                hash = skf.generateSecret(spec).getEncoded();
            } finally {
                spec.clearPassword();
            }
            String encoded = "PBKDF2$" + ITERATIONS + "$" +
                    Base64.getEncoder().encodeToString(salt) + "$" +
                    Base64.getEncoder().encodeToString(hash);
            // Clear sensitive material
            Arrays.fill(salt, (byte) 0);
            Arrays.fill(hash, (byte) 0);
            return encoded;
        }
    }

    interface UserRepository {
        boolean insertUser(String username, String passwordHash);
        boolean exists(String username);
    }

    static class InMemoryUserRepository implements UserRepository {
        private final Map<String, String> store = new ConcurrentHashMap<>();

        @Override
        public boolean insertUser(String username, String passwordHash) {
            if (username == null || passwordHash == null) return false;
            return store.putIfAbsent(username, passwordHash) == null;
        }

        @Override
        public boolean exists(String username) {
            return store.containsKey(username);
        }
    }

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{3,64}$");

    public static boolean isValidUsername(String username) {
        if (username == null) return false;
        return USERNAME_PATTERN.matcher(username).matches();
    }

    public static boolean isValidPassword(char[] password) {
        if (password == null) return false;
        int len = password.length;
        // Enforce reasonable bounds
        return len >= 8 && len <= 128;
    }

    // Registers the user: validates inputs, hashes password, and inserts into repository.
    // Returns true if inserted, false otherwise.
    public static boolean registerUser(UserRepository repo, String username, char[] password) {
        if (repo == null) return false;
        try {
            if (!isValidUsername(username)) {
                return false;
            }
            if (!isValidPassword(password)) {
                Arrays.fill(password, '\0');
                return false;
            }
            String hash = PasswordHasher.hashPassword(password);
            return repo.insertUser(username, hash);
        } catch (Exception e) {
            return false;
        } finally {
            if (password != null) {
                Arrays.fill(password, '\0');
            }
        }
    }

    // 5 test cases demonstrating usage
    public static void main(String[] args) {
        InMemoryUserRepository repo = new InMemoryUserRepository();

        char[] pw1 = "StrongPass!123".toCharArray();
        System.out.println("Test1 insert alice: " + registerUser(repo, "alice", pw1));

        char[] pw2 = "Another$trong1".toCharArray();
        System.out.println("Test2 duplicate alice: " + registerUser(repo, "alice", pw2));

        char[] pw3 = "short".toCharArray();
        System.out.println("Test3 short password: " + registerUser(repo, "bob", pw3));

        char[] pw4 = "ValidPass#2025".toCharArray();
        System.out.println("Test4 invalid username: " + registerUser(repo, "in valid", pw4));

        char[] pw5 = "Y3tAn0ther$tr0ng".toCharArray();
        System.out.println("Test5 insert carol: " + registerUser(repo, "carol-01", pw5));
    }
}