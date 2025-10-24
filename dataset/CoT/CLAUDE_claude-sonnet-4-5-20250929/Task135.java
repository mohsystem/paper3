
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.time.Instant;

public class Task135 {
    private static final int TOKEN_LENGTH = 32;
    private static final long TOKEN_EXPIRY_SECONDS = 3600; // 1 hour
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Map<String, TokenData> tokenStore = new HashMap<>();
    
    static class TokenData {
        String token;
        long expiryTime;
        String sessionId;
        
        TokenData(String token, long expiryTime, String sessionId) {
            this.token = token;
            this.expiryTime = expiryTime;
            this.sessionId = sessionId;
        }
    }
    
    public static String generateCSRFToken(String sessionId) {
        if (sessionId == null || sessionId.trim().isEmpty()) {
            throw new IllegalArgumentException("Session ID cannot be null or empty");
        }
        
        byte[] tokenBytes = new byte[TOKEN_LENGTH];
        secureRandom.nextBytes(tokenBytes);
        String token = Base64.getUrlEncoder().withoutPadding().encodeToString(tokenBytes);
        
        long expiryTime = Instant.now().getEpochSecond() + TOKEN_EXPIRY_SECONDS;
        tokenStore.put(sessionId, new TokenData(token, expiryTime, sessionId));
        
        return token;
    }
    
    public static boolean validateCSRFToken(String sessionId, String token) {
        if (sessionId == null || sessionId.trim().isEmpty()) {
            return false;
        }
        
        if (token == null || token.trim().isEmpty()) {
            return false;
        }
        
        TokenData storedData = tokenStore.get(sessionId);
        if (storedData == null) {
            return false;
        }
        
        long currentTime = Instant.now().getEpochSecond();
        if (currentTime > storedData.expiryTime) {
            tokenStore.remove(sessionId);
            return false;
        }
        
        boolean isValid = constantTimeEquals(storedData.token, token);
        
        if (isValid) {
            tokenStore.remove(sessionId);
        }
        
        return isValid;
    }
    
    private static boolean constantTimeEquals(String a, String b) {
        if (a.length() != b.length()) {
            return false;
        }
        
        int result = 0;
        for (int i = 0; i < a.length(); i++) {
            result |= a.charAt(i) ^ b.charAt(i);
        }
        
        return result == 0;
    }
    
    public static void invalidateToken(String sessionId) {
        if (sessionId != null) {
            tokenStore.remove(sessionId);
        }
    }
    
    public static void main(String[] args) {
        System.out.println("CSRF Protection Test Cases:");
        System.out.println("===========================\\n");
        
        // Test Case 1: Valid token generation and validation
        System.out.println("Test Case 1: Valid token generation and validation");
        String session1 = "user_session_123";
        String token1 = generateCSRFToken(session1);
        System.out.println("Generated Token: " + token1);
        boolean result1 = validateCSRFToken(session1, token1);
        System.out.println("Validation Result: " + result1);
        System.out.println("Expected: true\\n");
        
        // Test Case 2: Invalid token validation
        System.out.println("Test Case 2: Invalid token validation");
        String session2 = "user_session_456";
        String token2 = generateCSRFToken(session2);
        boolean result2 = validateCSRFToken(session2, "invalid_token_xyz");
        System.out.println("Validation Result: " + result2);
        System.out.println("Expected: false\\n");
        
        // Test Case 3: Token reuse prevention
        System.out.println("Test Case 3: Token reuse prevention");
        String session3 = "user_session_789";
        String token3 = generateCSRFToken(session3);
        validateCSRFToken(session3, token3);
        boolean result3 = validateCSRFToken(session3, token3);
        System.out.println("Second Validation Result: " + result3);
        System.out.println("Expected: false (token already used)\\n");
        
        // Test Case 4: Session mismatch
        System.out.println("Test Case 4: Session mismatch");
        String session4a = "user_session_aaa";
        String session4b = "user_session_bbb";
        String token4 = generateCSRFToken(session4a);
        boolean result4 = validateCSRFToken(session4b, token4);
        System.out.println("Validation Result: " + result4);
        System.out.println("Expected: false\\n");
        
        // Test Case 5: Null/empty input validation
        System.out.println("Test Case 5: Null/empty input validation");
        boolean result5a = validateCSRFToken(null, "some_token");
        boolean result5b = validateCSRFToken("session", null);
        boolean result5c = validateCSRFToken("", "token");
        System.out.println("Null session validation: " + result5a);
        System.out.println("Null token validation: " + result5b);
        System.out.println("Empty session validation: " + result5c);
        System.out.println("Expected: false, false, false\\n");
    }
}
