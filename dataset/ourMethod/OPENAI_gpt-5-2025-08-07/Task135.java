import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Task135 {

    // CSRF Token Manager using synchronizer token pattern with in-memory store
    public static final class CsrfTokenStore {
        private final ConcurrentHashMap<String, ConcurrentHashMap<String, Long>> store = new ConcurrentHashMap<>();
        private final SecureRandom secureRandom = new SecureRandom();
        private final long ttlSeconds;

        public CsrfTokenStore(long ttlSeconds) {
            if (ttlSeconds <= 0 || ttlSeconds > 86400L) {
                throw new IllegalArgumentException("Invalid TTL");
            }
            this.ttlSeconds = ttlSeconds;
        }

        private static boolean isValidSessionId(String sessionId) {
            if (sessionId == null) return false;
            int len = sessionId.length();
            if (len < 1 || len > 64) return false;
            for (int i = 0; i < len; i++) {
                char c = sessionId.charAt(i);
                if (!(Character.isAlphabetic(c) || Character.isDigit(c) || c == '-' || c == '_')) {
                    return false;
                }
            }
            return true;
        }

        private static long nowSeconds() {
            return Instant.now().getEpochSecond();
        }

        private void purgeExpired() {
            long now = nowSeconds();
            for (Map.Entry<String, ConcurrentHashMap<String, Long>> e : store.entrySet()) {
                ConcurrentHashMap<String, Long> m = e.getValue();
                for (Map.Entry<String, Long> te : m.entrySet()) {
                    Long exp = te.getValue();
                    if (exp == null || exp <= now) {
                        m.remove(te.getKey());
                    }
                }
                if (m.isEmpty()) {
                    store.remove(e.getKey());
                }
            }
        }

        public String generateToken(String sessionId) {
            if (!isValidSessionId(sessionId)) {
                throw new IllegalArgumentException("Invalid sessionId");
            }
            byte[] bytes = new byte[32];
            secureRandom.nextBytes(bytes);
            String token = Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
            long exp = nowSeconds() + ttlSeconds;
            store.computeIfAbsent(sessionId, k -> new ConcurrentHashMap<>()).put(token, exp);
            purgeExpired();
            return token;
        }

        public boolean validateToken(String sessionId, String token) {
            if (!isValidSessionId(sessionId)) return false;
            if (token == null) return false;
            int tlen = token.length();
            if (tlen < 16 || tlen > 256) return false;

            ConcurrentHashMap<String, Long> m = store.get(sessionId);
            if (m == null) return false;
            Long exp = m.get(token);
            long now = nowSeconds();
            if (exp == null) {
                purgeExpired();
                return false;
            }
            if (exp <= now) {
                m.remove(token);
                purgeExpired();
                return false;
            }
            // One-time use to prevent replay
            m.remove(token);
            if (m.isEmpty()) {
                store.remove(sessionId);
            }
            purgeExpired();
            return true;
        }

        // Test helper to force-expire a token (not for production)
        public boolean expireTokenForTest(String sessionId, String token) {
            ConcurrentHashMap<String, Long> m = store.get(sessionId);
            if (m == null) return false;
            if (!m.containsKey(token)) return false;
            m.put(token, nowSeconds() - 1);
            return true;
        }
    }

    // Simple constant-time comparer if needed for future use
    public static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a == null || b == null) return false;
        int len = Math.max(a.length, b.length);
        int diff = a.length ^ b.length;
        for (int i = 0; i < len; i++) {
            byte ba = (i < a.length) ? a[i] : 0;
            byte bb = (i < b.length) ? b[i] : 0;
            diff |= (ba ^ bb);
        }
        return diff == 0;
    }

    public static void main(String[] args) {
        CsrfTokenStore store = new CsrfTokenStore(600);

        // Test 1: Valid token for a session should validate
        String s1 = "sessA";
        String tok1 = store.generateToken(s1);
        boolean t1 = store.validateToken(s1, tok1);
        System.out.println(t1);

        // Test 2: Token should not validate under a different session
        String s2 = "sessB";
        boolean t2 = store.validateToken(s2, tok1);
        System.out.println(t2);

        // Test 3: Tampered token should fail
        String tok3 = store.generateToken(s1);
        char[] arr = tok3.toCharArray();
        arr[0] = (arr[0] == 'A') ? 'B' : 'A';
        String tampered = new String(arr);
        boolean t3 = store.validateToken(s1, tampered);
        System.out.println(t3);

        // Test 4: Expired token should fail
        String tok4 = store.generateToken("sessX");
        store.expireTokenForTest("sessX", tok4);
        boolean t4 = store.validateToken("sessX", tok4);
        System.out.println(t4);

        // Test 5: Replay should fail on second use
        String tok5 = store.generateToken("sessY");
        boolean firstUse = store.validateToken("sessY", tok5);
        boolean secondUse = store.validateToken("sessY", tok5);
        boolean t5 = firstUse && !secondUse;
        System.out.println(t5);
    }
}