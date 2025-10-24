// Chain-of-Through Step 1: Problem understanding
// Implement a 2FA flow using securely generated random one-time passwords (OTPs).
// Expose functions that accept parameters and return outputs. Include expiry, attempt limits, and constant-time comparisons.

// Chain-of-Through Step 2: Security requirements
// - Use SecureRandom for OTP generation
// - Avoid modulo bias in digit generation by using per-digit nextInt(10)
// - Use constant-time comparison for OTP checks
// - Implement OTP expiration and attempt limits
// - Do not persist secrets longer than needed; invalidate on success or after attempts
// - Avoid printing OTPs in production (only for demo testing)

// Chain-of-Through Step 3: Secure coding generation
import java.util.*;
import java.security.SecureRandom;
import java.security.MessageDigest;

public class Task54 {

    // Constant-time string comparison to avoid timing attacks
    public static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        byte[] x = a.getBytes(java.nio.charset.StandardCharsets.UTF_8);
        byte[] y = b.getBytes(java.nio.charset.StandardCharsets.UTF_8);
        return MessageDigest.isEqual(x, y);
    }

    // Simple in-memory user store (demo only; in production store salted, hashed passwords)
    public static class UserStore {
        private final Map<String, String> userToPassword = new HashMap<>();
        public void addUser(String username, String password) {
            userToPassword.put(username, password);
        }
        public boolean verifyPassword(String username, String password) {
            String stored = userToPassword.get(username);
            if (stored == null) return false;
            return constantTimeEquals(stored, password);
        }
    }

    public static class TwoFAService {
        private static final SecureRandom RNG = new SecureRandom();

        private static class OtpRecord {
            String otp;
            long expiresAtMs;
            int attemptsLeft;
            boolean valid;
        }

        private final Map<String, OtpRecord> records = new HashMap<>();
        private final int otpLength;
        private final long ttlMillis;
        private final int maxAttempts;

        public TwoFAService(int otpLength, long ttlMillis, int maxAttempts) {
            if (otpLength < 4 || otpLength > 12) throw new IllegalArgumentException("OTP length out of bounds");
            if (ttlMillis < 1000) throw new IllegalArgumentException("TTL too short");
            if (maxAttempts < 1) throw new IllegalArgumentException("Attempts must be >=1");
            this.otpLength = otpLength;
            this.ttlMillis = ttlMillis;
            this.maxAttempts = maxAttempts;
        }

        // Secure OTP generation using per-digit sampling from SecureRandom
        public String generateOtp(String userId) {
            if (userId == null || userId.isEmpty()) throw new IllegalArgumentException("userId required");
            StringBuilder sb = new StringBuilder(otpLength);
            for (int i = 0; i < otpLength; i++) {
                int d = RNG.nextInt(10);
                sb.append((char) ('0' + d));
            }
            String otp = sb.toString();
            OtpRecord rec = new OtpRecord();
            rec.otp = otp;
            rec.expiresAtMs = System.currentTimeMillis() + ttlMillis;
            rec.attemptsLeft = maxAttempts;
            rec.valid = true;
            records.put(userId, rec);
            return otp; // In production, do not return; send via secure channel
        }

        public boolean verifyOtp(String userId, String otpInput) {
            OtpRecord rec = records.get(userId);
            long now = System.currentTimeMillis();
            if (rec == null || !rec.valid) return false;
            if (now > rec.expiresAtMs) {
                rec.valid = false;
                return false;
            }
            if (rec.attemptsLeft <= 0) {
                rec.valid = false;
                return false;
            }
            boolean ok = constantTimeEquals(rec.otp, otpInput);
            if (ok) {
                rec.valid = false; // invalidate on use
                return true;
            } else {
                rec.attemptsLeft--;
                if (rec.attemptsLeft <= 0) rec.valid = false;
                return false;
            }
        }

        // Test helper: force expiration
        public void expireNowForTest(String userId) {
            OtpRecord rec = records.get(userId);
            if (rec != null) rec.expiresAtMs = System.currentTimeMillis() - 1;
        }
    }

    // Step-wise login: first password, then OTP
    public static boolean loginStep1(UserStore store, String username, String password) {
        return store.verifyPassword(username, password);
    }

    public static void main(String[] args) {
        // Chain-of-Through Step 4: Code review (embedded via comments)
        // Verified: SecureRandom, constant-time compare, expiry, attempt limits, no OTP logging by default.
        // Demo prints OTPs to simulate delivery for tests.

        UserStore store = new UserStore();
        store.addUser("alice", "pass123");
        store.addUser("bob", "secret!");
        store.addUser("carol", "p@ssw0rd");
        store.addUser("dave", "letmein");
        store.addUser("erin", "hunter2");

        TwoFAService twoFA = new TwoFAService(6, 2000L, 3); // 2 second TTL for demo

        // Test 1: Successful login (correct password and OTP)
        {
            boolean pw = loginStep1(store, "alice", "pass123");
            String otp = pw ? twoFA.generateOtp("alice") : null;
            // Simulate sending OTP: in tests we print
            System.out.println("Test1 OTP (simulated send): " + otp);
            boolean ok = pw && twoFA.verifyOtp("alice", otp);
            System.out.println("Test1 success: " + ok);
        }

        // Test 2: Wrong password
        {
            boolean pw = loginStep1(store, "bob", "wrong");
            boolean ok = pw; // no OTP step since pw failed
            System.out.println("Test2 success (should be false): " + ok);
        }

        // Test 3: Wrong OTP
        {
            boolean pw = loginStep1(store, "carol", "p@ssw0rd");
            String otp = pw ? twoFA.generateOtp("carol") : null;
            System.out.println("Test3 OTP (simulated send): " + otp);
            boolean ok = pw && twoFA.verifyOtp("carol", "000000");
            System.out.println("Test3 success (should be false): " + ok);
        }

        // Test 4: Expired OTP
        {
            boolean pw = loginStep1(store, "dave", "letmein");
            String otp = pw ? twoFA.generateOtp("dave") : null;
            System.out.println("Test4 OTP (simulated send): " + otp);
            twoFA.expireNowForTest("dave");
            boolean ok = pw && twoFA.verifyOtp("dave", otp);
            System.out.println("Test4 success (should be false): " + ok);
        }

        // Test 5: Attempt limit exceeded
        {
            boolean pw = loginStep1(store, "erin", "hunter2");
            String otp = pw ? twoFA.generateOtp("erin") : null;
            System.out.println("Test5 OTP (simulated send): " + otp);
            boolean a = twoFA.verifyOtp("erin", "111111"); // wrong
            boolean b = twoFA.verifyOtp("erin", "222222"); // wrong
            boolean c = twoFA.verifyOtp("erin", "333333"); // wrong -> should invalidate
            boolean d = twoFA.verifyOtp("erin", otp); // even correct should fail now
            System.out.println("Test5 sequence (should be false,false,false,false): " + a + "," + b + "," + c + "," + d);
        }

        // Chain-of-Through Step 5: Secure code output finalized
    }
}