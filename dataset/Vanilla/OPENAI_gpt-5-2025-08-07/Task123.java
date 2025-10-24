import java.util.*;
import java.util.concurrent.atomic.AtomicLong;

public class Task123 {

    public static class Session {
        public final String id;
        public final String userId;
        public final long createdAt;
        public long lastAccess;
        public final long ttlMillis;
        public boolean invalidated;

        public Session(String id, String userId, long createdAt, long ttlMillis) {
            this.id = id;
            this.userId = userId;
            this.createdAt = createdAt;
            this.lastAccess = createdAt;
            this.ttlMillis = ttlMillis;
            this.invalidated = false;
        }

        @Override
        public String toString() {
            return "Session{id='" + id + "', userId='" + userId + "', createdAt=" + createdAt +
                    ", lastAccess=" + lastAccess + ", ttlMillis=" + ttlMillis +
                    ", invalidated=" + invalidated + "}";
        }
    }

    public static class SessionManager {
        private final Map<String, Session> sessions = new HashMap<>();
        private final AtomicLong counter = new AtomicLong(0);

        private boolean isExpired(Session s, long nowMillis) {
            return (nowMillis - s.lastAccess) > s.ttlMillis;
        }

        public String createSession(String userId, long nowMillis, long ttlMillis) {
            String id = "S" + counter.incrementAndGet();
            sessions.put(id, new Session(id, userId, nowMillis, ttlMillis));
            return id;
        }

        public boolean isActive(String sessionId, long nowMillis) {
            Session s = sessions.get(sessionId);
            if (s == null) return false;
            if (s.invalidated) return false;
            if (isExpired(s, nowMillis)) return false;
            return true;
        }

        public boolean touch(String sessionId, long nowMillis) {
            Session s = sessions.get(sessionId);
            if (s == null) return false;
            if (s.invalidated) return false;
            if (isExpired(s, nowMillis)) return false;
            s.lastAccess = nowMillis;
            return true;
        }

        public boolean invalidate(String sessionId) {
            Session s = sessions.get(sessionId);
            if (s == null) return false;
            if (s.invalidated) return false;
            s.invalidated = true;
            return true;
        }

        public int purgeExpired(long nowMillis) {
            List<String> toRemove = new ArrayList<>();
            for (Map.Entry<String, Session> e : sessions.entrySet()) {
                Session s = e.getValue();
                if (s.invalidated || isExpired(s, nowMillis)) {
                    toRemove.add(e.getKey());
                }
            }
            for (String id : toRemove) sessions.remove(id);
            return toRemove.size();
        }

        public List<String> listActiveSessions(String userId, long nowMillis) {
            List<String> ids = new ArrayList<>();
            for (Session s : sessions.values()) {
                if (s.userId.equals(userId) && !s.invalidated && !isExpired(s, nowMillis)) {
                    ids.add(s.id);
                }
            }
            Collections.sort(ids);
            return ids;
        }

        public String getSessionInfo(String sessionId) {
            Session s = sessions.get(sessionId);
            return s == null ? null : s.toString();
        }
    }

    public static void main(String[] args) {
        SessionManager mgr = new SessionManager();

        // Test 1: Create a session for Alice and check active
        String s1 = mgr.createSession("alice", 1000, 30000);
        System.out.println("Test1:isActive(s1@1000)=" + mgr.isActive(s1, 1000));
        System.out.println("Test1:listActive(alice@1000)=" + mgr.listActiveSessions("alice", 1000));

        // Test 2: Touch session and verify active before expiration
        System.out.println("Test2:touch(s1@20000)=" + mgr.touch(s1, 20000));
        System.out.println("Test2:isActive(s1@20000)=" + mgr.isActive(s1, 20000));

        // Test 3: Create second session for Alice and list
        String s2 = mgr.createSession("alice", 21000, 30000);
        System.out.println("Test3:listActive(alice@21000)=" + mgr.listActiveSessions("alice", 21000));

        // Test 4: Bob session expires and purge
        String sb = mgr.createSession("bob", 1000, 10000);
        System.out.println("Test4:isActive(sb@12000)=" + mgr.isActive(sb, 12000));
        System.out.println("Test4:purged@12000=" + mgr.purgeExpired(12000));
        System.out.println("Test4:listActive(bob@12000)=" + mgr.listActiveSessions("bob", 12000));

        // Test 5: Invalidate one of Alice's sessions and list
        System.out.println("Test5:invalidate(s1)=" + mgr.invalidate(s1));
        System.out.println("Test5:listActive(alice@22000)=" + mgr.listActiveSessions("alice", 22000));
    }
}