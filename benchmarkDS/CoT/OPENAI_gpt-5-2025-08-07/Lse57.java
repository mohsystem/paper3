import java.util.*;
import java.util.regex.Pattern;
import java.security.SecureRandom;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

/*
Chain-of-Through in code (comments only):
1) Problem understanding:
   - Implement a simple login and email change workflow.
   - Must be logged in to change email.
   - Must provide existing old email, new email, and confirm (current) password.
   - Verify that confirm password is correct for the logged-in user whose old email is provided.
   - On success, update the email.

2) Security requirements:
   - Store passwords using salted PBKDF2 (not plaintext).
   - Use constant-time comparison for secret values.
   - Validate input (email format).
   - Avoid leaking sensitive info via logs/prints.

3) Secure coding generation:
   - Encapsulate credential handling.
   - Use SecureRandom for salts.
   - Use PBKDF2WithHmacSHA256.

4) Code review:
   - All comparisons of password hashes are constant-time.
   - Validations cover presence, format, and state (logged-in).
   - No sensitive data printed.

5) Secure code output:
   - Final code with safeguards applied and 5 test cases.
*/

public class Lse57 {

    // Simple DTO for a user
    static final class User {
        final String username;
        volatile String email; // can change
        final byte[] salt;
        final byte[] pwdHash;

        User(String username, String email, byte[] salt, byte[] pwdHash) {
            this.username = username;
            this.email = email;
            this.salt = salt;
            this.pwdHash = pwdHash;
        }
    }

    // Secure password utilities using PBKDF2
    static final class PasswordUtil {
        private static final int ITERATIONS = 120_000;
        private static final int SALT_LEN = 16;
        private static final int KEY_LEN = 32; // bytes
        private static final SecureRandom RNG = new SecureRandom();

        static byte[] newSalt() {
            byte[] s = new byte[SALT_LEN];
            RNG.nextBytes(s);
            return s;
        }

        static byte[] derive(char[] password, byte[] salt) {
            try {
                PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LEN * 8);
                SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
                return skf.generateSecret(spec).getEncoded();
            } catch (Exception e) {
                throw new RuntimeException("KDF error", e);
            }
        }

        static boolean constantTimeEquals(byte[] a, byte[] b) {
            if (a == null || b == null) return false;
            int len = Math.max(a.length, b.length);
            int result = 0;
            for (int i = 0; i < len; i++) {
                byte x = i < a.length ? a[i] : 0;
                byte y = i < b.length ? b[i] : 0;
                result |= (x ^ y);
            }
            return result == 0 && a.length == b.length;
        }
    }

    // In-memory user store
    static final class UserStore {
        private final Map<String, User> usersByUsername = new HashMap<>();
        private final Map<String, User> usersByEmail = new HashMap<>();

        synchronized boolean registerUser(String username, String email, char[] password) {
            if (username == null || username.isEmpty() || email == null || email.isEmpty() || password == null) return false;
            if (usersByUsername.containsKey(username)) return false;
            if (usersByEmail.containsKey(email.toLowerCase(Locale.ROOT))) return false;
            byte[] salt = PasswordUtil.newSalt();
            byte[] hash = PasswordUtil.derive(password, salt);
            User u = new User(username, email, salt, hash);
            usersByUsername.put(username, u);
            usersByEmail.put(email.toLowerCase(Locale.ROOT), u);
            return true;
        }

        synchronized User findByUsername(String username) {
            return usersByUsername.get(username);
        }

        synchronized User findByEmail(String email) {
            if (email == null) return null;
            return usersByEmail.get(email.toLowerCase(Locale.ROOT));
        }

        synchronized boolean updateEmail(User user, String newEmail) {
            if (user == null || newEmail == null) return false;
            String oldKey = user.email.toLowerCase(Locale.ROOT);
            String newKey = newEmail.toLowerCase(Locale.ROOT);
            if (usersByEmail.containsKey(newKey)) return false;
            usersByEmail.remove(oldKey);
            user.email = newEmail;
            usersByEmail.put(newKey, user);
            return true;
        }
    }

    // Auth and email change service
    static final class AuthService {
        private final UserStore store;
        private User currentUser;

        AuthService(UserStore store) {
            this.store = store;
        }

        boolean login(String username, char[] password) {
            if (username == null || password == null) return false;
            User u = store.findByUsername(username);
            if (u == null) return false;
            byte[] derived = PasswordUtil.derive(password, u.salt);
            if (PasswordUtil.constantTimeEquals(derived, u.pwdHash)) {
                currentUser = u;
                return true;
            }
            return false;
        }

        void logout() {
            currentUser = null;
        }

        boolean isLoggedIn() {
            return currentUser != null;
        }

        static boolean isValidEmail(String email) {
            if (email == null || email.length() > 254) return false;
            String regex = "^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}$";
            return Pattern.compile(regex, Pattern.CASE_INSENSITIVE).matcher(email).matches();
        }

        // Change email: must be logged in, must provide existing (old) email, new email, and confirm password
        boolean changeEmail(String oldEmail, String newEmail, char[] confirmPassword) {
            if (!isLoggedIn()) return false;
            if (oldEmail == null || newEmail == null || confirmPassword == null) return false;
            if (!isValidEmail(oldEmail) || !isValidEmail(newEmail)) return false;
            if (oldEmail.equalsIgnoreCase(newEmail)) return false; // no-op not allowed for this demo

            // Verify the old email belongs to the logged-in user
            if (!oldEmail.equalsIgnoreCase(currentUser.email)) return false;

            // Verify the confirm password is correct
            byte[] derived = PasswordUtil.derive(confirmPassword, currentUser.salt);
            if (!PasswordUtil.constantTimeEquals(derived, currentUser.pwdHash)) return false;

            // Ensure old email exists and new email not taken
            if (store.findByEmail(oldEmail) == null) return false;
            if (store.findByEmail(newEmail) != null) return false;

            // Update
            return store.updateEmail(currentUser, newEmail);
        }
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        UserStore store = new UserStore();
        AuthService auth = new AuthService(store);

        // Register users
        store.registerUser("alice", "alice@example.com", "StrongPass!1".toCharArray());
        store.registerUser("bob", "bob@example.com", "CorrectHorseBatteryStaple".toCharArray());

        // Test case 1: Successful login and email change
        boolean t1login = auth.login("alice", "StrongPass!1".toCharArray());
        boolean t1change = auth.changeEmail("alice@example.com", "alice.new@example.com", "StrongPass!1".toCharArray());
        System.out.println("Test1 success=" + (t1login && t1change));
        auth.logout();

        // Test case 2: Attempt change without login (should fail)
        boolean t2 = auth.changeEmail("bob@example.com", "bob.new@example.com", "CorrectHorseBatteryStaple".toCharArray());
        System.out.println("Test2 success=" + (!t2));

        // Test case 3: Wrong old email provided (should fail)
        boolean t3login = auth.login("bob", "CorrectHorseBatteryStaple".toCharArray());
        boolean t3change = auth.changeEmail("wrongbob@example.com", "bob.new@example.com", "CorrectHorseBatteryStaple".toCharArray());
        System.out.println("Test3 success=" + (t3login && !t3change));
        auth.logout();

        // Test case 4: Wrong confirm password (should fail)
        boolean t4login = auth.login("bob", "CorrectHorseBatteryStaple".toCharArray());
        boolean t4change = auth.changeEmail("bob@example.com", "bob.new@example.com", "wrongpassword".toCharArray());
        System.out.println("Test4 success=" + (t4login && !t4change));
        auth.logout();

        // Test case 5: New email already taken or invalid (should fail for taken)
        boolean t5login = auth.login("bob", "CorrectHorseBatteryStaple".toCharArray());
        boolean t5changeTaken = auth.changeEmail("bob@example.com", "alice.new@example.com", "CorrectHorseBatteryStaple".toCharArray());
        boolean t5changeInvalid = auth.changeEmail("bob@example.com", "not-an-email", "CorrectHorseBatteryStaple".toCharArray());
        System.out.println("Test5 success=" + (t5login && !t5changeTaken && !t5changeInvalid));
        auth.logout();
    }
}