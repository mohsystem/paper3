import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Collections;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class Task43 {

    public static class Session {
        final String token;
        final String userId;
        final String clientIP;
        final String userAgent;
        final long expiresAt;

        Session(String token, String userId, String clientIP, String userAgent, long expiresAt) {
            this.token = token;
            this.userId = userId;
            this.clientIP = clientIP;
            this.userAgent = userAgent;
            this.expiresAt = expiresAt;
        }
    }

    public static class SessionManager {
        private final SecureRandom secureRandom = new SecureRandom();
        private final Map<String, Session> sessions = new ConcurrentHashMap<>();
        private final Map<String, Set<String>> userIndex = new ConcurrentHashMap<>();
        private final long defaultTtlMs;

        public SessionManager(long defaultTtlMs) {
            this.defaultTtlMs = defaultTtlMs;
        }

        private static boolean isNullOrTooLong(String s, int max) {
            return s == null || s.length() == 0 || s.length() > max;
        }

        private static boolean ctEquals(String a, String b) {
            if (a == null || b == null) return false;
            byte[] x = a.getBytes(java.nio.charset.StandardCharsets.UTF_8);
            byte[] y = b.getBytes(java.nio.charset.StandardCharsets.UTF_8);
            return MessageDigest.isEqual(x, y);
        }

        private static String bytesToHex(byte[] bytes) {
            char[] hexArray = "0123456789abcdef".toCharArray();
            char[] hexChars = new char[bytes.length * 2];
            for (int j = 0; j < bytes.length; j++) {
                int v = bytes[j] & 0xFF;
                hexChars[j * 2] = hexArray[v >>> 4];
                hexChars[j * 2 + 1] = hexArray[v & 0x0F];
            }
            return new String(hexChars);
        }

        private String newToken() {
            byte[] buf = new byte[32];
            secureRandom.nextBytes(buf);
            return bytesToHex(buf);
        }

        private void indexAdd(String userId, String token) {
            userIndex.computeIfAbsent(userId, k -> Collections.newSetFromMap(new ConcurrentHashMap<>())).add(token);
        }

        private void indexRemove(String userId, String token) {
            Set<String> set = userIndex.get(userId);
            if (set != null) {
                set.remove(token);
                if (set.isEmpty()) userIndex.remove(userId);
            }
        }

        private void cleanupExpired() {
            long now = System.currentTimeMillis();
            for (Map.Entry<String, Session> e : sessions.entrySet()) {
                Session s = e.getValue();
                if (s.expiresAt <= now) {
                    sessions.remove(e.getKey());
                    indexRemove(s.userId, e.getKey());
                }
            }
        }

        public String createSession(String userId, String clientIP, String userAgent) {
            return createSession(userId, clientIP, userAgent, defaultTtlMs);
        }

        public String createSession(String userId, String clientIP, String userAgent, long ttlMs) {
            cleanupExpired();
            if (isNullOrTooLong(userId, 128) || isNullOrTooLong(clientIP, 64) || isNullOrTooLong(userAgent, 256)) {
                return null;
            }
            String token = newToken();
            long expires = System.currentTimeMillis() + Math.max(1_000L, Math.min(ttlMs, 86_400_000L));
            Session s = new Session(token, userId, clientIP, userAgent, expires);
            sessions.put(token, s);
            indexAdd(userId, token);
            return token;
        }

        public boolean validateSession(String token, String clientIP, String userAgent) {
            cleanupExpired();
            if (isNullOrTooLong(token, 128) || isNullOrTooLong(clientIP, 64) || isNullOrTooLong(userAgent, 256)) {
                return false;
            }
            Session s = sessions.get(token);
            if (s == null) return false;
            if (s.expiresAt <= System.currentTimeMillis()) {
                sessions.remove(token);
                indexRemove(s.userId, token);
                return false;
            }
            return ctEquals(s.clientIP, clientIP) && ctEquals(s.userAgent, userAgent);
        }

        public String getUserIdIfValid(String token, String clientIP, String userAgent) {
            return validateSession(token, clientIP, userAgent) ? sessions.get(token).userId : null;
        }

        public String refreshSession(String token, String clientIP, String userAgent) {
            cleanupExpired();
            if (!validateSession(token, clientIP, userAgent)) return null;
            Session old = sessions.get(token);
            if (old == null) return null;
            String newTok = newToken();
            long expires = System.currentTimeMillis() + defaultTtlMs;
            Session newer = new Session(newTok, old.userId, old.clientIP, old.userAgent, expires);
            sessions.put(newTok, newer);
            indexAdd(old.userId, newTok);
            sessions.remove(token);
            indexRemove(old.userId, token);
            return newTok;
        }

        public boolean terminateSession(String token) {
            Session s = sessions.remove(token);
            if (s != null) {
                indexRemove(s.userId, token);
                return true;
            }
            return false;
        }

        public int terminateAllSessionsForUser(String userId) {
            if (isNullOrTooLong(userId, 128)) return 0;
            Set<String> tokens = userIndex.remove(userId);
            if (tokens == null) return 0;
            int count = 0;
            for (String t : tokens) {
                if (sessions.remove(t) != null) count++;
            }
            return count;
        }
    }

    public static void main(String[] args) {
        SessionManager sm = new SessionManager(30 * 60 * 1000L);
        String ip = "203.0.113.10";
        String ua = "ExampleBrowser/1.0";
        String badIp = "198.51.100.5";

        // Test 1: Create and validate session
        String tok1 = sm.createSession("alice", ip, ua);
        System.out.println("T1 valid: " + sm.validateSession(tok1, ip, ua));

        // Test 2: Validate with wrong IP (should fail)
        System.out.println("T2 wrong IP valid: " + sm.validateSession(tok1, badIp, ua));

        // Test 3: Refresh session rotates token
        String tok2 = sm.refreshSession(tok1, ip, ua);
        System.out.println("T3 old valid: " + sm.validateSession(tok1, ip, ua));
        System.out.println("T3 new valid: " + sm.validateSession(tok2, ip, ua));

        // Test 4: Force expire and validate
        // Simulate expire by terminating and recreating with 1s TTL and waiting
        String tok3 = sm.createSession("bob", ip, ua, 1000L);
        try { Thread.sleep(1200L); } catch (InterruptedException ignored) {}
        System.out.println("T4 expired valid: " + sm.validateSession(tok3, ip, ua));

        // Test 5: Terminate session
        String tok4 = sm.createSession("carol", ip, ua);
        System.out.println("T5 before terminate: " + sm.validateSession(tok4, ip, ua));
        sm.terminateSession(tok4);
        System.out.println("T5 after terminate: " + sm.validateSession(tok4, ip, ua));
    }
}