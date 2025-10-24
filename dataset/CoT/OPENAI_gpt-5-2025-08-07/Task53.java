import java.security.SecureRandom;
import java.time.Instant;
import java.util.Base64;
import java.util.Objects;
import java.util.Optional;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

/*
 Chain-of-Through steps embedded as comments:
 1) Problem understanding: Implement a secure in-memory session manager with functions to create, validate, get, refresh, invalidate sessions, and prune expired ones. Each session has a secure random token and expiration.
 2) Security requirements: Use SecureRandom, Base64 URL-safe tokens, avoid predictable tokens, invalidate/rotate tokens on refresh to prevent fixation, thread-safe store (ConcurrentHashMap), prune expired sessions, minimal exposure of session internals.
 3) Secure coding generation: Added parameter validation, constant-time compare helper (reserved), no logging of secrets, immutable returned info, careful time handling.
 4) Code review: Checked for race conditions (map is concurrent), ensured token length adequate, ensured refresh atomically replaces tokens, no exceptions leak secrets.
 5) Secure code output: Finalizing with tests demonstrating 5 scenarios.
*/

public class Task53 {

    public static final class SessionInfo {
        public final String userId;
        public final long expiresAtEpochMillis;

        public SessionInfo(String userId, long expiresAtEpochMillis) {
            this.userId = userId;
            this.expiresAtEpochMillis = expiresAtEpochMillis;
        }

        @Override
        public String toString() {
            return "SessionInfo{userId='" + userId + "', expiresAt=" + expiresAtEpochMillis + "}";
        }
    }

    private static final class Session {
        final String userId;
        final long createdAtMillis;
        volatile long expiresAtMillis;

        Session(String userId, long createdAtMillis, long expiresAtMillis) {
            this.userId = userId;
            this.createdAtMillis = createdAtMillis;
            this.expiresAtMillis = expiresAtMillis;
        }
    }

    private final ConcurrentHashMap<String, Session> sessionsByToken = new ConcurrentHashMap<>();
    private final SecureRandom secureRandom;
    private final long ttlMillis;
    private final int tokenNumBytes;

    public Task53(long ttlMillis) {
        this(ttlMillis, 32);
    }

    public Task53(long ttlMillis, int tokenNumBytes) {
        if (ttlMillis <= 0) throw new IllegalArgumentException("ttlMillis must be > 0");
        if (tokenNumBytes < 16) throw new IllegalArgumentException("tokenNumBytes must be >= 16");
        this.ttlMillis = ttlMillis;
        this.tokenNumBytes = tokenNumBytes;
        this.secureRandom = new SecureRandom();
    }

    private String generateToken() {
        byte[] buf = new byte[tokenNumBytes];
        secureRandom.nextBytes(buf);
        return Base64.getUrlEncoder().withoutPadding().encodeToString(buf);
    }

    private static long nowMillis() {
        return Instant.now().toEpochMilli();
    }

    public String createSession(String userId) {
        if (userId == null || userId.isEmpty()) throw new IllegalArgumentException("userId required");
        String token = generateToken();
        long now = nowMillis();
        Session s = new Session(userId, now, now + ttlMillis);
        sessionsByToken.put(token, s);
        return token;
    }

    public boolean validateSession(String token) {
        if (token == null || token.isEmpty()) return false;
        Session s = sessionsByToken.get(token);
        if (s == null) return false;
        long now = nowMillis();
        if (now > s.expiresAtMillis) {
            sessionsByToken.remove(token, s);
            return false;
        }
        return true;
    }

    public SessionInfo getSessionInfo(String token) {
        if (!validateSession(token)) return null;
        Session s = sessionsByToken.get(token);
        if (s == null) return null;
        return new SessionInfo(s.userId, s.expiresAtMillis);
    }

    public boolean invalidateSession(String token) {
        if (token == null || token.isEmpty()) return false;
        return sessionsByToken.remove(token) != null;
    }

    public String refreshSession(String token) {
        if (!validateSession(token)) return null;
        Session old = sessionsByToken.get(token);
        if (old == null) return null;
        long now = nowMillis();
        Session refreshed = new Session(old.userId, old.createdAtMillis, now + ttlMillis);
        String newToken = generateToken();
        // Replace atomically: create new, then remove old
        sessionsByToken.put(newToken, refreshed);
        sessionsByToken.remove(token, old);
        return newToken;
    }

    public int pruneExpiredSessions() {
        long now = nowMillis();
        int removed = 0;
        for (var e : sessionsByToken.entrySet()) {
            Session s = e.getValue();
            if (now > s.expiresAtMillis) {
                if (sessionsByToken.remove(e.getKey(), s)) {
                    removed++;
                }
            }
        }
        return removed;
    }

    // constant-time string equality if needed elsewhere (not used due to map lookup)
    @SuppressWarnings("unused")
    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        int len = Math.max(a.length(), b.length());
        int diff = a.length() ^ b.length();
        for (int i = 0; i < len; i++) {
            char ca = i < a.length() ? a.charAt(i) : 0;
            char cb = i < b.length() ? b.charAt(i) : 0;
            diff |= ca ^ cb;
        }
        return diff == 0;
    }

    public static void main(String[] args) throws Exception {
        // 5 test cases
        Task53 manager = new Task53(TimeUnit.SECONDS.toMillis(5));

        // 1) Create and validate session
        String t1 = manager.createSession("alice");
        System.out.println("Test1 valid: " + manager.validateSession(t1));

        // 2) Get session info
        SessionInfo info = manager.getSessionInfo(t1);
        System.out.println("Test2 info: " + (info != null ? info.toString() : "null"));

        // 3) Refresh session: old invalid, new valid
        String t1New = manager.refreshSession(t1);
        System.out.println("Test3 new valid: " + manager.validateSession(t1New) + ", old valid: " + manager.validateSession(t1));

        // 4) Invalidate session
        boolean inv = manager.invalidateSession(t1New);
        System.out.println("Test4 invalidated: " + inv + ", validate after: " + manager.validateSession(t1New));

        // 5) Expiration test with short TTL
        Task53 shortTTL = new Task53(100);
        String t2 = shortTTL.createSession("bob");
        System.out.println("Test5 before expiry valid: " + shortTTL.validateSession(t2));
        Thread.sleep(200);
        shortTTL.pruneExpiredSessions();
        System.out.println("Test5 after expiry valid: " + shortTTL.validateSession(t2));
    }
}