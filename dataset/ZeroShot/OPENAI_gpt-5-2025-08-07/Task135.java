import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Iterator;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Task135 {
    public static class CsrfTokenService {
        private static final int TOKEN_BYTES = 32;
        private final SecureRandom secureRandom = new SecureRandom();
        private final Map<String, TokenRecord> store = new ConcurrentHashMap<>();
        private final long ttlMillis;

        private static class TokenRecord {
            String token;
            long expiry;
            TokenRecord(String token, long expiry) {
                this.token = token;
                this.expiry = expiry;
            }
        }

        public CsrfTokenService(long ttlMillis) {
            if (ttlMillis <= 0) throw new IllegalArgumentException("ttlMillis must be positive");
            this.ttlMillis = ttlMillis;
        }

        public String issueToken(String sessionId) {
            if (sessionId == null) throw new IllegalArgumentException("sessionId required");
            String token = generateToken(TOKEN_BYTES);
            long expiry = System.currentTimeMillis() + ttlMillis;
            store.put(sessionId, new TokenRecord(token, expiry));
            return token;
        }

        public boolean validateToken(String sessionId, String providedToken) {
            if (sessionId == null || providedToken == null) return false;
            TokenRecord rec = store.get(sessionId);
            if (rec == null) return false;
            long now = System.currentTimeMillis();
            if (now > rec.expiry) {
                store.remove(sessionId);
                return false;
            }
            boolean ok = constantTimeEquals(rec.token, providedToken);
            if (ok) {
                // Rotate token on success to prevent replay
                String newToken = generateToken(TOKEN_BYTES);
                rec.token = newToken;
                rec.expiry = now + ttlMillis;
                store.put(sessionId, rec);
            }
            return ok;
        }

        public int purgeExpired() {
            int removed = 0;
            long now = System.currentTimeMillis();
            Iterator<Map.Entry<String, TokenRecord>> it = store.entrySet().iterator();
            while (it.hasNext()) {
                Map.Entry<String, TokenRecord> e = it.next();
                if (now > e.getValue().expiry) {
                    it.remove();
                    removed++;
                }
            }
            return removed;
        }

        private String generateToken(int numBytes) {
            byte[] bytes = new byte[numBytes];
            secureRandom.nextBytes(bytes);
            return Base64.getUrlEncoder().withoutPadding().encodeToString(bytes);
        }

        private boolean constantTimeEquals(String a, String b) {
            byte[] aBytes = a.getBytes(java.nio.charset.StandardCharsets.UTF_8);
            byte[] bBytes = b.getBytes(java.nio.charset.StandardCharsets.UTF_8);
            return MessageDigest.isEqual(aBytes, bBytes);
        }
    }

    public static void main(String[] args) throws Exception {
        CsrfTokenService service = new CsrfTokenService(200); // 200 ms TTL for demonstration

        // Test 1: Valid token for session
        String s1 = "sessionA";
        String t1 = service.issueToken(s1);
        boolean r1 = service.validateToken(s1, t1);

        // Test 2: Invalid token
        boolean r2 = service.validateToken(s1, "not-a-valid-token");

        // Test 3: Expired token
        CsrfTokenService shortTTL = new CsrfTokenService(10);
        String s2 = "sessionB";
        String t2 = shortTTL.issueToken(s2);
        Thread.sleep(30);
        boolean r3 = shortTTL.validateToken(s2, t2);

        // Test 4: Token bound to different session
        String s3 = "sessionC";
        String t3 = service.issueToken(s3);
        boolean r4 = service.validateToken("otherSession", t3);

        // Test 5: Replay attempt fails after rotation
        String s4 = "sessionD";
        String t4 = service.issueToken(s4);
        boolean firstUse = service.validateToken(s4, t4);
        boolean secondUse = service.validateToken(s4, t4);
        boolean r5 = firstUse && !secondUse;

        System.out.println(r1);
        System.out.println(r2);
        System.out.println(r3);
        System.out.println(r4);
        System.out.println(r5);
    }
}