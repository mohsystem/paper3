import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.Objects;
import java.util.regex.Pattern;

public class Lse56 {

    // Chain-of-Through Step 1: Problem understanding and structure
    // - Implement a simple authentication and email change flow.
    // - Requirements: user must be logged in; must supply oldEmail, newEmail, confirmPassword; password must be correct; newEmail must differ from oldEmail.
    // - Inputs as parameters, outputs as return values, no interactive input.
    // Chain-of-Through Step 2: Security requirements
    // - Use salted hashing (SHA-256) for password storage.
    // - Constant-time comparison for hashes.
    // - Validate email format and reasonable lengths.
    // - Avoid leaking sensitive info in messages.
    // Chain-of-Through Step 3: Secure coding generation
    // - Implement below.
    // Chain-of-Through Step 4: Code review (embedded via comments and checks).
    // Chain-of-Through Step 5: Secure code output (finalized code).

    static final class User {
        private final String username;
        private volatile String email;
        private final byte[] salt;
        private final byte[] passwordHash;

        User(String username, String email, byte[] salt, byte[] passwordHash) {
            this.username = username;
            this.email = email;
            this.salt = Arrays.copyOf(salt, salt.length);
            this.passwordHash = Arrays.copyOf(passwordHash, passwordHash.length);
        }
        public String getUsername() { return username; }
        public String getEmail() { return email; }
        public void setEmail(String newEmail) { this.email = newEmail; }
        public byte[] getSalt() { return Arrays.copyOf(salt, salt.length); }
        public byte[] getPasswordHash() { return Arrays.copyOf(passwordHash, passwordHash.length); }
    }

    static final class AuthService {
        private User currentUser; // logged-in user or null
        private static final int MAX_EMAIL_LEN = 254;
        private static final Pattern EMAIL_REGEX = Pattern.compile("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,189}\\.[A-Za-z]{2,24}$");

        public AuthService() {
            this.currentUser = null;
        }

        public static byte[] generateSalt(int len) {
            SecureRandom sr = new SecureRandom();
            byte[] salt = new byte[len];
            sr.nextBytes(salt);
            return salt;
        }

        public static byte[] sha256(byte[] data) {
            try {
                MessageDigest md = MessageDigest.getInstance("SHA-256");
                return md.digest(data);
            } catch (NoSuchAlgorithmException e) {
                throw new RuntimeException("SHA-256 not available", e);
            }
        }

        public static byte[] hashPassword(byte[] salt, char[] password) {
            // Simple salted hash: SHA256(salt || UTF-8(password) || salt), iterated to slow slightly.
            byte[] pwdBytes = new String(password).getBytes(java.nio.charset.StandardCharsets.UTF_8);
            byte[] input = new byte[salt.length + pwdBytes.length + salt.length];
            System.arraycopy(salt, 0, input, 0, salt.length);
            System.arraycopy(pwdBytes, 0, input, salt.length, pwdBytes.length);
            System.arraycopy(salt, 0, input, salt.length + pwdBytes.length, salt.length);
            byte[] out = sha256(input);
            // small number of extra iterations
            for (int i = 0; i < 999; i++) {
                out = sha256(xorConcat(out, salt));
            }
            Arrays.fill(pwdBytes, (byte)0);
            Arrays.fill(input, (byte)0);
            return out;
        }

        private static byte[] xorConcat(byte[] a, byte[] b) {
            byte[] res = new byte[a.length + b.length];
            System.arraycopy(a, 0, res, 0, a.length);
            System.arraycopy(b, 0, res, a.length, b.length);
            for (int i = 0; i < res.length; i++) {
                res[i] ^= (byte)(i * 31);
            }
            return res;
        }

        public static boolean constantTimeEquals(byte[] a, byte[] b) {
            if (a == null || b == null) return false;
            int len = Math.max(a.length, b.length);
            int diff = 0;
            for (int i = 0; i < len; i++) {
                byte ba = i < a.length ? a[i] : 0;
                byte bb = i < b.length ? b[i] : 0;
                diff |= (ba ^ bb);
            }
            return diff == 0 && a.length == b.length;
        }

        public static boolean isValidEmail(String email) {
            if (email == null) return false;
            String trimmed = email.trim();
            if (trimmed.length() == 0 || trimmed.length() > MAX_EMAIL_LEN) return false;
            return EMAIL_REGEX.matcher(trimmed).matches();
        }

        public static User registerDefaultUser(String username, String email, String password) {
            Objects.requireNonNull(username);
            Objects.requireNonNull(email);
            Objects.requireNonNull(password);
            byte[] salt = generateSalt(16);
            byte[] hash = hashPassword(salt, password.toCharArray());
            return new User(username, email, salt, hash);
        }

        public boolean login(User user, String username, String password) {
            if (user == null || username == null || password == null) return false;
            if (!user.getUsername().equals(username)) return false;
            byte[] expected = user.getPasswordHash();
            byte[] actual = hashPassword(user.getSalt(), password.toCharArray());
            boolean ok = constantTimeEquals(expected, actual);
            Arrays.fill(actual, (byte)0);
            if (ok) {
                this.currentUser = user;
                return true;
            }
            return false;
        }

        public void logout() {
            this.currentUser = null;
        }

        // Returns a message describing result but avoids leaking sensitive details
        public String changeEmail(String oldEmail, String newEmail, String confirmPassword) {
            if (this.currentUser == null) {
                return "Error: You must be logged in to change your email.";
            }
            if (oldEmail == null || newEmail == null || confirmPassword == null) {
                return "Error: Missing required fields.";
            }
            String oldE = oldEmail.trim();
            String newE = newEmail.trim();
            if (!isValidEmail(oldE) || !isValidEmail(newE)) {
                return "Error: Invalid email format.";
            }
            if (!this.currentUser.getEmail().equals(oldE)) {
                return "Error: The provided old email does not match our records.";
            }
            if (oldE.equalsIgnoreCase(newE)) {
                return "Error: New email must be different from old email.";
            }
            // verify password
            byte[] actual = hashPassword(this.currentUser.getSalt(), confirmPassword.toCharArray());
            boolean ok = constantTimeEquals(this.currentUser.getPasswordHash(), actual);
            Arrays.fill(actual, (byte)0);
            if (!ok) {
                return "Error: Invalid credentials.";
            }
            this.currentUser.setEmail(newE);
            return "Success: Email changed to " + newE + ".";
        }

        public static String userSnapshot(User u) {
            if (u == null) return "null";
            return "User(username=" + u.getUsername() + ", email=" + u.getEmail() + ", salt=" + Base64.getEncoder().encodeToString(u.getSalt()) + ")";
        }
    }

    public static void main(String[] args) {
        // Create a default user
        User user = AuthService.registerDefaultUser("alice", "alice@old.com", "P@ssw0rd!");
        AuthService auth = new AuthService();

        // Test 1: Successful login and email change
        boolean login1 = auth.login(user, "alice", "P@ssw0rd!");
        String res1 = auth.changeEmail("alice@old.com", "alice@new.com", "P@ssw0rd!");
        System.out.println("T1 login=" + login1 + " -> " + res1 + " | " + AuthService.userSnapshot(user));
        auth.logout();

        // Test 2: Attempt to change email while not logged in
        String res2 = auth.changeEmail("alice@new.com", "alice@newer.com", "P@ssw0rd!");
        System.out.println("T2 -> " + res2 + " | " + AuthService.userSnapshot(user));

        // Test 3: Wrong password
        boolean login3 = auth.login(user, "alice", "P@ssw0rd!");
        String res3 = auth.changeEmail("alice@new.com", "alice@fresh.com", "WRONGPASS");
        System.out.println("T3 login=" + login3 + " -> " + res3 + " | " + AuthService.userSnapshot(user));
        auth.logout();

        // Test 4: New email equals old email
        boolean login4 = auth.login(user, "alice", "P@ssw0rd!");
        String res4 = auth.changeEmail("alice@new.com", "alice@new.com", "P@ssw0rd!");
        System.out.println("T4 login=" + login4 + " -> " + res4 + " | " + AuthService.userSnapshot(user));
        auth.logout();

        // Test 5: Old email mismatch
        boolean login5 = auth.login(user, "alice", "P@ssw0rd!");
        String res5 = auth.changeEmail("wrong@old.com", "alice@final.com", "P@ssw0rd!");
        System.out.println("T5 login=" + login5 + " -> " + res5 + " | " + AuthService.userSnapshot(user));
        auth.logout();
    }
}