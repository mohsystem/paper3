
import java.security.SecureRandom;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

public class Task135 {
    private static final int TOKEN_LENGTH = 32;
    private static final long TOKEN_EXPIRY_MS = TimeUnit.MINUTES.toMillis(30);
    private final Map<String, TokenData> tokenStore;
    private final SecureRandom secureRandom;

    static class TokenData {
        String token;
        long createdAt;
        String sessionId;

        TokenData(String token, long createdAt, String sessionId) {
            this.token = token;
            this.createdAt = createdAt;
            this.sessionId = sessionId;
        }

        boolean isExpired() {
            return System.currentTimeMillis() - createdAt > TOKEN_EXPIRY_MS;
        }
    }

    public Task135() {
        this.tokenStore = new ConcurrentHashMap<>();
        this.secureRandom = new SecureRandom();
    }

    public String generateToken(String sessionId) {
        byte[] randomBytes = new byte[TOKEN_LENGTH];
        secureRandom.nextBytes(randomBytes);
        String token = Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);
        
        TokenData tokenData = new TokenData(token, System.currentTimeMillis(), sessionId);
        tokenStore.put(sessionId, tokenData);
        
        return token;
    }

    public boolean validateToken(String sessionId, String token) {
        if (sessionId == null || token == null) {
            return false;
        }

        TokenData tokenData = tokenStore.get(sessionId);
        
        if (tokenData == null) {
            return false;
        }

        if (tokenData.isExpired()) {
            tokenStore.remove(sessionId);
            return false;
        }

        return tokenData.token.equals(token);
    }

    public void invalidateToken(String sessionId) {
        tokenStore.remove(sessionId);
    }

    public void cleanExpiredTokens() {
        tokenStore.entrySet().removeIf(entry -> entry.getValue().isExpired());
    }

    public int getActiveTokenCount() {
        cleanExpiredTokens();
        return tokenStore.size();
    }

    public static void main(String[] args) throws InterruptedException {
        Task135 csrfProtection = new Task135();

        // Test Case 1: Generate and validate token
        System.out.println("Test Case 1: Generate and validate token");
        String session1 = "session_123";
        String token1 = csrfProtection.generateToken(session1);
        System.out.println("Generated token: " + token1);
        System.out.println("Token valid: " + csrfProtection.validateToken(session1, token1));
        System.out.println();

        // Test Case 2: Invalid token validation
        System.out.println("Test Case 2: Invalid token validation");
        String session2 = "session_456";
        String token2 = csrfProtection.generateToken(session2);
        System.out.println("Valid token check: " + csrfProtection.validateToken(session2, token2));
        System.out.println("Invalid token check: " + csrfProtection.validateToken(session2, "invalid_token"));
        System.out.println();

        // Test Case 3: Token invalidation
        System.out.println("Test Case 3: Token invalidation");
        String session3 = "session_789";
        String token3 = csrfProtection.generateToken(session3);
        System.out.println("Before invalidation: " + csrfProtection.validateToken(session3, token3));
        csrfProtection.invalidateToken(session3);
        System.out.println("After invalidation: " + csrfProtection.validateToken(session3, token3));
        System.out.println();

        // Test Case 4: Multiple sessions
        System.out.println("Test Case 4: Multiple sessions");
        String sessionA = "session_A";
        String sessionB = "session_B";
        String tokenA = csrfProtection.generateToken(sessionA);
        String tokenB = csrfProtection.generateToken(sessionB);
        System.out.println("Session A valid: " + csrfProtection.validateToken(sessionA, tokenA));
        System.out.println("Session B valid: " + csrfProtection.validateToken(sessionB, tokenB));
        System.out.println("Cross-session invalid: " + csrfProtection.validateToken(sessionA, tokenB));
        System.out.println("Active tokens: " + csrfProtection.getActiveTokenCount());
        System.out.println();

        // Test Case 5: Null handling
        System.out.println("Test Case 5: Null handling");
        System.out.println("Null session: " + csrfProtection.validateToken(null, "token"));
        System.out.println("Null token: " + csrfProtection.validateToken("session", null));
        System.out.println("Both null: " + csrfProtection.validateToken(null, null));
    }
}
