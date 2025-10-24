import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Base64;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class Task135 {

    // CSRF Protector implementing HMAC token with time and nonce + origin/referrer checks
    public static class CsrfProtector {
        private final int ttlSeconds;
        private final String expectedOrigin;
        private final SecureRandom secureRandom = new SecureRandom();
        private final byte[] secretKey;
        private final Base64.Encoder b64UrlNoPad = Base64.getUrlEncoder().withoutPadding();
        private final Base64.Decoder b64Url = Base64.getUrlDecoder();

        // sessionId -> (nonce -> issuedAtEpochSeconds)
        private final Map<String, ConcurrentHashMap<String, Long>> usedNoncesPerSession = new ConcurrentHashMap<>();

        public CsrfProtector(int ttlSeconds, String expectedOrigin) {
            this.ttlSeconds = ttlSeconds;
            this.expectedOrigin = expectedOrigin;
            this.secretKey = new byte[32];
            secureRandom.nextBytes(this.secretKey);
        }

        // Generate token for a given sessionId (ts.nonce.sig)
        public String generateToken(String sessionId) {
            long now = Instant.now().getEpochSecond();
            return generateTokenAt(sessionId, now);
        }

        // Generate token at a specific timestamp (for testing)
        public String generateTokenAt(String sessionId, long issuedAtEpochSeconds) {
            byte[] nonceBytes = new byte[16];
            secureRandom.nextBytes(nonceBytes);
            String nonce = b64UrlNoPad.encodeToString(nonceBytes);
            String payload = sessionId + ":" + issuedAtEpochSeconds + ":" + nonce;
            String sig = b64UrlNoPad.encodeToString(hmacSha256(secretKey, payload.getBytes(StandardCharsets.UTF_8)));
            return issuedAtEpochSeconds + "." + nonce + "." + sig;
        }

        // Validate state-changing request; returns true if allowed
        public boolean validateRequest(String method, String sessionId, String token, String originHeader, String refererHeader) {
            if (!isStateChangingMethod(method)) {
                return true; // CSRF not required for safe methods
            }
            if (!validateOriginReferer(originHeader, refererHeader)) {
                return false;
            }
            if (token == null || token.isEmpty() || sessionId == null || sessionId.isEmpty()) {
                return false;
            }
            String[] parts = token.split("\\.");
            if (parts.length != 3) {
                return false;
            }
            long ts;
            try {
                ts = Long.parseLong(parts[0]);
            } catch (NumberFormatException e) {
                return false;
            }
            String nonce = parts[1];
            String sigB64 = parts[2];
            long now = Instant.now().getEpochSecond();
            if (ts > now || (now - ts) > ttlSeconds) {
                return false; // expired or from the future
            }

            String payload = sessionId + ":" + ts + ":" + nonce;
            byte[] expectedSig = hmacSha256(secretKey, payload.getBytes(StandardCharsets.UTF_8));
            byte[] providedSig;
            try {
                providedSig = b64Url.decode(sigB64);
            } catch (IllegalArgumentException e) {
                return false;
            }
            if (!constantTimeEquals(expectedSig, providedSig)) {
                return false;
            }

            // Replay protection: ensure nonce unused per session
            ConcurrentHashMap<String, Long> sessionNonces =
                usedNoncesPerSession.computeIfAbsent(sessionId, k -> new ConcurrentHashMap<>());
            Long existing = sessionNonces.putIfAbsent(nonce, ts);
            if (existing != null) {
                return false; // replay
            }
            // Optional cleanup of old nonces
            sessionNonces.entrySet().removeIf(e -> (now - e.getValue()) > ttlSeconds * 2L);
            return true;
        }

        private boolean isStateChangingMethod(String method) {
            if (method == null) return false;
            String m = method.toUpperCase();
            return m.equals("POST") || m.equals("PUT") || m.equals("PATCH") || m.equals("DELETE");
        }

        private boolean validateOriginReferer(String originHeader, String refererHeader) {
            if (expectedOrigin == null || expectedOrigin.isEmpty()) {
                return true; // if no expected origin configured, skip (not recommended)
            }
            if (originHeader != null && !originHeader.isEmpty()) {
                return expectedOrigin.equals(originHeader);
            }
            if (refererHeader != null && !refererHeader.isEmpty()) {
                return refererHeader.startsWith(expectedOrigin);
            }
            return false; // no verifiable origin info
        }

        private static byte[] hmacSha256(byte[] key, byte[] data) {
            try {
                Mac mac = Mac.getInstance("HmacSHA256");
                mac.init(new SecretKeySpec(key, "HmacSHA256"));
                return mac.doFinal(data);
            } catch (Exception e) {
                // Should not happen with standard algorithm
                return new byte[0];
            }
        }

        private static boolean constantTimeEquals(byte[] a, byte[] b) {
            if (a == null || b == null) return false;
            if (a.length != b.length) return false;
            int result = 0;
            for (int i = 0; i < a.length; i++) {
                result |= (a[i] ^ b[i]);
            }
            return result == 0;
        }
    }

    // Main with 5 test cases
    public static void main(String[] args) {
        CsrfProtector protector = new CsrfProtector(5, "https://example.com"); // short TTL for demo
        String sessionId = "session-123";

        String validToken = protector.generateToken(sessionId);

        boolean t1 = protector.validateRequest("POST", sessionId, validToken, "https://example.com", null);

        boolean t2 = protector.validateRequest("POST", sessionId, validToken, "https://evil.com", null);

        String tampered = validToken.substring(0, validToken.length() - 1) + (validToken.endsWith("A") ? "B" : "A");
        boolean t3 = protector.validateRequest("POST", sessionId, tampered, "https://example.com", null);

        // Expired token: generate with timestamp far in the past
        long past = Instant.now().getEpochSecond() - 3600;
        String expired = protector.generateTokenAt(sessionId, past);
        boolean t4 = protector.validateRequest("POST", sessionId, expired, "https://example.com", null);

        // Replay: reuse the valid token after it was already used in t1
        boolean t5 = protector.validateRequest("POST", sessionId, validToken, "https://example.com", null);

        System.out.println("T1_valid=" + t1);
        System.out.println("T2_bad_origin=" + t2);
        System.out.println("T3_tampered=" + t3);
        System.out.println("T4_expired=" + t4);
        System.out.println("T5_replay=" + t5);
    }
}