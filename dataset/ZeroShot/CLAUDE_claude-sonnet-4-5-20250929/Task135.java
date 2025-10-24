
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Task135 {
    private static final int TOKEN_LENGTH = 32;
    private static final long TOKEN_EXPIRY_TIME = 30 * 60 * 1000; // 30 minutes
    
    private Map<String, TokenData> tokenStore;
    private SecureRandom secureRandom;
    
    public Task135() {
        this.tokenStore = new ConcurrentHashMap<>();
        this.secureRandom = new SecureRandom();
    }
    
    // Generate a new CSRF token for a session
    public String generateToken(String sessionId) {
        byte[] tokenBytes = new byte[TOKEN_LENGTH];
        secureRandom.nextBytes(tokenBytes);
        String token = Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
        
        TokenData tokenData = new TokenData(token, System.currentTimeMillis());
        tokenStore.put(sessionId, tokenData);
        
        return token;
    }
    
    // Validate CSRF token
    public boolean validateToken(String sessionId, String token) {
        if (sessionId == null || token == null) {
            return false;
        }
        
        TokenData tokenData = tokenStore.get(sessionId);
        
        if (tokenData == null) {
            return false;
        }
        
        // Check if token has expired
        if (System.currentTimeMillis() - tokenData.timestamp > TOKEN_EXPIRY_TIME) {
            tokenStore.remove(sessionId);
            return false;
        }
        
        // Constant-time comparison to prevent timing attacks
        return constantTimeEquals(tokenData.token, token);
    }
    
    // Invalidate token after use (for single-use tokens)
    public void invalidateToken(String sessionId) {
        tokenStore.remove(sessionId);
    }
    
    // Constant-time string comparison
    private boolean constantTimeEquals(String a, String b) {
        if (a.length() != b.length()) {
            return false;
        }
        
        int result = 0;
        for (int i = 0; i < a.length(); i++) {
            result |= a.charAt(i) ^ b.charAt(i);
        }
        
        return result == 0;
    }
    
    // Clean up expired tokens
    public void cleanupExpiredTokens() {
        long currentTime = System.currentTimeMillis();
        tokenStore.entrySet().removeIf(entry -> 
            currentTime - entry.getValue().timestamp > TOKEN_EXPIRY_TIME
        );
    }
    
    // Inner class to store token data
    private static class TokenData {
        String token;
        long timestamp;
        
        TokenData(String token, long timestamp) {
            this.token = token;
            this.timestamp = timestamp;
        }
    }
    
    public static void main(String[] args) {
        Task135 csrfProtection = new Task135();
        
        System.out.println("=== CSRF Protection Test Cases ===\\n");
        
        // Test Case 1: Generate and validate valid token
        System.out.println("Test Case 1: Generate and validate valid token");
        String session1 = "session123";
        String token1 = csrfProtection.generateToken(session1);
        System.out.println("Generated Token: " + token1);
        System.out.println("Validation Result: " + csrfProtection.validateToken(session1, token1));
        System.out.println();
        
        // Test Case 2: Validate with wrong token
        System.out.println("Test Case 2: Validate with wrong token");
        String session2 = "session456";
        String token2 = csrfProtection.generateToken(session2);
        String wrongToken = "wrongToken123";
        System.out.println("Generated Token: " + token2);
        System.out.println("Wrong Token: " + wrongToken);
        System.out.println("Validation Result: " + csrfProtection.validateToken(session2, wrongToken));
        System.out.println();
        
        // Test Case 3: Validate with non-existent session
        System.out.println("Test Case 3: Validate with non-existent session");
        String nonExistentSession = "nonExistent";
        String token3 = "someToken";
        System.out.println("Validation Result: " + csrfProtection.validateToken(nonExistentSession, token3));
        System.out.println();
        
        // Test Case 4: Invalidate token and try to use it
        System.out.println("Test Case 4: Invalidate token and try to use it");
        String session4 = "session789";
        String token4 = csrfProtection.generateToken(session4);
        System.out.println("Generated Token: " + token4);
        System.out.println("Validation Before Invalidation: " + csrfProtection.validateToken(session4, token4));
        csrfProtection.invalidateToken(session4);
        System.out.println("Validation After Invalidation: " + csrfProtection.validateToken(session4, token4));
        System.out.println();
        
        // Test Case 5: Multiple sessions with different tokens
        System.out.println("Test Case 5: Multiple sessions with different tokens");
        String sessionA = "sessionA";
        String sessionB = "sessionB";
        String tokenA = csrfProtection.generateToken(sessionA);
        String tokenB = csrfProtection.generateToken(sessionB);
        System.out.println("Session A Token: " + tokenA);
        System.out.println("Session B Token: " + tokenB);
        System.out.println("Validate A with A's token: " + csrfProtection.validateToken(sessionA, tokenA));
        System.out.println("Validate B with B's token: " + csrfProtection.validateToken(sessionB, tokenB));
        System.out.println("Validate A with B's token: " + csrfProtection.validateToken(sessionA, tokenB));
        System.out.println("Validate B with A's token: " + csrfProtection.validateToken(sessionB, tokenA));
    }
}
