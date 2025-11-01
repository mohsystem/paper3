// Chain-of-Through step 1: Problem understanding
// Build a simple registration form: accept username and password, validate securely, hash password with PBKDF2, return structured output with salt and hash.
// Chain-of-Through step 2: Security requirements
// - Use strong KDF (PBKDF2WithHmacSHA256) with high iterations and random salt.
// - Validate username and password strength.
// - Avoid returning raw passwords; clear sensitive data where possible.
// Chain-of-Through step 3: Secure coding generation (implemented below)
// Chain-of-Through step 4: Code review (implicit via comments and checks)
// Chain-of-Through step 5: Secure code output (final code)

import java.security.SecureRandom;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse92 {

    public static final class RegistrationResult {
        public final String username;
        public final String algorithm;
        public final int iterations;
        public final String saltB64;
        public final String hashB64;

        public RegistrationResult(String username, String algorithm, int iterations, String saltB64, String hashB64) {
            this.username = username;
            this.algorithm = algorithm;
            this.iterations = iterations;
            this.saltB64 = saltB64;
            this.hashB64 = hashB64;
        }

        @Override
        public String toString() {
            return "{"
                + "\"username\":\"" + username + "\","
                + "\"algorithm\":\"" + algorithm + "\","
                + "\"iterations\":" + iterations + ","
                + "\"salt_b64\":\"" + saltB64 + "\","
                + "\"hash_b64\":\"" + hashB64 + "\""
                + "}";
        }
    }

    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_.-]{3,32}$");
    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 200_000;
    private static final int KEY_LEN_BITS = 256;

    private static void wipeCharArray(char[] arr) {
        if (arr != null) {
            java.util.Arrays.fill(arr, '\0');
        }
    }

    private static void wipeByteArray(byte[] arr) {
        if (arr != null) {
            java.util.Arrays.fill(arr, (byte) 0);
        }
    }

    public static RegistrationResult register(String username, char[] password) throws NoSuchAlgorithmException, InvalidKeySpecException {
        // Validate inputs
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            throw new IllegalArgumentException("Invalid username. Use 3-32 chars: letters, digits, _ . -");
        }
        if (password == null || password.length < 12 || !isStrongPassword(password)) {
            throw new IllegalArgumentException("Weak password. Min 12 chars with upper, lower, digit, special.");
        }

        // Secure salt
        byte[] salt = new byte[SALT_LEN];
        new SecureRandom().nextBytes(salt);

        // Derive key using PBKDF2WithHmacSHA256
        char[] pwCopy = java.util.Arrays.copyOf(password, password.length); // local copy to wipe
        PBEKeySpec spec = new PBEKeySpec(pwCopy, salt, ITERATIONS, KEY_LEN_BITS);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] hash = null;
        try {
            hash = skf.generateSecret(spec).getEncoded();
        } finally {
            spec.clearPassword();
            wipeCharArray(pwCopy);
        }

        String saltB64 = Base64.getEncoder().encodeToString(salt);
        String hashB64 = Base64.getEncoder().encodeToString(hash);

        wipeByteArray(salt); // optional: not needed if storing, but we return encoded values
        wipeByteArray(hash);

        return new RegistrationResult(username, "PBKDF2-HMAC-SHA256", ITERATIONS, saltB64, hashB64);
    }

    private static boolean isStrongPassword(char[] password) {
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        for (char c : password) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSpecial = true;
        }
        return password.length >= 12 && hasUpper && hasLower && hasDigit && hasSpecial;
    }

    // 5 test cases
    public static void main(String[] args) throws Exception {
        String[] usernames = { "alice_1", "Bob-User", "charlie.user", "dave_2025", "eve.test" };
        String[] passwords = {
            "StrongPassw0rd!",
            "Another$tr0ngPass!",
            "Y3tAn0ther#Pass!",
            "S0methingComp!ex",
            "Unbr3akable@Key!"
        };

        for (int i = 0; i < 5; i++) {
            try {
                RegistrationResult res = register(usernames[i], passwords[i].toCharArray());
                System.out.println(res.toString());
            } catch (IllegalArgumentException ex) {
                System.out.println("{\"error\":\"" + ex.getMessage().replace("\"", "\\\"") + "\"}");
            }
        }
    }
}