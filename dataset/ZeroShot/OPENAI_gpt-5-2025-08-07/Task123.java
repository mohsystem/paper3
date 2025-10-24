import java.security.MessageDigest;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Base64;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;

public class Task123 {

    private static final long MAX_TTL_SECONDS = 60L * 60L * 24L * 30L; // 30 days
    private static final int TOKEN_BYTES = 32;
    private final SecureRandom rng = new SecureRandom();

    private static final class Session {
        final String userId;
        volatile long expiryEpochSeconds;
        Session(String userId, long expiryEpochSeconds) {
            this.userId = userId;
            this.expiryEpochSeconds = expiryEpochSeconds;
        }
    }

    private final ConcurrentHashMap<String, Session> sessions = new ConcurrentHashMap<>();

    private static boolean isNullOrBlank(String s) {
        return s == null || s.trim().isEmpty();
    }

    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        byte[] ab = a.getBytes(java.nio.charset.StandardCharsets.UTF_8);
        byte[] bb = b.getBytes(java.nio.charset.StandardCharsets.UTF_8);
        return MessageDigest.isEqual(ab, bb);
    }

    private static long nowSeconds() {
        return Instant.now().getEpochSecond();
    }

    private String generateToken() {
        byte[] bytes = new byte[TOKEN_BYTES];
        rng.nextBytes(bytes);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
    }

    public String createSession(String userId, long ttlSeconds) {
        if (isNullOrBlank(userId)) return null;
        if (ttlSeconds <= 0) return null;
        if (ttlSeconds > MAX_TTL_SECONDS) ttlSeconds = MAX_TTL_SECONDS;

        String token;
        do {
            token = generateToken();
        } while (sessions.containsKey(token)); // extremely unlikely, but safe

        long expiry = nowSeconds() + ttlSeconds;
        sessions.put(token, new Session(userId, expiry));
        return token;
    }

    public String validateSession(String token) {
        if (isNullOrBlank(token)) return null;
        Session s = sessions.get(token);
        if (s == null) return null;

        // Reconfirm key using constant time to avoid subtle timing hints (defensive)
        for (Map.Entry<String, Session> e : sessions.entrySet()) {
            if (e.getValue() == s) {
                if (!constantTimeEquals(e.getKey(), token)) return null;
                break;
            }
        }

        long now = nowSeconds();
        if (now > s.expiryEpochSeconds) {
            sessions.remove(token);
            return null;
        }
        return s.userId;
    }

    public boolean refreshSession(String token, long additionalTtlSeconds) {
        if (isNullOrBlank(token) || additionalTtlSeconds <= 0) return false;
        if (additionalTtlSeconds > MAX_TTL_SECONDS) additionalTtlSeconds = MAX_TTL_SECONDS;

        return sessions.computeIfPresent(token, (k, s) -> {
            long now = nowSeconds();
            if (now > s.expiryEpochSeconds) {
                // expired; remove by returning null
                return null;
            }
            long newExpiry = s.expiryEpochSeconds + additionalTtlSeconds;
            long maxExpiry = now + MAX_TTL_SECONDS;
            if (newExpiry > maxExpiry) newExpiry = maxExpiry;
            s.expiryEpochSeconds = newExpiry;
            return s;
        }) != null;
    }

    public boolean endSession(String token) {
        if (isNullOrBlank(token)) return false;
        return sessions.remove(token) != null;
    }

    public int cleanupExpired() {
        long now = nowSeconds();
        int[] removed = new int[1];
        sessions.forEach((k, s) -> {
            if (now > s.expiryEpochSeconds) {
                if (sessions.remove(k, s)) {
                    removed[0]++;
                }
            }
        });
        return removed[0];
    }

    // Demonstration with 5 test cases
    public static void main(String[] args) throws Exception {
        Task123 mgr = new Task123();

        // Test 1: Create and validate
        String t1 = mgr.createSession("alice", 2);
        System.out.println("T1 token: " + t1);
        System.out.println("T1 validate: " + mgr.validateSession(t1));

        // Test 2: Validate random/invalid token
        System.out.println("T2 validate invalid: " + mgr.validateSession("invalid_token"));

        // Test 3: Refresh and validate
        boolean refreshed = mgr.refreshSession(t1, 5);
        System.out.println("T3 refreshed: " + refreshed + ", validate: " + mgr.validateSession(t1));

        // Test 4: End session and validate
        boolean ended = mgr.endSession(t1);
        System.out.println("T4 ended: " + ended + ", validate after end: " + mgr.validateSession(t1));

        // Test 5: Expiry and cleanup
        String t2 = mgr.createSession("bob", 1);
        System.out.println("T5 token: " + t2 + ", validate: " + mgr.validateSession(t2));
        Thread.sleep(2000);
        int cleaned = mgr.cleanupExpired();
        System.out.println("T5 cleaned: " + cleaned + ", validate after expiry: " + mgr.validateSession(t2));
    }
}