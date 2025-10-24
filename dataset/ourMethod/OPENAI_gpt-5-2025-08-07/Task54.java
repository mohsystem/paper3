import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.security.spec.KeySpec;
import java.time.Instant;
import java.util.Arrays;
import java.util.Base64;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Task54 {

    // ===================== Core 2FA Implementation =====================

    private static final class User {
        final String username;
        final byte[] passwordSalt;
        final byte[] passwordHash;
        final int iterations;
        volatile byte[] otpSalt;
        volatile byte[] otpHash;
        volatile long otpExpiryEpochSeconds;

        User(String username, byte[] passwordSalt, byte[] passwordHash, int iterations) {
            this.username = username;
            this.passwordSalt = passwordSalt;
            this.passwordHash = passwordHash;
            this.iterations = iterations;
            this.otpSalt = null;
            this.otpHash = null;
            this.otpExpiryEpochSeconds = 0L;
        }
    }

    public static final class LoginResult {
        public final boolean success;
        // For testing only; do not print/log in real systems.
        public final String otp;

        public LoginResult(boolean success, String otp) {
            this.success = success;
            this.otp = otp;
        }
    }

    private static final ConcurrentHashMap<String, User> USERS = new ConcurrentHashMap<>();
    private static final SecureRandom RNG = new SecureRandom();
    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32;
    private static final int PBKDF2_ITER = 210_000;
    private static final long OTP_TTL_SECONDS = 300; // 5 minutes

    // ------------------ Public API ------------------

    public static boolean registerUser(String username, char[] password) {
        Objects.requireNonNull(username, "username");
        Objects.requireNonNull(password, "password");
        try {
            if (!isValidUsername(username)) return false;
            if (!isStrongPassword(password)) return false;
            if (USERS.containsKey(username)) return false;

            byte[] salt = randomBytes(SALT_LEN);
            byte[] hash = pbkdf2(password, salt, PBKDF2_ITER, HASH_LEN);
            User u = new User(username, salt, hash, PBKDF2_ITER);
            return USERS.putIfAbsent(username, u) == null;
        } finally {
            zeroCharArray(password);
        }
    }

    public static LoginResult startLogin(String username, char[] password) {
        Objects.requireNonNull(username, "username");
        Objects.requireNonNull(password, "password");
        try {
            User u = USERS.get(username);
            if (u == null) return new LoginResult(false, null);
            byte[] cand = pbkdf2(password, u.passwordSalt, u.iterations, HASH_LEN);
            boolean ok = constantTimeEq(cand, u.passwordHash);
            zeroByteArray(cand);
            if (!ok) return new LoginResult(false, null);

            String otp = generateNumericOtp(6);
            byte[] otpSalt = randomBytes(SALT_LEN);
            byte[] otpHash = pbkdf2(otp.toCharArray(), otpSalt, PBKDF2_ITER, HASH_LEN);
            long expiry = Instant.now().getEpochSecond() + OTP_TTL_SECONDS;

            // Set OTP atomically
            u.otpSalt = otpSalt;
            u.otpHash = otpHash;
            u.otpExpiryEpochSeconds = expiry;

            return new LoginResult(true, otp);
        } finally {
            zeroCharArray(password);
        }
    }

    public static boolean verifyOtp(String username, String otp) {
        Objects.requireNonNull(username, "username");
        Objects.requireNonNull(otp, "otp");
        User u = USERS.get(username);
        if (u == null) return false;
        long now = Instant.now().getEpochSecond();
        if (u.otpHash == null || u.otpSalt == null || now > u.otpExpiryEpochSeconds) {
            return false;
        }
        byte[] cand = pbkdf2(otp.toCharArray(), u.otpSalt, PBKDF2_ITER, HASH_LEN);
        boolean ok = constantTimeEq(cand, u.otpHash);
        zeroByteArray(cand);
        if (ok) {
            // Invalidate OTP after successful verification
            u.otpHash = null;
            u.otpSalt = null;
            u.otpExpiryEpochSeconds = 0L;
        }
        return ok;
    }

    // For testing only: force expire OTP
    public static void expireOtpForTest(String username) {
        User u = USERS.get(username);
        if (u != null) {
            u.otpExpiryEpochSeconds = Instant.now().getEpochSecond() - 1;
        }
    }

    // ------------------ Helpers ------------------

    private static boolean isValidUsername(String username) {
        if (username.length() < 3 || username.length() > 64) return false;
        for (int i = 0; i < username.length(); i++) {
            char c = username.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '.' || c == '_' || c == '-';
            if (!ok) return false;
        }
        return true;
    }

    private static boolean isStrongPassword(char[] pwd) {
        if (pwd.length < 12 || pwd.length > 1024) return false;
        boolean hasU = false, hasL = false, hasD = false, hasS = false;
        for (char c : pwd) {
            if (c >= 'A' && c <= 'Z') hasU = true;
            else if (c >= 'a' && c <= 'z') hasL = true;
            else if (c >= '0' && c <= '9') hasD = true;
            else hasS = true;
        }
        return hasU && hasL && hasD && hasS;
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iter, int len) {
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, iter, len * 8);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return skf.generateSecret(spec).getEncoded();
        } catch (Exception e) {
            return new byte[0];
        }
    }

    private static boolean constantTimeEq(byte[] a, byte[] b) {
        return MessageDigest.isEqual(a, b);
    }

    private static String generateNumericOtp(int digits) {
        if (digits <= 0 || digits > 9) digits = 6;
        long bound = 1;
        for (int i = 0; i < digits; i++) bound *= 10L;
        long limit = (0x1_0000_0000L / bound) * bound; // floor(2^32/bound)*bound
        long val;
        do {
            int x = RNG.nextInt();
            val = (x & 0xFFFF_FFFFL);
        } while (val >= limit);
        long num = val % bound;
        String s = Long.toString(num);
        if (s.length() < digits) {
            StringBuilder sb = new StringBuilder(digits);
            for (int i = s.length(); i < digits; i++) sb.append('0');
            sb.append(s);
            return sb.toString();
        }
        return s;
    }

    private static byte[] randomBytes(int n) {
        byte[] b = new byte[n];
        RNG.nextBytes(b);
        return b;
    }

    private static void zeroCharArray(char[] a) {
        Arrays.fill(a, '\0');
    }

    private static void zeroByteArray(byte[] a) {
        if (a != null) Arrays.fill(a, (byte) 0);
    }

    // ===================== Demo Tests =====================

    public static void main(String[] args) {
        // Test 1: Successful registration and 2FA login
        boolean reg1 = registerUser("alice", "Str0ng!Passw0rd".toCharArray());
        LoginResult lr1 = startLogin("alice", "Str0ng!Passw0rd".toCharArray());
        boolean v1 = lr1.success && verifyOtp("alice", lr1.otp);
        System.out.println("Test1 (success 2FA): " + (reg1 && v1));

        // Test 2: Wrong password
        LoginResult lr2 = startLogin("alice", "wrongPassword1!".toCharArray());
        System.out.println("Test2 (wrong password): " + (!lr2.success));

        // Test 3: Wrong OTP
        LoginResult lr3 = startLogin("alice", "Str0ng!Passw0rd".toCharArray());
        boolean v3 = lr3.success && verifyOtp("alice", "000000");
        System.out.println("Test3 (wrong OTP): " + (!v3));

        // Test 4: Expired OTP
        LoginResult lr4 = startLogin("alice", "Str0ng!Passw0rd".toCharArray());
        expireOtpForTest("alice");
        boolean v4 = lr4.success && verifyOtp("alice", lr4.otp);
        System.out.println("Test4 (expired OTP): " + (!v4));

        // Test 5: Another user end-to-end
        boolean reg2 = registerUser("bob_user", "An0ther$trongPass".toCharArray());
        LoginResult lr5 = startLogin("bob_user", "An0ther$trongPass".toCharArray());
        boolean v5 = lr5.success && verifyOtp("bob_user", lr5.otp);
        System.out.println("Test5 (second user success): " + (reg2 && v5));
    }
}