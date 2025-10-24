import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public final class Task122 {
    // Constants
    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 210_000;
    private static final int DK_LEN = 32; // 256-bit
    private static final SecureRandom RNG = new SecureRandom();

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,64}$");

    // User record storing salted hash only (no plaintext)
    private static final class UserRecord {
        final byte[] salt;
        final byte[] hash;
        final int iterations;

        UserRecord(byte[] salt, byte[] hash, int iterations) {
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
        }
    }

    // Password policy validation result
    private static final class PolicyResult {
        final boolean ok;
        final String message;

        PolicyResult(boolean ok, String message) {
            this.ok = ok;
            this.message = message;
        }
    }

    // In-memory store
    private static final class UserStore {
        private final Map<String, UserRecord> users = new HashMap<>();

        boolean signup(String username, char[] password) {
            try {
                if (username == null || password == null) return false;
                if (!isValidUsername(username)) return false;

                PolicyResult pr = validatePasswordPolicy(password);
                if (!pr.ok) return false;

                if (users.containsKey(username)) return false;

                byte[] salt = generateSalt(SALT_LEN);
                byte[] hash = deriveKey(password, salt, ITERATIONS, DK_LEN);
                users.put(username, new UserRecord(salt, hash, ITERATIONS));
                return true;
            } catch (Exception e) {
                return false;
            } finally {
                // Best-effort clear password
                if (password != null) Arrays.fill(password, '\0');
            }
        }

        private static boolean isValidUsername(String username) {
            if (username.length() < 1 || username.length() > 64) return false;
            return USERNAME_PATTERN.matcher(username).matches();
        }

        private static PolicyResult validatePasswordPolicy(char[] password) {
            if (password.length < 12 || password.length > 128) {
                return new PolicyResult(false, "length");
            }
            boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
            String specials = "!@#$%^&*()-_=+[]{};:'\",.<>/?`~\\|";
            for (char c : password) {
                if (Character.isUpperCase(c)) hasUpper = true;
                else if (Character.isLowerCase(c)) hasLower = true;
                else if (Character.isDigit(c)) hasDigit = true;
                else if (specials.indexOf(c) >= 0) hasSpecial = true;
                if (c == '\u0000' || Character.isISOControl(c)) {
                    return new PolicyResult(false, "invalid-char");
                }
            }
            if (!hasUpper || !hasLower || !hasDigit || !hasSpecial) {
                return new PolicyResult(false, "complexity");
            }
            return new PolicyResult(true, "ok");
        }
    }

    // Crypto helpers
    private static byte[] generateSalt(int len) {
        byte[] salt = new byte[len];
        RNG.nextBytes(salt);
        return salt;
    }

    private static byte[] deriveKey(char[] password, byte[] salt, int iterations, int dkLen) throws Exception {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] out = skf.generateSecret(spec).getEncoded();
        spec.clearPassword();
        return out;
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        UserStore store = new UserStore();

        char[][] pwds = new char[][]{
                "ValidPassw0rd!".toCharArray(),   // should succeed
                "short".toCharArray(),            // too weak
                "AnotherValid1!".toCharArray(),   // duplicate user
                "Another$trong1".toCharArray(),   // should succeed
                "NoSpecialChar1".toCharArray()    // missing special
        };
        String[] users = new String[]{
                "alice",
                "bob",
                "alice",   // duplicate
                "carol",
                "dave"
        };

        boolean[] results = new boolean[users.length];
        for (int i = 0; i < users.length; i++) {
            results[i] = store.signup(users[i], pwds[i]);
        }

        // Print concise outcomes without revealing secrets
        for (int i = 0; i < users.length; i++) {
            System.out.println(users[i] + ": " + (results[i] ? "signup ok" : "signup failed"));
        }
    }
}