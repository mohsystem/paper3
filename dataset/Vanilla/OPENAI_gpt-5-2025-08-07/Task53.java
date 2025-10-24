import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Task53 {
    static class Session {
        String sessionId;
        String userId;
        long expiryMillis;
        Map<String, String> attrs = new ConcurrentHashMap<>();
        Session(String sessionId, String userId, long expiryMillis) {
            this.sessionId = sessionId;
            this.userId = userId;
            this.expiryMillis = expiryMillis;
        }
    }

    static class SessionManager {
        private final Map<String, Session> sessions = new ConcurrentHashMap<>();

        private long now() {
            return System.currentTimeMillis();
        }

        public String createSession(String userId, long ttlMillis) {
            String sid = UUID.randomUUID().toString();
            long expiry = now() + ttlMillis;
            sessions.put(sid, new Session(sid, userId, expiry));
            return sid;
        }

        public boolean validateSession(String sessionId) {
            Session s = sessions.get(sessionId);
            if (s == null) return false;
            if (s.expiryMillis < now()) {
                sessions.remove(sessionId);
                return false;
            }
            return true;
        }

        public String getUser(String sessionId) {
            if (!validateSession(sessionId)) return null;
            return sessions.get(sessionId).userId;
        }

        public boolean setAttribute(String sessionId, String key, String value) {
            if (!validateSession(sessionId)) return false;
            sessions.get(sessionId).attrs.put(key, value);
            return true;
        }

        public String getAttribute(String sessionId, String key) {
            if (!validateSession(sessionId)) return null;
            return sessions.get(sessionId).attrs.getOrDefault(key, null);
        }

        public boolean touch(String sessionId, long ttlExtendMillis) {
            if (!validateSession(sessionId)) return false;
            sessions.get(sessionId).expiryMillis = now() + ttlExtendMillis;
            return true;
        }

        public boolean invalidateSession(String sessionId) {
            return sessions.remove(sessionId) != null;
        }

        public int cleanupExpired() {
            int removed = 0;
            long t = now();
            Iterator<Map.Entry<String, Session>> it = sessions.entrySet().iterator();
            while (it.hasNext()) {
                Map.Entry<String, Session> e = it.next();
                if (e.getValue().expiryMillis < t) {
                    it.remove();
                    removed++;
                }
            }
            return removed;
        }

        public int countSessions() {
            return sessions.size();
        }
    }

    public static void main(String[] args) {
        SessionManager sm = new SessionManager();

        // Test 1: Create and validate session
        String sid1 = sm.createSession("userA", 10000);
        System.out.println("T1_valid=" + sm.validateSession(sid1) + ", user=" + sm.getUser(sid1) + ", count=" + sm.countSessions());

        // Test 2: Set and get attribute
        sm.setAttribute(sid1, "role", "admin");
        System.out.println("T2_role=" + sm.getAttribute(sid1, "role"));

        // Test 3: Touch session to extend TTL
        boolean touched = sm.touch(sid1, 10000);
        System.out.println("T3_touched=" + touched + ", stillValid=" + sm.validateSession(sid1));

        // Test 4: Invalidate session
        sm.invalidateSession(sid1);
        System.out.println("T4_validAfterInvalidate=" + sm.validateSession(sid1) + ", count=" + sm.countSessions());

        // Test 5: Expired sessions cleanup
        String sid2 = sm.createSession("userB", 0); // expires immediately
        String sid3 = sm.createSession("userC", -1000); // already expired
        int removed = sm.cleanupExpired();
        System.out.println("T5_removed=" + removed + ", finalCount=" + sm.countSessions());
    }
}