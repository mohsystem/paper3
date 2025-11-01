import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse58 {

    // Simple in-memory user model with secure password hashing
    static final class User {
        private final String username;
        private String email;
        private byte[] salt;        // 16 bytes
        private byte[] pwdHash;     // 32 bytes (256-bit)
        private boolean loggedIn;

        User(String username, String email) {
            this.username = username;
            this.email = email;
            this.loggedIn = false;
        }

        public String getEmail() {
            return email;
        }

        public boolean isLoggedIn() {
            return loggedIn;
        }

        public String getUsername() {
            return username;
        }
    }

    // Security utilities
    static final class Security {
        private static final int SALT_LEN = 16;
        private static final int HASH_LEN = 32;
        private static final int PBKDF2_ITERATIONS = 210000;

        private static final SecureRandom RNG = new SecureRandom();

        static byte[] generateSalt() {
            byte[] s = new byte[SALT_LEN];
            RNG.nextBytes(s);
            return s;
        }

        static boolean isPasswordStrong(char[] password) {
            if (password == null || password.length < 12) return false;
            boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
            for (char c : password) {
                if (Character.isUpperCase(c)) hasUpper = true;
                else if (Character.isLowerCase(c)) hasLower = true;
                else if (Character.isDigit(c)) hasDigit = true;
                else hasSpecial = true;
            }
            return hasUpper && hasLower && hasDigit && hasSpecial;
        }

        static boolean isEmailValid(String email) {
            if (email == null) return false;
            if (email.length() < 5 || email.length() > 254) return false;
            // Simple RFC-like pattern
            String regex = "^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$";
            return Pattern.compile(regex).matcher(email).matches();
        }

        static byte[] pbkdf2(char[] password, byte[] salt) throws GeneralSecurityException {
            PBEKeySpec spec = new PBEKeySpec(password, salt, PBKDF2_ITERATIONS, HASH_LEN * 8);
            try {
                SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
                return skf.generateSecret(spec).getEncoded();
            } finally {
                spec.clearPassword();
            }
        }

        static void zeroize(char[] arr) {
            if (arr != null) Arrays.fill(arr, '\0');
        }

        static void zeroize(byte[] arr) {
            if (arr != null) Arrays.fill(arr, (byte) 0);
        }
    }

    // Simple auth service
    static final class AuthService {

        static boolean setPassword(User user, char[] newPassword) {
            if (user == null || newPassword == null) return false;
            if (!Security.isPasswordStrong(newPassword)) {
                return false;
            }
            byte[] salt = null;
            byte[] hash = null;
            try {
                salt = Security.generateSalt();
                hash = Security.pbkdf2(newPassword, salt);
                user.salt = salt;
                user.pwdHash = hash;
                return true;
            } catch (GeneralSecurityException e) {
                return false;
            } finally {
                Security.zeroize(newPassword);
                // Do not zeroize salt/hash assigned to user
            }
        }

        static boolean verifyPassword(User user, char[] password) {
            if (user == null || password == null || user.salt == null || user.pwdHash == null) return false;
            byte[] derived = null;
            try {
                derived = Security.pbkdf2(password, user.salt);
                boolean eq = constantTimeEquals(user.pwdHash, derived);
                return eq;
            } catch (GeneralSecurityException e) {
                return false;
            } finally {
                Security.zeroize(password);
                Security.zeroize(derived);
            }
        }

        static boolean login(User user, String username, char[] password) {
            if (user == null || username == null || password == null) return false;
            if (!username.equals(user.getUsername())) {
                Security.zeroize(password);
                return false;
            }
            boolean ok = verifyPassword(user, password);
            if (ok) {
                user.loggedIn = true;
            }
            return ok;
        }

        static boolean changeEmail(User user, String oldEmail, String newEmail, char[] confirmPassword) {
            // Enforce: must be logged in, old email matches, password correct, new email valid
            if (user == null || oldEmail == null || newEmail == null || confirmPassword == null) return false;
            if (!user.isLoggedIn()) {
                Security.zeroize(confirmPassword);
                return false;
            }
            if (!oldEmail.equals(user.getEmail())) {
                Security.zeroize(confirmPassword);
                return false;
            }
            if (!Security.isEmailValid(newEmail)) {
                Security.zeroize(confirmPassword);
                return false;
            }
            boolean ok = verifyPassword(user, confirmPassword);
            if (!ok) {
                return false;
            }
            user.email = newEmail;
            return true;
        }

        private static boolean constantTimeEquals(byte[] a, byte[] b) {
            if (a == null || b == null || a.length != b.length) return false;
            int res = 0;
            for (int i = 0; i < a.length; i++) {
                res |= a[i] ^ b[i];
            }
            return res == 0;
        }
    }

    // Generate a strong random password that satisfies our policy
    static char[] generateStrongPassword() {
        SecureRandom rng = new SecureRandom();
        String upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        String lower = "abcdefghijklmnopqrstuvwxyz";
        String digits = "0123456789";
        String special = "!@#$%^&*()-_=+[]{};:,.?/|";
        String all = upper + lower + digits + special;

        int length = 16;
        char[] out = new char[length];
        out[0] = upper.charAt(rng.nextInt(upper.length()));
        out[1] = lower.charAt(rng.nextInt(lower.length()));
        out[2] = digits.charAt(rng.nextInt(digits.length()));
        out[3] = special.charAt(rng.nextInt(special.length()));
        for (int i = 4; i < length; i++) {
            out[i] = all.charAt(rng.nextInt(all.length()));
        }
        // shuffle
        for (int i = length - 1; i > 0; i--) {
            int j = rng.nextInt(i + 1);
            char tmp = out[i];
            out[i] = out[j];
            out[j] = tmp;
        }
        return out;
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        User user = new User("alice", "alice@example.com");
        char[] initialPassword = generateStrongPassword();
        boolean setOK = AuthService.setPassword(user, Arrays.copyOf(initialPassword, initialPassword.length));
        if (!setOK) {
            System.out.println("Setup failed");
            return;
        }

        // Test 1: Attempt change email while not logged in -> should fail (false)
        boolean t1 = AuthService.changeEmail(user, "alice@example.com", "newalice@example.com",
                Arrays.copyOf(initialPassword, initialPassword.length));
        System.out.println("Test1 change while not logged in: " + t1);

        // Test 2: Login with correct password -> should succeed (true)
        boolean t2 = AuthService.login(user, "alice", Arrays.copyOf(initialPassword, initialPassword.length));
        System.out.println("Test2 login with correct password: " + t2);

        // Test 3: Change with wrong old email after login -> should fail (false)
        boolean t3 = AuthService.changeEmail(user, "wrong@example.com", "newalice@example.com",
                Arrays.copyOf(initialPassword, initialPassword.length));
        System.out.println("Test3 change with wrong old email: " + t3);

        // Test 4: Change with wrong password -> should fail (false)
        char[] wrongPass = "WrongPass!234".toCharArray();
        boolean t4 = AuthService.changeEmail(user, "alice@example.com", "newalice@example.com",
                Arrays.copyOf(wrongPass, wrongPass.length));
        Security.zeroize(wrongPass);
        System.out.println("Test4 change with wrong password: " + t4);

        // Test 5: Change with correct old email and password -> should succeed (true)
        boolean t5 = AuthService.changeEmail(user, "alice@example.com", "newalice@example.com",
                Arrays.copyOf(initialPassword, initialPassword.length));
        System.out.println("Test5 change with correct old email and password: " + t5);

        // Cleanup sensitive in-memory password
        Security.zeroize(initialPassword);
    }
}