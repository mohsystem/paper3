import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.GeneralSecurityException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Task54 {
    private static final int SALT_LEN = 16;
    private static final int HASH_LEN = 32;
    private static final int PBKDF2_ITER = 120_000;
    private static final long OTP_TTL_MILLIS = 2 * 60 * 1000L; // 2 minutes
    private static final SecureRandom SECURE_RANDOM = secureRandom();

    private static class User {
        final byte[] salt;
        final byte[] hash;
        final int iterations;
        User(byte[] salt, byte[] hash, int iterations) {
            this.salt = salt;
            this.hash = hash;
            this.iterations = iterations;
        }
    }

    private static class PendingOtp {
        final String otp;
        final long expiresAtMs;
        boolean used;
        PendingOtp(String otp, long expiresAtMs) {
            this.otp = otp;
            this.expiresAtMs = expiresAtMs;
            this.used = false;
        }
    }

    private static final Map<String, User> users = new HashMap<>();
    private static final Map<String, PendingOtp> pendingOtps = new HashMap<>();

    private static SecureRandom secureRandom() {
        try {
            return SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            return new SecureRandom();
        }
    }

    private static byte[] pbkdf2(char[] password, byte[] salt, int iterations, int length) throws GeneralSecurityException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, iterations, length * 8);
        SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
        byte[] out = skf.generateSecret(spec).getEncoded();
        spec.clearPassword();
        return out;
    }

    private static boolean timingSafeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        int result = 0;
        for (int i = 0; i < a.length; i++) result |= (a[i] ^ b[i]);
        return result == 0;
    }

    private static boolean timingSafeEqualsString(String a, String b) {
        if (a == null || b == null) return false;
        byte[] ab = a.getBytes(StandardCharsets.UTF_8);
        byte[] bb = b.getBytes(StandardCharsets.UTF_8);
        return timingSafeEquals(ab, bb);
    }

    public static boolean registerUser(String username, char[] password) {
        if (username == null || username.isEmpty() || username.length() > 64) return false;
        if (password == null || password.length < 8) return false;
        if (users.containsKey(username)) return false;
        byte[] salt = new byte[SALT_LEN];
        SECURE_RANDOM.nextBytes(salt);
        try {
            byte[] hash = pbkdf2(password, salt, PBKDF2_ITER, HASH_LEN);
            users.put(username, new User(salt, hash, PBKDF2_ITER));
            Arrays.fill(password, '\0');
            return true;
        } catch (GeneralSecurityException e) {
            Arrays.fill(password, '\0');
            return false;
        }
    }

    public static String loginRequest(String username, char[] password) {
        if (username == null || password == null) return null;
        User user = users.get(username);
        if (user == null) {
            Arrays.fill(password, '\0');
            return null;
        }
        try {
            byte[] candidate = pbkdf2(password, user.salt, user.iterations, HASH_LEN);
            Arrays.fill(password, '\0');
            if (!timingSafeEquals(candidate, user.hash)) {
                Arrays.fill(candidate, (byte)0);
                return null;
            }
            Arrays.fill(candidate, (byte)0);
        } catch (GeneralSecurityException e) {
            Arrays.fill(password, '\0');
            return null;
        }
        int code = SECURE_RANDOM.nextInt(1_000_000);
        String otp = String.format("%06d", code);
        long expires = System.currentTimeMillis() + OTP_TTL_MILLIS;
        pendingOtps.put(username, new PendingOtp(otp, expires));
        return otp;
    }

    public static boolean verifyOtp(String username, String otp) {
        if (username == null || otp == null) return false;
        PendingOtp po = pendingOtps.get(username);
        if (po == null) return false;
        long now = System.currentTimeMillis();
        if (po.used || now > po.expiresAtMs) {
            pendingOtps.remove(username);
            return false;
        }
        boolean ok = timingSafeEqualsString(po.otp, otp);
        if (ok) {
            po.used = true;
            pendingOtps.remove(username);
            return true;
        } else {
            return false;
        }
    }

    public static void main(String[] args) {
        // Test 1: Successful login with correct password and OTP
        System.out.println("Register alice: " + registerUser("alice", "password123".toCharArray()));
        String otp1 = loginRequest("alice", "password123".toCharArray());
        System.out.println("OTP issued (alice): " + (otp1 != null ? otp1 : "null"));
        System.out.println("Verify correct OTP (alice): " + verifyOtp("alice", otp1));

        // Test 2: Wrong password
        String otpWrongPwd = loginRequest("alice", "wrongpass!".toCharArray());
        System.out.println("Login with wrong password returns OTP? " + (otpWrongPwd != null));

        // Test 3: Wrong OTP
        String otp2 = loginRequest("alice", "password123".toCharArray());
        System.out.println("OTP issued (alice) second time: " + (otp2 != null ? otp2 : "null"));
        System.out.println("Verify wrong OTP (alice): " + verifyOtp("alice", "000000"));

        // Test 4: Nonexistent user
        String otpNoUser = loginRequest("bob", "anything123".toCharArray());
        System.out.println("OTP issued (nonexistent user bob): " + (otpNoUser != null ? otpNoUser : "null"));

        // Test 5: OTP reuse should fail
        String otp3 = loginRequest("alice", "password123".toCharArray());
        System.out.println("OTP issued (alice third time): " + (otp3 != null ? otp3 : "null"));
        boolean firstUse = verifyOtp("alice", otp3);
        boolean secondUse = verifyOtp("alice", otp3);
        System.out.println("First use success? " + firstUse + ", second reuse should fail: " + secondUse);
    }
}