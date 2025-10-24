
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class Task135 {
    private static final int TOKEN_LENGTH = 32;
    private static final long TOKEN_EXPIRY_MS = 3600000; // 1 hour
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Map<String, TokenData> tokenStore = new ConcurrentHashMap<>();
    private static final byte[] hmacKey = generateSecureKey();

    private static class TokenData {
        final String sessionId;
        final long expiryTime;

        TokenData(String sessionId, long expiryTime) {
            this.sessionId = sessionId;
            this.expiryTime = expiryTime;
        }
    }

    private static byte[] generateSecureKey() {
        byte[] key = new byte[32];
        secureRandom.nextBytes(key);
        return key;
    }

    public static String generateCSRFToken(String sessionId) {
        if (sessionId == null || sessionId.trim().isEmpty()) {
            throw new IllegalArgumentException("Session ID cannot be null or empty");
        }

        byte[] randomBytes = new byte[TOKEN_LENGTH];
        secureRandom.nextBytes(randomBytes);
        String token = Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);

        long expiryTime = System.currentTimeMillis() + TOKEN_EXPIRY_MS;
        tokenStore.put(token, new TokenData(sessionId, expiryTime));

        cleanExpiredTokens();
        return token;
    }

    public static boolean validateCSRFToken(String token, String sessionId) {
        if (token == null || token.trim().isEmpty()) {
            return false;
        }
        if (sessionId == null || sessionId.trim().isEmpty()) {
            return false;
        }

        TokenData data = tokenStore.get(token);
        if (data == null) {
            return false;
        }

        if (System.currentTimeMillis() > data.expiryTime) {
            tokenStore.remove(token);
            return false;
        }

        boolean isValid = constantTimeEquals(data.sessionId, sessionId);
        if (isValid) {
            tokenStore.remove(token);
        }

        return isValid;
    }

    public static String generateHMACToken(String sessionId, String action) {
        if (sessionId == null || sessionId.trim().isEmpty()) {
            throw new IllegalArgumentException("Session ID cannot be null or empty");
        }
        if (action == null || action.trim().isEmpty()) {
            throw new IllegalArgumentException("Action cannot be null or empty");
        }

        try {
            long timestamp = System.currentTimeMillis();
            String message = sessionId + ":" + action + ":" + timestamp;

            Mac hmac = Mac.getInstance("HmacSHA256");
            SecretKeySpec secretKey = new SecretKeySpec(hmacKey, "HmacSHA256");
            hmac.init(secretKey);

            byte[] hash = hmac.doFinal(message.getBytes(StandardCharsets.UTF_8));
            String signature = Base64.getUrlEncoder().withoutPadding().encodeToString(hash);

            return signature + ":" + timestamp;
        } catch (NoSuchAlgorithmException | InvalidKeyException e) {
            throw new RuntimeException("HMAC generation failed", e);
        }
    }

    public static boolean validateHMACToken(String token, String sessionId, String action) {
        if (token == null || token.trim().isEmpty()) {
            return false;
        }
        if (sessionId == null || sessionId.trim().isEmpty()) {
            return false;
        }
        if (action == null || action.trim().isEmpty()) {
            return false;
        }

        String[] parts = token.split(":");
        if (parts.length != 2) {
            return false;
        }

        try {
            String providedSignature = parts[0];
            long timestamp = Long.parseLong(parts[1]);

            if (System.currentTimeMillis() - timestamp > TOKEN_EXPIRY_MS) {
                return false;
            }

            String message = sessionId + ":" + action + ":" + timestamp;
            Mac hmac = Mac.getInstance("HmacSHA256");
            SecretKeySpec secretKey = new SecretKeySpec(hmacKey, "HmacSHA256");
            hmac.init(secretKey);

            byte[] hash = hmac.doFinal(message.getBytes(StandardCharsets.UTF_8));
            String expectedSignature = Base64.getUrlEncoder().withoutPadding().encodeToString(hash);

            return constantTimeEquals(providedSignature, expectedSignature);
        } catch (NumberFormatException | NoSuchAlgorithmException | InvalidKeyException e) {
            return false;
        }
    }

    private static boolean constantTimeEquals(String a, String b) {
        if (a == null || b == null) {
            return false;
        }
        return MessageDigest.isEqual(
            a.getBytes(StandardCharsets.UTF_8),
            b.getBytes(StandardCharsets.UTF_8)
        );
    }

    private static void cleanExpiredTokens() {
        long currentTime = System.currentTimeMillis();
        tokenStore.entrySet().removeIf(entry -> currentTime > entry.getValue().expiryTime);
    }

    public static void main(String[] args) {
        System.out.println("CSRF Protection Implementation Test Cases\\n");

        // Test Case 1: Generate and validate token
        System.out.println("Test 1: Generate and validate CSRF token");
        String sessionId1 = "session123";
        String token1 = generateCSRFToken(sessionId1);
        System.out.println("Generated token: " + token1);
        boolean valid1 = validateCSRFToken(token1, sessionId1);
        System.out.println("Validation result: " + valid1);
        System.out.println();

        // Test Case 2: Invalid session ID
        System.out.println("Test 2: Validate with wrong session ID");
        String sessionId2 = "session456";
        String token2 = generateCSRFToken(sessionId2);
        boolean valid2 = validateCSRFToken(token2, "wrongSession");
        System.out.println("Validation result (should be false): " + valid2);
        System.out.println();

        // Test Case 3: Token reuse protection
        System.out.println("Test 3: Token reuse protection");
        String sessionId3 = "session789";
        String token3 = generateCSRFToken(sessionId3);
        boolean firstUse = validateCSRFToken(token3, sessionId3);
        boolean secondUse = validateCSRFToken(token3, sessionId3);
        System.out.println("First use: " + firstUse);
        System.out.println("Second use (should be false): " + secondUse);
        System.out.println();

        // Test Case 4: HMAC-based token generation and validation
        System.out.println("Test 4: HMAC-based CSRF token");
        String sessionId4 = "sessionABC";
        String action4 = "deleteAccount";
        String hmacToken = generateHMACToken(sessionId4, action4);
        System.out.println("Generated HMAC token: " + hmacToken);
        boolean validHMAC = validateHMACToken(hmacToken, sessionId4, action4);
        System.out.println("HMAC validation result: " + validHMAC);
        System.out.println();

        // Test Case 5: HMAC token with wrong action
        System.out.println("Test 5: HMAC token with wrong action");
        String sessionId5 = "sessionXYZ";
        String action5 = "updateProfile";
        String hmacToken5 = generateHMACToken(sessionId5, action5);
        boolean validHMAC5 = validateHMACToken(hmacToken5, sessionId5, "wrongAction");
        System.out.println("HMAC validation result (should be false): " + validHMAC5);
    }
}
