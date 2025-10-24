import java.security.SecureRandom;
import java.nio.charset.StandardCharsets;
import java.util.Map;
import java.util.HashMap;
import java.util.Iterator;

public final class Task123 {

    // Session record
    private static final class Session {
        private final String user;
        private long expiresAt; // epoch seconds
        private long lastAccess; // epoch seconds

        private Session(String user, long expiresAt, long lastAccess) {
            this.user = user;
            this.expiresAt = expiresAt;
            this.lastAccess = lastAccess;
        }
    }

    // Session manager
    public static final class SessionManager {
        private final Map<String, Session> sessions = new HashMap<>();

        // Adds a session with provided token and TTL. Returns true on success.
        public boolean addSession(String token, String user, long ttlSeconds) {
            if (!isValidToken(token) || !isValidUser(user)) return false;
            long ttl = clampTtl(ttlSeconds);
            long now = nowSeconds();
            if (ttl <= 0) return false;
            // Avoid overwriting an existing token
            if (sessions.containsKey(token)) return false;
            sessions.put(token, new Session(user, now + ttl, now));
            return true;
        }

        // Returns associated user for valid, non-expired token; empty string otherwise.
        public String validateSession(String token) {
            if (!isValidToken(token)) return "";
            Session s = sessions.get(token);
            if (s == null) return "";
            long now = nowSeconds();
            if (now > s.expiresAt) {
                sessions.remove(token);
                return "";
            }
            s.lastAccess = now;
            return s.user;
        }

        // Rotates a token: oldToken -> newToken with new TTL. Returns true on success.
        public boolean refreshSession(String oldToken, String newToken, long ttlSeconds) {
            if (!isValidToken(oldToken) || !isValidToken(newToken)) return false;
            if (constantTimeEquals(oldToken, newToken)) return false;
            Session s = sessions.get(oldToken);
            if (s == null) return false;
            long now = nowSeconds();
            if (now > s.expiresAt) {
                sessions.remove(oldToken);
                return false;
            }
            if (sessions.containsKey(newToken)) return false;
            long ttl = clampTtl(ttlSeconds);
            Session ns = new Session(s.user, now + ttl, now);
            sessions.remove(oldToken);
            sessions.put(newToken, ns);
            return true;
        }

        // Revokes a session token. Returns true if removed.
        public boolean revokeSession(String token) {
            if (!isValidToken(token)) return false;
            return sessions.remove(token) != null;
        }

        // Removes expired sessions; returns count removed.
        public int pruneExpired() {
            int removed = 0;
            long now = nowSeconds();
            Iterator<Map.Entry<String, Session>> it = sessions.entrySet().iterator();
            while (it.hasNext()) {
                Map.Entry<String, Session> e = it.next();
                if (now > e.getValue().expiresAt) {
                    it.remove();
                    removed++;
                }
            }
            return removed;
        }

        // Counts non-expired sessions.
        public int countActive() {
            pruneExpired();
            return sessions.size();
        }
    }

    // Helper: token generation (for testing; do not log tokens)
    private static String generateToken(int bytes) {
        if (bytes < 16) bytes = 16;
        if (bytes > 64) bytes = 64;
        byte[] b = new byte[bytes];
        SecureRandom sr = new SecureRandom();
        sr.nextBytes(b);
        // Base64 URL-safe without padding
        String t = java.util.Base64.getUrlEncoder().withoutPadding().encodeToString(b);
        // Ensure allowed charset (Base64 URL already compliant)
        return t;
    }

    // Validation helpers
    private static boolean isValidToken(String token) {
        if (token == null) return false;
        int len = token.length();
        if (len < 16 || len > 256) return false;
        for (int i = 0; i < len; i++) {
            char c = token.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '-' || c == '_';
            if (!ok) return false;
        }
        return true;
    }

    private static boolean isValidUser(String user) {
        if (user == null) return false;
        int len = user.length();
        if (len < 1 || len > 64) return false;
        // Allow letters, digits, underscore, dash, dot
        for (int i = 0; i < len; i++) {
            char c = user.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') ||
                         (c >= 'a' && c <= 'z') ||
                         (c >= '0' && c <= '9') ||
                         c == '-' || c == '_' || c == '.';
            if (!ok) return false;
        }
        return true;
    }

    private static long clampTtl(long ttl) {
        if (ttl < 1) return 0;
        long max = 86_400L; // 1 day
        if (ttl > max) return max;
        return ttl;
    }

    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) return false;
        byte[] x = a.getBytes(StandardCharsets.UTF_8);
        byte[] y = b.getBytes(StandardCharsets.UTF_8);
        int lx = x.length, ly = y.length;
        int max = Math.max(lx, ly);
        int diff = lx ^ ly;
        for (int i = 0; i < max; i++) {
            byte bx = i < lx ? x[i] : 0;
            byte by = i < ly ? y[i] : 0;
            diff |= (bx ^ by);
        }
        return diff == 0;
    }

    private static long nowSeconds() {
        return System.currentTimeMillis() / 1000L;
    }

    // Demonstration tests (no sensitive token output)
    public static void main(String[] args) throws Exception {
        SessionManager mgr = new SessionManager();

        // Test 1: Create and validate session
        String tok1 = generateToken(32);
        boolean add1 = mgr.addSession(tok1, "alice", 5);
        String v1 = mgr.validateSession(tok1);
        System.out.println("T1 add=" + add1 + " validUser=" + v1 + " active=" + mgr.countActive());

        // Test 2: Refresh token
        String tok2 = generateToken(32);
        boolean ref = mgr.refreshSession(tok1, tok2, 5);
        String vOld = mgr.validateSession(tok1);
        String vNew = mgr.validateSession(tok2);
        System.out.println("T2 refresh=" + ref + " oldValid='" + vOld + "' newValid='" + vNew + "' active=" + mgr.countActive());

        // Test 3: Revoke token
        boolean rev = mgr.revokeSession(tok2);
        String v3 = mgr.validateSession(tok2);
        System.out.println("T3 revoke=" + rev + " postRevokeValid='" + v3 + "' active=" + mgr.countActive());

        // Test 4: Expiration
        String tok3 = generateToken(24);
        boolean add2 = mgr.addSession(tok3, "bob", 1);
        Thread.sleep(1500);
        mgr.pruneExpired();
        String v4 = mgr.validateSession(tok3);
        System.out.println("T4 add=" + add2 + " afterExpireValid='" + v4 + "' active=" + mgr.countActive());

        // Test 5: Invalid token input
        boolean addBad = mgr.addSession("short", "charlie", 10);
        System.out.println("T5 invalidTokenAdd=" + addBad + " active=" + mgr.countActive());
    }
}