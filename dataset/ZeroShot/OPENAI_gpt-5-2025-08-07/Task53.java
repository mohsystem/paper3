import java.security.SecureRandom;
import java.time.Instant;
import java.util.Base64;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class Task53 {
    public static final class Session {
        public final String id;
        public final String userId;
        public final long createdAtEpochSec;
        public volatile long expiresAtEpochSec;
        public volatile long lastAccessedEpochSec;

        private Session(String id, String userId, long createdAt, long expiresAt) {
            this.id = id;
            this.userId = userId;
            this.createdAtEpochSec = createdAt;
            this.expiresAtEpochSec = expiresAt;
            this.lastAccessedEpochSec = createdAt;
        }
    }

    public static final class SessionManager {
        private final Map<String, Session> sessions = new ConcurrentHashMap<>();
        private final Map<String, Set<String>> userSessions = new ConcurrentHashMap<>();
        private final SecureRandom secureRandom = new SecureRandom();
        private final int idBytes;

        public SessionManager() {
            this(32);
        }

        public SessionManager(int idBytes) {
            if (idBytes < 16) throw new IllegalArgumentException("idBytes too small");
            this.idBytes = idBytes;
        }

        private String generateSessionId() {
            byte[] buf = new byte[idBytes];
            String id;
            do {
                secureRandom.nextBytes(buf);
                id = Base64.getUrlEncoder().withoutPadding().encodeToString(buf);
            } while (sessions.containsKey(id)); // avoid collisions
            return id;
        }

        private static boolean ctEquals(String a, String b) {
            if (a == null || b == null) return false;
            int lenA = a.length();
            int lenB = b.length();
            int result = lenA ^ lenB;
            int len = Math.min(lenA, lenB);
            for (int i = 0; i < len; i++) {
                result |= a.charAt(i) ^ b.charAt(i);
            }
            return result == 0;
        }

        private static long nowSec() {
            return Instant.now().getEpochSecond();
        }

        private boolean isExpired(Session s, long now) {
            return s.expiresAtEpochSec <= now;
        }

        private void addUserSession(String userId, String sessionId) {
            userSessions.compute(userId, (k, v) -> {
                if (v == null) v = Collections.synchronizedSet(new HashSet<>());
                v.add(sessionId);
                return v;
            });
        }

        private void removeUserSession(String userId, String sessionId) {
            Set<String> set = userSessions.get(userId);
            if (set != null) {
                synchronized (set) {
                    set.remove(sessionId);
                    if (set.isEmpty()) {
                        userSessions.remove(userId);
                    }
                }
            }
        }

        public String createSession(String userId, long ttlSeconds) {
            if (userId == null || userId.isEmpty()) throw new IllegalArgumentException("userId");
            if (ttlSeconds <= 0) throw new IllegalArgumentException("ttlSeconds must be > 0");
            String id = generateSessionId();
            long now = nowSec();
            Session s = new Session(id, userId, now, now + ttlSeconds);
            sessions.put(id, s);
            addUserSession(userId, id);
            return id;
        }

        public boolean validateSession(String sessionId) {
            if (sessionId == null) return false;
            Session s = sessions.get(sessionId);
            if (s == null) return false;
            long now = nowSec();
            if (isExpired(s, now)) {
                invalidateSession(sessionId);
                return false;
            }
            s.lastAccessedEpochSec = now;
            return true;
        }

        public String getUserForSession(String sessionId) {
            Session s = sessions.get(sessionId);
            if (s == null) return null;
            long now = nowSec();
            if (isExpired(s, now)) {
                invalidateSession(sessionId);
                return null;
            }
            s.lastAccessedEpochSec = now;
            return s.userId;
        }

        public boolean invalidateSession(String sessionId) {
            if (sessionId == null) return false;
            Session s = sessions.remove(sessionId);
            if (s != null) {
                removeUserSession(s.userId, sessionId);
                return true;
            }
            return false;
        }

        public int invalidateAllSessionsForUser(String userId) {
            if (userId == null) return 0;
            Set<String> set = userSessions.remove(userId);
            int count = 0;
            if (set != null) {
                synchronized (set) {
                    for (String sid : set) {
                        Session removed = sessions.remove(sid);
                        if (removed != null) count++;
                    }
                    set.clear();
                }
            }
            return count;
        }

        public boolean touchSession(String sessionId, long extendTtlSeconds) {
            if (extendTtlSeconds <= 0) return false;
            Session s = sessions.get(sessionId);
            if (s == null) return false;
            long now = nowSec();
            if (isExpired(s, now)) {
                invalidateSession(sessionId);
                return false;
            }
            s.expiresAtEpochSec = Math.max(s.expiresAtEpochSec, now) + extendTtlSeconds;
            s.lastAccessedEpochSec = now;
            return true;
        }

        public String rotateSession(String sessionId) {
            Session s = sessions.get(sessionId);
            if (s == null) return null;
            long now = nowSec();
            if (isExpired(s, now)) {
                invalidateSession(sessionId);
                return null;
            }
            long remaining = Math.max(0, s.expiresAtEpochSec - now);
            if (remaining == 0) {
                invalidateSession(sessionId);
                return null;
            }
            String newId = generateSessionId();
            Session n = new Session(newId, s.userId, now, now + remaining);
            sessions.put(newId, n);
            addUserSession(s.userId, newId);
            invalidateSession(sessionId);
            return newId;
        }

        public int cleanupExpiredSessions() {
            long now = nowSec();
            int count = 0;
            for (Map.Entry<String, Session> e : new HashMap<>(sessions).entrySet()) {
                Session s = e.getValue();
                if (isExpired(s, now)) {
                    if (sessions.remove(e.getKey(), s)) {
                        removeUserSession(s.userId, s.id);
                        count++;
                    }
                }
            }
            return count;
        }

        public boolean secureCompareIds(String a, String b) {
            return ctEquals(a, b);
        }
    }

    public static void main(String[] args) throws Exception {
        SessionManager sm = new SessionManager();

        // Test 1: Create and validate
        String s1 = sm.createSession("alice", 5);
        System.out.println("Test1 valid=" + sm.validateSession(s1) + " user=" + sm.getUserForSession(s1));

        // Test 2: Touch/extend session
        boolean touched = sm.touchSession(s1, 5);
        System.out.println("Test2 touched=" + touched + " stillValid=" + sm.validateSession(s1));

        // Test 3: Rotate session
        String rotated = sm.rotateSession(s1);
        System.out.println("Test3 oldValid=" + sm.validateSession(s1) + " newValid=" + sm.validateSession(rotated));

        // Test 4: Expiration
        String s2 = sm.createSession("bob", 1);
        Thread.sleep(1500);
        System.out.println("Test4 expiredValid=" + sm.validateSession(s2));

        // Test 5: Invalidate all sessions for a user
        String u3s1 = sm.createSession("carol", 10);
        String u3s2 = sm.createSession("carol", 10);
        int invalidated = sm.invalidateAllSessionsForUser("carol");
        System.out.println("Test5 invalidated=" + invalidated + " s1Valid=" + sm.validateSession(u3s1) + " s2Valid=" + sm.validateSession(u3s2));
    }
}