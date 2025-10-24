import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.time.Instant;
import java.util.Arrays;
import java.util.Base64;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task133 {
    private static final ConcurrentHashMap<String, UserRecord> USERS = new ConcurrentHashMap<>();
    private static final SecureRandom SECURE_RANDOM = new SecureRandom();
    private static final long TOKEN_TTL_SECONDS = 15 * 60; // 15 minutes
    private static final int PBKDF2_ITERATIONS = 150_000;
    private static final int SALT_LEN = 16;
    private static final int DERIVED_KEY_LEN = 32; // 256-bit

    private static final Set<String> COMMON_PASSWORDS;
    static {
        Set<String> s = new HashSet<>(Arrays.asList(
                "password", "123456", "123456789", "qwerty", "111111",
                "12345678", "abc123", "password1", "1234567", "12345",
                "letmein", "admin", "welcome", "monkey", "login"
        ));
        COMMON_PASSWORDS = Collections.unmodifiableSet(s);
    }

    private static class UserRecord {
        final String email;
        String passwordHash; // format: pbkdf2$sha256$iter$saltB64$hashB64
        String resetTokenHashHex; // SHA-256 hex of token
        long resetTokenExpiryEpochSec;

        UserRecord(String email, String passwordHash) {
            this.email = email;
            this.passwordHash = passwordHash;
            this.resetTokenHashHex = null;
            this.resetTokenExpiryEpochSec = 0L;
        }
    }

    // Public API

    // Registers a user with a strong password; returns true on success.
    public static boolean registerUser(String email, String password) {
        if (email == null || email.isEmpty() || password == null) return false;
        if (!isStrongPassword(password, email)) return false;
        String hash = hashPassword(password);
        UserRecord rec = new UserRecord(email.toLowerCase(), hash);
        return USERS.putIfAbsent(rec.email, rec) == null;
    }

    // Requests a password reset; returns a token (would be emailed). For unknown emails, returns a fake token to avoid enumeration.
    public static String requestPasswordReset(String email) {
        if (email == null) return "";
        String key = email.toLowerCase();
        String token = generateToken();
        String tokenHashHex = sha256Hex(token);
        long expiry = Instant.now().getEpochSecond() + TOKEN_TTL_SECONDS;

        UserRecord rec = USERS.get(key);
        if (rec != null) {
            rec.resetTokenHashHex = tokenHashHex;
            rec.resetTokenExpiryEpochSec = expiry;
        }
        // Return token regardless to avoid leaking account existence in real systems.
        return token;
    }

    // Resets the password if token is valid and password strong; returns true on success.
    public static boolean resetPassword(String email, String token, String newPassword) {
        if (email == null || token == null || newPassword == null) return false;
        String key = email.toLowerCase();
        UserRecord rec = USERS.get(key);
        if (rec == null) return false;

        long now = Instant.now().getEpochSecond();
        if (rec.resetTokenHashHex == null || now > rec.resetTokenExpiryEpochSec) {
            return false;
        }
        String providedHashHex = sha256Hex(token);
        if (!constantTimeEqualsHex(rec.resetTokenHashHex, providedHashHex)) {
            return false;
        }
        if (!isStrongPassword(newPassword, email)) {
            return false;
        }
        // Prevent reusing the same password
        if (verifyPassword(newPassword, rec.passwordHash)) {
            return false;
        }
        rec.passwordHash = hashPassword(newPassword);
        // Invalidate token after use
        rec.resetTokenHashHex = null;
        rec.resetTokenExpiryEpochSec = 0L;
        return true;
    }

    // Verifies login credentials
    public static boolean verifyLogin(String email, String password) {
        if (email == null || password == null) return false;
        UserRecord rec = USERS.get(email.toLowerCase());
        if (rec == null) return false;
        return verifyPassword(password, rec.passwordHash);
    }

    // Internal helpers

    private static boolean isStrongPassword(String password, String email) {
        if (password == null) return false;
        if (password.length() < 12) return false;
        boolean hasUpper = false, hasLower = false, hasDigit = false, hasSymbol = false;
        for (char c : password.toCharArray()) {
            if (Character.isUpperCase(c)) hasUpper = true;
            else if (Character.isLowerCase(c)) hasLower = true;
            else if (Character.isDigit(c)) hasDigit = true;
            else hasSymbol = true;
        }
        if (!(hasUpper && hasLower && hasDigit && hasSymbol)) return false;
        String lower = password.toLowerCase();
        if (COMMON_PASSWORDS.contains(lower)) return false;
        if (email != null) {
            String userPart = email.toLowerCase().split("@")[0];
            if (!userPart.isEmpty() && lower.contains(userPart)) return false;
        }
        return true;
    }

    private static String hashPassword(String password) {
        byte[] salt = new byte[SALT_LEN];
        SECURE_RANDOM.nextBytes(salt);
        byte[] dk = pbkdf2(password.toCharArray(), salt, PBKDF2_ITERATIONS, DERIVED_KEY_LEN);
        String saltB64 = Base64.getEncoder().withoutPadding().encodeToString(salt);
        String hashB64 = Base64.getEncoder().withoutPadding().encodeToString(dk);
        return "pbkdf2$sha256$" + PBKDF2_ITERATIONS + "$" + saltB64 + "$" + hashB64;
    }

    private static boolean verifyPassword(String password, String stored) {
        try {
            String[] parts = stored.split("\\$");
            if (parts.length != 5) return false;
            if (!"pbkdf2".equals(parts[0]) || !"sha256".equals(parts[1])) return false;
            int iter = Integer.parseInt(parts[2]);
            byte[] salt = Base64.getDecoder().decode(parts[3]);
            byte[] expected = Base64.getDecoder().decode(parts[4]);
            byte[] got = pbkdf2(password.toCharArray(), salt, iter, expected.length);
            return MessageDigest.isEqual(expected, got);
        } catch (Exception e) {
            return false;
        }
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int dkLen) {
        try {
            KeySpec spec = new PBEKeySpec(password, salt, iterations, dkLen * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            throw new RuntimeException("PBKDF2 error", e);
        }
    }

    private static String generateToken() {
        byte[] buf = new byte[32];
        SECURE_RANDOM.nextBytes(buf);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(buf);
    }

    private static String sha256Hex(String data) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] dig = md.digest(data.getBytes(java.nio.charset.StandardCharsets.UTF_8));
            return toHex(dig);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException(e);
        }
    }

    private static boolean constantTimeEqualsHex(String hexA, String hexB) {
        if (hexA == null || hexB == null) return false;
        byte[] a = fromHex(hexA);
        byte[] b = fromHex(hexB);
        return MessageDigest.isEqual(a, b);
    }

    private static String toHex(byte[] data) {
        StringBuilder sb = new StringBuilder(data.length * 2);
        for (byte b : data) sb.append(String.format("%02x", b));
        return sb.toString();
    }

    private static byte[] fromHex(String hex) {
        int len = hex.length();
        if ((len & 1) != 0) return new byte[0];
        byte[] out = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            out[i / 2] = (byte) Integer.parseInt(hex.substring(i, i + 2), 16);
        }
        return out;
    }

    public static void main(String[] args) {
        // Test setup users
        System.out.println("Register Alice: " + registerUser("alice@example.com", "InitialPassw0rd!"));
        System.out.println("Register Bob:   " + registerUser("bob@example.com", "Secur3P@ssw0rd!"));
        System.out.println("Register Charlie: " + registerUser("charlie@example.com", "Another$tr0ng1"));
        System.out.println("Register Dave: " + registerUser("dave@example.com", "Y3t@Str0ngPass!"));
        System.out.println("Register Eve:  " + registerUser("eve@example.com", "InitialPassw0rd!"));

        // 1) Successful reset
        String t1 = requestPasswordReset("alice@example.com");
        boolean r1 = resetPassword("alice@example.com", t1, "NewStrongP@ssw0rd!");
        System.out.println("Test1 success reset: " + r1 + " | login new: " + verifyLogin("alice@example.com", "NewStrongP@ssw0rd!"));

        // 2) Expired token
        String t2 = requestPasswordReset("bob@example.com");
        // Simulate expiry
        UserRecord bob = USERS.get("bob@example.com");
        if (bob != null) bob.resetTokenExpiryEpochSec = Instant.now().getEpochSecond() - 1;
        boolean r2 = resetPassword("bob@example.com", t2, "AnotherN3wP@ss!");
        System.out.println("Test2 expired token result: " + r2);

        // 3) Weak password attempt
        String t3 = requestPasswordReset("charlie@example.com");
        boolean r3 = resetPassword("charlie@example.com", t3, "password");
        System.out.println("Test3 weak password result: " + r3);

        // 4) Invalid token
        requestPasswordReset("dave@example.com");
        boolean r4 = resetPassword("dave@example.com", "INVALID_TOKEN", "Th!sIsStr0ngP@ss");
        System.out.println("Test4 invalid token result: " + r4);

        // 5) Reuse old password not allowed
        String t5 = requestPasswordReset("eve@example.com");
        boolean r5 = resetPassword("eve@example.com", t5, "InitialPassw0rd!");
        System.out.println("Test5 reuse old password result: " + r5);
    }
}