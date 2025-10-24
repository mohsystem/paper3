import java.security.MessageDigest;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.time.Instant;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task133 {
    // Chain-of-Through Step 1: Problem understanding
    // Implement in-memory password reset with secure token issuance, validation, and password policy.

    // User record
    static class User {
        String email;
        byte[] salt;
        byte[] hash;
        int iterations;
        byte[] tokenHash; // SHA-256(token)
        long tokenExpiryMs; // epoch millis

        User(String email, byte[] salt, byte[] hash, int iterations) {
            this.email = email;
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
            this.tokenHash = null;
            this.tokenExpiryMs = 0L;
        }
    }

    // Storage
    private static final Map<String, User> USERS = new HashMap<>();
    private static final SecureRandom RNG = new SecureRandom();
    private static final int SALT_LEN = 16;
    private static final int ITERATIONS = 100_000;
    private static final int KEY_LEN = 32; // 256-bit
    private static final long TOKEN_TTL_MS = 10 * 60 * 1000L; // 10 minutes

    // Chain-of-Through Step 2: Security requirements
    // - Use PBKDF2-HMAC-SHA256 for password hashing.
    // - Generate cryptographically secure tokens.
    // - Store only token hash, not raw token.
    // - Constant-time comparisons for sensitive data.
    // - Enforce strong password policy.
    // - Clear sensitive arrays where possible.

    // Create user securely
    public static boolean createUser(String email, String password) {
        if (email == null || password == null) return false;
        if (USERS.containsKey(email)) return false;
        if (!isPasswordStrong(password)) return false;
        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        byte[] hash = pbkdf2(password.toCharArray(), salt, ITERATIONS, KEY_LEN);
        USERS.put(email, new User(email, salt, hash, ITERATIONS));
        // Best-effort wipe
        zeroize(hash);
        return true;
    }

    // Request a reset token; returns a URL-safe token string or null
    public static String requestReset(String email) {
        User u = USERS.get(email);
        if (u == null) return null;
        byte[] token = new byte[32];
        RNG.nextBytes(token);
        String tokenStr = base64Url(token);
        byte[] tokenHash = sha256(tokenStr.getBytes(java.nio.charset.StandardCharsets.UTF_8));
        u.tokenHash = tokenHash;
        u.tokenExpiryMs = Instant.now().toEpochMilli() + TOKEN_TTL_MS;
        // wipe token bytes
        zeroize(token);
        return tokenStr;
    }

    // Reset password using token
    public static boolean resetPassword(String token, String newPassword) {
        if (token == null || newPassword == null) return false;
        byte[] tokHash = sha256(token.getBytes(java.nio.charset.StandardCharsets.UTF_8));
        long now = Instant.now().toEpochMilli();
        User matched = null;
        for (User u : USERS.values()) {
            if (u.tokenHash != null && constantTimeEquals(u.tokenHash, tokHash)) {
                matched = u;
                break;
            }
        }
        zeroize(tokHash);
        if (matched == null) return false;
        if (matched.tokenExpiryMs < now) {
            matched.tokenHash = null;
            matched.tokenExpiryMs = 0L;
            return false;
        }
        if (!isPasswordStrong(newPassword)) {
            return false;
        }
        // Re-hash with new salt
        byte[] salt = new byte[SALT_LEN];
        RNG.nextBytes(salt);
        byte[] hash = pbkdf2(newPassword.toCharArray(), salt, ITERATIONS, KEY_LEN);
        matched.salt = salt;
        matched.hash = hash;
        matched.iterations = ITERATIONS;
        // Invalidate token
        matched.tokenHash = null;
        matched.tokenExpiryMs = 0L;
        return true;
    }

    // Password policy
    public static boolean isPasswordStrong(String p) {
        if (p == null) return false;
        if (p.length() < 10 || p.length() > 100) return false;
        boolean up=false, lo=false, di=false, sp=false;
        for (int i=0;i<p.length();i++) {
            char c = p.charAt(i);
            if (Character.isUpperCase(c)) up = true;
            else if (Character.isLowerCase(c)) lo = true;
            else if (Character.isDigit(c)) di = true;
            else sp = true;
        }
        if (!(up && lo && di && sp)) return false;
        String lower = p.toLowerCase();
        // common bad patterns to discourage
        String[] bad = {"password", "12345", "qwerty", "letmein", "admin"};
        for (String b : bad) if (lower.contains(b)) return false;
        return true;
    }

    // PBKDF2-HMAC-SHA256
    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int keyLen) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, iterations, keyLen * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] out = skf.generateSecret(spec).getEncoded();
            return out;
        } catch (Exception e) {
            throw new RuntimeException("PBKDF2 error", e);
        } finally {
            // wipe password char[]
            java.util.Arrays.fill(password, '\0');
        }
    }

    private static byte[] sha256(byte[] data) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            return md.digest(data);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) {
            // compare anyway to avoid length leak
            int len = Math.max(a.length, b.length);
            int result = 0;
            for (int i=0;i<len;i++) {
                byte x = i < a.length ? a[i] : 0;
                byte y = i < b.length ? b[i] : 0;
                result |= (x ^ y);
            }
            return false;
        }
        int res = 0;
        for (int i=0;i<a.length;i++) res |= (a[i] ^ b[i]);
        return res == 0;
    }

    private static String base64Url(byte[] data) {
        return Base64.getUrlEncoder().withoutPadding().encodeToString(data);
    }

    private static void zeroize(byte[] arr) {
        if (arr != null) java.util.Arrays.fill(arr, (byte)0);
    }

    // Chain-of-Through Step 4 & 5: Review and output done via secure coding choices above.

    // Main with 5 test cases
    public static void main(String[] args) {
        System.out.println("Java tests:");
        // 1) Unknown user reset request
        String t1 = requestReset("unknown@example.com");
        System.out.println("Test1 token for unknown user should be null: " + t1);

        // 2) Create user and request reset, then try wrong token
        boolean created = createUser("alice@example.com", "OldPassw0rd!");
        System.out.println("Test2 created user: " + created);
        String token = requestReset("alice@example.com");
        System.out.println("Test2 got token: " + (token != null));
        boolean wrong = resetPassword("badtoken", "NewPassw0rd!");
        System.out.println("Test2 reset with wrong token: " + wrong);

        // 3) Try weak password
        boolean weak = resetPassword(token, "weak");
        System.out.println("Test3 reset with weak password: " + weak);

        // 4) Successful reset
        boolean ok = resetPassword(token, "N3w_Str0ngPass!");
        System.out.println("Test4 reset with correct token and strong password: " + ok);

        // 5) Reuse token should fail; request new token and reset again
        boolean reuse = resetPassword(token, "AnotherStr0ng!");
        System.out.println("Test5 reuse token: " + reuse);
        String token2 = requestReset("alice@example.com");
        boolean ok2 = resetPassword(token2, "UltraStr0ng#2025");
        System.out.println("Test5 new token reset: " + ok2);
    }
}