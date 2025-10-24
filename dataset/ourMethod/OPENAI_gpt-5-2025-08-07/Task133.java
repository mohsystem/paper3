import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.security.GeneralSecurityException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task133 {
    private static final int ITERATIONS = 210_000;
    private static final int SALT_LEN = 16;
    private static final int KEY_LEN = 32;
    private static final int TOKEN_LEN = 32;
    private static final long TOKEN_TTL_MILLIS = 15L * 60L * 1000L; // 15 minutes
    private static final SecureRandom RNG = new SecureRandom();
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[A-Za-z0-9_]{3,32}$");

    private static final class UserRecord {
        final String username;
        final byte[] salt;
        byte[] pwdHash;
        long pwdChangedAt;

        byte[] tokenSalt;
        byte[] tokenHash;
        long tokenExpiry;

        UserRecord(String username, byte[] salt, byte[] pwdHash, long pwdChangedAt) {
            this.username = username;
            this.salt = salt;
            this.pwdHash = pwdHash;
            this.pwdChangedAt = pwdChangedAt;
            this.tokenSalt = null;
            this.tokenHash = null;
            this.tokenExpiry = 0L;
        }

        void clearToken() {
            this.tokenSalt = null;
            this.tokenHash = null;
            this.tokenExpiry = 0L;
        }
    }

    // Public API
    public static boolean registerUser(Map<String, UserRecord> db, String username, String password) {
        if (!validateUsername(username) || !isStrongPassword(password, username)) return false;
        if (db.containsKey(username)) return false;

        byte[] salt = randomBytes(SALT_LEN);
        byte[] hash = hashPBKDF2(password, salt, ITERATIONS, KEY_LEN);
        if (hash == null) return false;
        db.put(username, new UserRecord(username, salt, hash, System.currentTimeMillis()));
        zeroize(hash);
        return true;
    }

    public static boolean authenticate(Map<String, UserRecord> db, String username, String password) {
        if (!validateUsername(username) || password == null) return false;
        UserRecord u = db.get(username);
        if (u == null) return false;
        byte[] derived = hashPBKDF2(password, u.salt, ITERATIONS, KEY_LEN);
        if (derived == null) return false;
        boolean ok = constantTimeEquals(derived, u.pwdHash);
        zeroize(derived);
        return ok;
    }

    public static String requestPasswordReset(Map<String, UserRecord> db, String username) {
        if (!validateUsername(username)) return "";
        UserRecord u = db.get(username);
        if (u == null) {
            return "";
        }
        byte[] token = randomBytes(TOKEN_LEN);
        byte[] tokenSalt = randomBytes(SALT_LEN);
        byte[] tokenHash = hashPBKDF2(token, tokenSalt, ITERATIONS, KEY_LEN);
        if (tokenHash == null) return "";
        u.tokenSalt = tokenSalt;
        u.tokenHash = tokenHash;
        u.tokenExpiry = System.currentTimeMillis() + TOKEN_TTL_MILLIS;
        String tokenHex = toHex(token);
        zeroize(token);
        return tokenHex;
    }

    public static boolean resetPassword(Map<String, UserRecord> db, String username, String tokenHex, String newPassword) {
        if (!validateUsername(username) || tokenHex == null || newPassword == null) return false;
        UserRecord u = db.get(username);
        if (u == null) return false;
        if (u.tokenHash == null || u.tokenSalt == null) return false;
        if (System.currentTimeMillis() > u.tokenExpiry) return false;

        byte[] token = fromHex(tokenHex);
        if (token == null || token.length != TOKEN_LEN) return false;
        byte[] providedHash = hashPBKDF2(token, u.tokenSalt, ITERATIONS, KEY_LEN);
        zeroize(token);
        if (providedHash == null) return false;
        boolean tokenMatch = constantTimeEquals(providedHash, u.tokenHash);
        zeroize(providedHash);
        if (!tokenMatch) return false;

        if (!isStrongPassword(newPassword, username)) return false;
        // Prevent reusing the same password
        byte[] newDerivedWithOldSalt = hashPBKDF2(newPassword, u.salt, ITERATIONS, KEY_LEN);
        if (newDerivedWithOldSalt == null) return false;
        boolean sameAsOld = constantTimeEquals(newDerivedWithOldSalt, u.pwdHash);
        zeroize(newDerivedWithOldSalt);
        if (sameAsOld) return false;

        // Update password with fresh salt
        byte[] newSalt = randomBytes(SALT_LEN);
        byte[] newHash = hashPBKDF2(newPassword, newSalt, ITERATIONS, KEY_LEN);
        if (newHash == null) return false;
        System.arraycopy(newSalt, 0, u.salt, 0, SALT_LEN); // reuse salt array for immutability of field reference
        u.pwdHash = newHash;
        u.pwdChangedAt = System.currentTimeMillis();
        u.clearToken();
        return true;
    }

    // Helpers
    private static boolean validateUsername(String username) {
        if (username == null) return false;
        if (!USERNAME_PATTERN.matcher(username).matches()) return false;
        return true;
    }

    private static boolean isStrongPassword(String password, String username) {
        if (password == null) return false;
        if (password.length() < 12 || password.length() > 128) return false;
        boolean up=false, lo=false, di=false, sp=false;
        for (int i=0;i<password.length();i++){
            char c = password.charAt(i);
            if (Character.isUpperCase(c)) up = true;
            else if (Character.isLowerCase(c)) lo = true;
            else if (Character.isDigit(c)) di = true;
            else if (!Character.isWhitespace(c)) sp = true;
            if (Character.isWhitespace(c)) return false;
        }
        if (!(up && lo && di && sp)) return false;
        String lc = password.toLowerCase();
        if (username != null && !username.isEmpty()) {
            if (lc.contains(username.toLowerCase())) return false;
        }
        String[] common = {"password","123456","qwerty","letmein","admin","welcome","iloveyou","monkey","abc123"};
        for (String w : common) {
            if (lc.contains(w)) return false;
        }
        return true;
    }

    private static byte[] hashPBKDF2(String input, byte[] salt, int iterations, int keyLen) {
        return hashPBKDF2(input.getBytes(StandardCharsets.UTF_8), salt, iterations, keyLen);
    }

    private static byte[] hashPBKDF2(byte[] input, byte[] salt, int iterations, int keyLen) {
        try {
            PBEKeySpec spec = new PBEKeySpec(bytesToChars(input), salt, iterations, keyLen * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            byte[] out = skf.generateSecret(spec).getEncoded();
            spec.clearPassword();
            return out;
        } catch (GeneralSecurityException e) {
            return null;
        }
    }

    private static char[] bytesToChars(byte[] b) {
        // Treat as UTF-8 bytes here as we use this for tokens also; safe mapping
        char[] out = new char[b.length];
        for (int i = 0; i < b.length; i++) out[i] = (char)(b[i] & 0xFF);
        return out;
    }

    private static byte[] randomBytes(int len) {
        byte[] b = new byte[len];
        RNG.nextBytes(b);
        return b;
    }

    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        return MessageDigest.isEqual(a, b);
    }

    private static String toHex(byte[] data) {
        char[] hex = new char[data.length * 2];
        final char[] DIGITS = "0123456789abcdef".toCharArray();
        for (int i = 0, j = 0; i < data.length; i++) {
            int v = data[i] & 0xFF;
            hex[j++] = DIGITS[v >>> 4];
            hex[j++] = DIGITS[v & 0x0F];
        }
        return new String(hex);
    }

    private static byte[] fromHex(String hex) {
        if (hex == null || (hex.length() & 1) != 0) return null;
        int len = hex.length() / 2;
        byte[] out = new byte[len];
        for (int i = 0; i < len; i++) {
            int hi = Character.digit(hex.charAt(2*i), 16);
            int lo = Character.digit(hex.charAt(2*i+1), 16);
            if (hi < 0 || lo < 0) return null;
            out[i] = (byte)((hi << 4) | lo);
        }
        return out;
    }

    private static void zeroize(byte[] b) {
        if (b != null) Arrays.fill(b, (byte)0);
    }

    // Demo tests - 5 test cases
    public static void main(String[] args) {
        Map<String, UserRecord> db = new HashMap<>();
        boolean t1 = registerUser(db, "alice", "Str0ng!Passw0rd");
        boolean t2 = authenticate(db, "alice", "Str0ng!Passw0rd");

        String token = requestPasswordReset(db, "alice");
        boolean t3 = resetPassword(db, "alice", token, "An0ther!StrongPwd") && authenticate(db, "alice", "An0ther!StrongPwd");

        boolean t4 = resetPassword(db, "alice", "deadbeef", "Yet@N0therStrong1"); // invalid token

        String token2 = requestPasswordReset(db, "alice");
        boolean t5 = resetPassword(db, "alice", token2, "weakpass"); // weak password

        System.out.println("Test1_RegisterStrongPassword: " + t1);
        System.out.println("Test2_AuthenticateCorrect: " + t2);
        System.out.println("Test3_ResetWithValidTokenAndAuth: " + t3);
        System.out.println("Test4_ResetWithInvalidToken: " + t4);
        System.out.println("Test5_AttemptWeakPasswordReset: " + t5);
    }
}