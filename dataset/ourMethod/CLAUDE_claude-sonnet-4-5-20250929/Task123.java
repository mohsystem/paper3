
import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.nio.charset.StandardCharsets;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Base64;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Secure user session management system.
 * Prevents session fixation, hijacking, and timing attacks.
 */
public class Task123 {

    /**
     * Represents a user session with security attributes.
     */
    static class Session {
        private final String sessionId;
        private final String userId;
        private final long createdAt;
        private long lastActivity;
        private final String ipAddress;
        private final String userAgentHash;

        public Session(String sessionId, String userId, long createdAt, 
                      long lastActivity, String ipAddress, String userAgentHash) {
            // Input validation in constructor
            if (sessionId == null || sessionId.trim().isEmpty()) {
                throw new IllegalArgumentException("sessionId cannot be null or empty");
            }
            if (userId == null || userId.trim().isEmpty()) {
                throw new IllegalArgumentException("userId cannot be null or empty");
            }
            if (ipAddress == null || ipAddress.trim().isEmpty()) {
                throw new IllegalArgumentException("ipAddress cannot be null or empty");
            }
            if (userAgentHash == null || userAgentHash.trim().isEmpty()) {
                throw new IllegalArgumentException("userAgentHash cannot be null or empty");
            }
            
            this.sessionId = sessionId;
            this.userId = userId;
            this.createdAt = createdAt;
            this.lastActivity = lastActivity;
            this.ipAddress = ipAddress;
            this.userAgentHash = userAgentHash;
        }

        public String getSessionId() { return sessionId; }
        public String getUserId() { return userId; }
        public long getCreatedAt() { return createdAt; }
        public long getLastActivity() { return lastActivity; }
        public void setLastActivity(long lastActivity) { this.lastActivity = lastActivity; }
        public String getIpAddress() { return ipAddress; }
        public String getUserAgentHash() { return userAgentHash; }
    }

    /**
     * Secure session manager with protection against common attacks.
     */
    static class SessionManager {
        private final Map<String, Session> sessions;
        private final int sessionTimeout;  // in seconds
        private final int absoluteTimeout;  // in seconds
        private final byte[] hmacKey;
        private final SecureRandom secureRandom;

        /**
         * Initialize session manager with security parameters.
         * 
         * @param sessionTimeout Idle timeout in seconds
         * @param absoluteTimeout Maximum session lifetime in seconds
         */
        public SessionManager(int sessionTimeout, int absoluteTimeout) {
            // Input validation: ensure timeouts are positive
            if (sessionTimeout <= 0) {
                throw new IllegalArgumentException("sessionTimeout must be positive");
            }
            if (absoluteTimeout <= 0) {
                throw new IllegalArgumentException("absoluteTimeout must be positive");
            }

            this.sessions = new ConcurrentHashMap<>();
            this.sessionTimeout = sessionTimeout;
            this.absoluteTimeout = absoluteTimeout;
            
            // Use SecureRandom for cryptographically secure randomness
            this.secureRandom = new SecureRandom();
            
            // Generate secure HMAC key (32 bytes = 256 bits)
            this.hmacKey = new byte[32];
            this.secureRandom.nextBytes(this.hmacKey);
        }

        /**
         * Create a new session with cryptographically secure ID.
         * Prevents session fixation by generating unpredictable IDs.
         * 
         * @param userId User identifier
         * @param ipAddress Client IP address
         * @param userAgent Client user agent
         * @return Secure session ID
         */
        public String createSession(String userId, String ipAddress, String userAgent) {
            // Input validation: ensure all parameters are non-null and non-empty
            if (userId == null || userId.trim().isEmpty()) {
                throw new IllegalArgumentException("userId must be non-empty");
            }
            if (ipAddress == null || ipAddress.trim().isEmpty()) {
                throw new IllegalArgumentException("ipAddress must be non-empty");
            }
            if (userAgent == null || userAgent.trim().isEmpty()) {
                throw new IllegalArgumentException("userAgent must be non-empty");
            }
            
            // Validate userId length to prevent abuse
            if (userId.length() > 256) {
                throw new IllegalArgumentException("userId exceeds maximum length of 256");
            }

            // Generate cryptographically secure session ID using SecureRandom
            // 32 bytes = 256 bits of entropy, Base64 URL-safe encoding
            byte[] randomBytes = new byte[32];
            secureRandom.nextBytes(randomBytes);
            String sessionId = Base64.getUrlEncoder().withoutPadding()
                .encodeToString(randomBytes);

            long currentTime = System.currentTimeMillis() / 1000;

            // Hash user agent to avoid storing sensitive browser fingerprint
            String userAgentHash = hashSHA256(userAgent);

            // Create session object with security metadata
            Session session = new Session(
                sessionId,
                userId,
                currentTime,
                currentTime,
                ipAddress,
                userAgentHash
            );

            // Store session in concurrent map (thread-safe)
            sessions.put(sessionId, session);

            return sessionId;
        }

        /**
         * Validate session and check for hijacking attempts.
         * Uses constant-time comparison to prevent timing attacks.
         * 
         * @param sessionId Session ID to validate
         * @param ipAddress Current client IP
         * @param userAgent Current client user agent
         * @return userId if valid, null otherwise
         */
        public String validateSession(String sessionId, String ipAddress, String userAgent) {
            // Input validation: ensure parameters are non-null and non-empty
            if (sessionId == null || sessionId.isEmpty()) {
                return null;
            }
            if (ipAddress == null || ipAddress.isEmpty()) {
                return null;
            }
            if (userAgent == null || userAgent.isEmpty()) {
                return null;
            }

            // Validate sessionId length to prevent abuse
            if (sessionId.length() > 256) {
                return null;
            }

            // Retrieve session - fail closed if not found
            Session session = sessions.get(sessionId);
            if (session == null) {
                return null;
            }

            long currentTime = System.currentTimeMillis() / 1000;

            // Check absolute timeout (maximum session lifetime)
            if (currentTime - session.getCreatedAt() > absoluteTimeout) {
                destroySession(sessionId);
                return null;
            }

            // Check idle timeout
            if (currentTime - session.getLastActivity() > sessionTimeout) {
                destroySession(sessionId);
                return null;
            }

            // Verify IP address matches (prevents session hijacking)
            // Use constant-time comparison to prevent timing attacks
            if (!constantTimeEquals(session.getIpAddress(), ipAddress)) {
                // IP mismatch - potential hijacking attempt
                destroySession(sessionId);
                return null;
            }

            // Verify user agent hash matches
            String userAgentHash = hashSHA256(userAgent);

            // Use constant-time comparison for user agent hash
            if (!constantTimeEquals(session.getUserAgentHash(), userAgentHash)) {
                // User agent mismatch - potential hijacking attempt
                destroySession(sessionId);
                return null;
            }

            // Update last activity timestamp (session still valid)
            session.setLastActivity(currentTime);

            return session.getUserId();
        }

        /**
         * Securely destroy a session.
         * 
         * @param sessionId Session ID to destroy
         * @return true if destroyed, false if not found
         */
        public boolean destroySession(String sessionId) {
            // Input validation
            if (sessionId == null || sessionId.isEmpty()) {
                return false;
            }

            // Remove session from storage
            return sessions.remove(sessionId) != null;
        }

        /**
         * Remove all expired sessions from storage.
         * Should be called periodically to prevent memory leaks.
         * 
         * @return Number of sessions removed
         */
        public int cleanupExpiredSessions() {
            long currentTime = System.currentTimeMillis() / 1000;
            int removedCount = 0;

            // Iterate and remove expired sessions
            for (Map.Entry<String, Session> entry : sessions.entrySet()) {
                Session session = entry.getValue();
                
                // Check both absolute and idle timeouts
                if (currentTime - session.getCreatedAt() > absoluteTimeout ||
                    currentTime - session.getLastActivity() > sessionTimeout) {
                    sessions.remove(entry.getKey());
                    removedCount++;
                }
            }

            return removedCount;
        }

        /**
         * Hash a string using SHA-256.
         * 
         * @param input String to hash
         * @return Hex-encoded hash
         */
        private String hashSHA256(String input) {
            try {
                MessageDigest digest = MessageDigest.getInstance("SHA-256");
                byte[] hash = digest.digest(input.getBytes(StandardCharsets.UTF_8));
                return bytesToHex(hash);
            } catch (NoSuchAlgorithmException e) {
                throw new RuntimeException("SHA-256 algorithm not available", e);
            }
        }

        /**
         * Constant-time string comparison to prevent timing attacks.
         * Uses MessageDigest.isEqual which performs constant-time comparison.
         * 
         * @param a First string
         * @param b Second string
         * @return true if equal, false otherwise
         */
        private boolean constantTimeEquals(String a, String b) {
            if (a == null || b == null) {
                return false;
            }
            
            byte[] aBytes = a.getBytes(StandardCharsets.UTF_8);
            byte[] bBytes = b.getBytes(StandardCharsets.UTF_8);
            
            // MessageDigest.isEqual performs constant-time comparison
            return MessageDigest.isEqual(aBytes, bBytes);
        }

        /**
         * Convert byte array to hex string.
         * 
         * @param bytes Byte array
         * @return Hex string
         */
        private String bytesToHex(byte[] bytes) {
            StringBuilder result = new StringBuilder();
            for (byte b : bytes) {
                result.append(String.format("%02x", b));
            }
            return result.toString();
        }

        /**
         * Get session count (for testing/monitoring).
         * 
         * @return Number of active sessions
         */
        public int getSessionCount() {
            return sessions.size();
        }
    }

    /**
     * Test cases demonstrating secure session management.
     */
    public static void main(String[] args) {
        System.out.println("=== Secure Session Manager Test Cases ===\\n");

        // Initialize session manager with 5-minute idle timeout, 1-hour absolute timeout
        SessionManager manager = new SessionManager(300, 3600);

        // Test Case 1: Create and validate a valid session
        System.out.println("Test 1: Create and validate valid session");
        String sessionId = manager.createSession(
            "user123",
            "192.168.1.100",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"
        );
        System.out.println("  Created session: " + sessionId.substring(0, 16) + "...");

        String userId = manager.validateSession(
            sessionId,
            "192.168.1.100",
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"
        );
        System.out.println("  Validated user: " + userId);
        assert "user123".equals(userId) : "Session validation failed";
        System.out.println("  ✓ Pass\\n");

        // Test Case 2: Reject session with mismatched IP (hijacking prevention)
        System.out.println("Test 2: Reject session with different IP address");
        userId = manager.validateSession(
            sessionId,
            "192.168.1.200",  // Different IP
            "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"
        );
        System.out.println("  Validation result: " + userId);
        assert userId == null : "Should reject mismatched IP";
        System.out.println("  ✓ Pass (correctly rejected)\\n");

        // Test Case 3: Reject session with mismatched user agent
        System.out.println("Test 3: Reject session with different user agent");
        String sessionId2 = manager.createSession(
            "user456",
            "10.0.0.50",
            "Chrome/120.0.0.0"
        );
        userId = manager.validateSession(
            sessionId2,
            "10.0.0.50",
            "Firefox/121.0"  // Different user agent
        );
        System.out.println("  Validation result: " + userId);
        assert userId == null : "Should reject mismatched user agent";
        System.out.println("  ✓ Pass (correctly rejected)\\n");

        // Test Case 4: Destroy session and verify it's invalid
        System.out.println("Test 4: Destroy session");
        String sessionId3 = manager.createSession(
            "user789",
            "172.16.0.10",
            "Safari/17.0"
        );
        boolean destroyed = manager.destroySession(sessionId3);
        System.out.println("  Session destroyed: " + destroyed);

        userId = manager.validateSession(
            sessionId3,
            "172.16.0.10",
            "Safari/17.0"
        );
        System.out.println("  Validation after destroy: " + userId);
        assert userId == null : "Destroyed session should be invalid";
        System.out.println("  ✓ Pass\\n");

        // Test Case 5: Input validation
        System.out.println("Test 5: Input validation for empty/invalid inputs");
        try {
            manager.createSession("", "192.168.1.1", "Browser");
            System.out.println("  ✗ Failed to reject empty user_id");
        } catch (IllegalArgumentException e) {
            System.out.println("  ✓ Correctly rejected empty user_id");
        }

        String result = manager.validateSession("", "192.168.1.1", "Browser");
        assert result == null : "Should reject empty session_id";
        System.out.println("  ✓ Correctly rejected empty session_id");
        System.out.println("  ✓ Pass\\n");

        System.out.println("=== All test cases passed ===");
    }
}
