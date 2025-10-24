// Chain-of-Through Step 1: Problem understanding
// Build a secure in-memory session manager that can create, validate, refresh, destroy,
// purge expired sessions, and revoke all sessions for a user.

// Chain-of-Through Step 2: Security requirements
// - Use cryptographically secure random tokens
// - Validate and clamp inputs (e.g., TTL and userId lengths)
// - Avoid leaking internal data structures; return copies where applicable
// - Thread-safe operations on shared state

// Chain-of-Through Step 3: Secure coding generation
// Implement with SecureRandom, ConcurrentHashMap, and safe validation.

// Chain-of-Through Step 4: Code review
// Checked for null checks, bounds, thread safety, and safe token generation.

// Chain-of-Through Step 5: Secure code output
// Final code below.

import java.security.SecureRandom;
import java.util.Base64;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentHashMap;

public class Task123 {
    private static final int TOKEN_BYTES = 32; // 256-bit tokens
    private static final int MAX_USERID_LEN = 128;
    private static final long MIN_TTL_SEC = 1;
    private static final long MAX_TTL_SEC = 7 * 24 * 60 * 60; // 7 days

    private static final SecureRandom RNG = new SecureRandom();

    private static final class Session {
        final String userId;
        final long createdAtEpochSec;
        final long ttlSec;
        volatile long expiresAtEpochSec;

        Session(String userId, long ttlSec) {
            this.userId = userId;
            this.createdAtEpochSec = nowSec();
            this.ttlSec = ttlSec;
            this.expiresAtEpochSec = this.createdAtEpochSec + ttlSec;
        }
    }

    private final ConcurrentHashMap<String, Session> sessions = new ConcurrentHashMap<>();

    private static long nowSec() {
        return System.currentTimeMillis() / 1000L;
    }

    private static String generateToken() {
        byte[] buf = new byte[TOKEN_BYTES];
        RNG.nextBytes(buf);
        // URL-safe, no padding
        return Base64.getUrlEncoder().withoutPadding().encodeToString(buf);
    }

    private static long clampTtl(long ttlSec) {
        if (ttlSec < MIN_TTL_SEC) return MIN_TTL_SEC;
        if (ttlSec > MAX_TTL_SEC) return MAX_TTL_SEC;
        return ttlSec;
    }

    private static boolean isValidUserId(String userId) {
        return userId != null && !userId.isEmpty() && userId.length() <= MAX_USERID_LEN;
    }

    public String createSession(String userId, long ttlSec) {
        if (!isValidUserId(userId)) {
            throw new IllegalArgumentException("Invalid userId");
        }
        long ttl = clampTtl(ttlSec);
        String token;
        // Ensure uniqueness (practically guaranteed, but loop to be safe)
        do {
            token = generateToken();
        } while (sessions.containsKey(token));
        sessions.put(token, new Session(userId, ttl));
        return token;
    }

    public boolean isValid(String token) {
        if (token == null) return false;
        Session s = sessions.get(token);
        if (s == null) return false;
        if (nowSec() >= s.expiresAtEpochSec) {
            sessions.remove(token);
            return false;
        }
        return true;
    }

    public boolean refresh(String token) {
        if (token == null) return false;
        Session s = sessions.get(token);
        if (s == null) return false;
        long now = nowSec();
        if (now >= s.expiresAtEpochSec) {
            sessions.remove(token);
            return false;
        }
        s.expiresAtEpochSec = now + s.ttlSec;
        return true;
    }

    public boolean destroy(String token) {
        if (token == null) return false;
        return sessions.remove(token) != null;
    }

    public int purgeExpired() {
        long now = nowSec();
        int removed = 0;
        for (Map.Entry<String, Session> e : sessions.entrySet()) {
            Session s = e.getValue();
            if (now >= s.expiresAtEpochSec) {
                if (sessions.remove(e.getKey(), s)) {
                    removed++;
                }
            }
        }
        return removed;
    }

    public int revokeUserSessions(String userId) {
        if (!isValidUserId(userId)) return 0;
        int removed = 0;
        for (Map.Entry<String, Session> e : sessions.entrySet()) {
            Session s = e.getValue();
            if (s.userId.equals(userId)) {
                if (sessions.remove(e.getKey(), s)) {
                    removed++;
                }
            }
        }
        return removed;
    }

    public int countActiveSessions() {
        purgeExpired();
        return sessions.size();
    }

    // Test helper only
    public boolean debugForceExpire(String token) {
        Session s = sessions.get(token);
        if (s == null) return false;
        s.expiresAtEpochSec = nowSec() - 1;
        return true;
    }

    public static void main(String[] args) {
        Task123 mgr = new Task123();

        // Test 1: Create and validate session
        String s1 = mgr.createSession("userA", 60);
        System.out.println("T1 valid: " + mgr.isValid(s1));

        // Test 2: Create, force expire, validate false, purge
        String s2 = mgr.createSession("userB", 5);
        mgr.debugForceExpire(s2);
        System.out.println("T2 valid after force expire: " + mgr.isValid(s2));
        System.out.println("T2 purge removed: " + mgr.purgeExpired());

        // Test 3: Refresh session
        System.out.println("T3 refresh s1: " + mgr.refresh(s1));
        System.out.println("T3 s1 still valid: " + mgr.isValid(s1));

        // Test 4: Destroy session
        System.out.println("T4 destroy s1: " + mgr.destroy(s1));
        System.out.println("T4 s1 valid after destroy: " + mgr.isValid(s1));

        // Test 5: Revoke user sessions
        String a = mgr.createSession("userC", 10);
        String b = mgr.createSession("userC", 10);
        String c = mgr.createSession("userC", 10);
        System.out.println("T5 revoke userC count: " + mgr.revokeUserSessions("userC"));
        System.out.println("T5 active sessions: " + mgr.countActiveSessions());
    }
}