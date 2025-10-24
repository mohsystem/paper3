import java.security.SecureRandom;
import java.time.Duration;
import java.time.Instant;
import java.util.Optional;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

public class Task123 {

    private static class SessionData {
        private final String userId;
        private final Instant expiration;

        SessionData(String userId, Instant expiration) {
            this.userId = userId;
            this.expiration = expiration;
        }

        public String getUserId() {
            return userId;
        }

        public boolean isExpired() {
            return Instant.now().isAfter(expiration);
        }
    }

    private static final int SESSION_ID_BYTES = 32;
    private static final Duration SESSION_TIMEOUT = Duration.ofMinutes(30);
    private final ConcurrentHashMap<String, SessionData> sessions = new ConcurrentHashMap<>();
    private final SecureRandom secureRandom = new SecureRandom();

    private String generateSessionId() {
        byte[] token = new byte[SESSION_ID_BYTES];
        secureRandom.nextBytes(token);
        return bytesToHex(token);
    }

    private static String bytesToHex(byte[] bytes) {
        StringBuilder hexString = new StringBuilder(2 * bytes.length);
        for (byte b : bytes) {
            String hex = Integer.toHexString(0xff & b);
            if (hex.length() == 1) {
                hexString.append('0');
            }
            hexString.append(hex);
        }
        return hexString.toString();
    }

    public String createSession(String userId) {
        if (userId == null || userId.isEmpty()) {
            throw new IllegalArgumentException("User ID cannot be null or empty.");
        }
        String sessionId = generateSessionId();
        Instant expiration = Instant.now().plus(SESSION_TIMEOUT);
        sessions.put(sessionId, new SessionData(userId, expiration));
        return sessionId;
    }

    public Optional<String> getSessionUser(String sessionId) {
        if (sessionId == null || sessionId.isEmpty()) {
            return Optional.empty();
        }

        SessionData session = sessions.get(sessionId);
        if (session == null) {
            return Optional.empty();
        }

        if (session.isExpired()) {
            // Lazily remove expired session
            sessions.remove(sessionId, session);
            return Optional.empty();
        }
        
        return Optional.of(session.getUserId());
    }

    public void invalidateSession(String sessionId) {
        if (sessionId != null && !sessionId.isEmpty()) {
            sessions.remove(sessionId);
        }
    }

    public static void main(String[] args) throws InterruptedException {
        Task123 sessionManager = new Task123();

        System.out.println("--- Test Case 1: Create and validate a session ---");
        String userId1 = "user-123";
        String sessionId1 = sessionManager.createSession(userId1);
        System.out.println("Created session for " + userId1);
        Optional<String> retrievedUser1 = sessionManager.getSessionUser(sessionId1);
        retrievedUser1.ifPresent(u -> System.out.println("Validated session, user is: " + u));
        System.out.println("Test Case 1 Passed: " + userId1.equals(retrievedUser1.orElse(null)));
        System.out.println();

        System.out.println("--- Test Case 2: Invalidate a session ---");
        sessionManager.invalidateSession(sessionId1);
        Optional<String> retrievedUser2 = sessionManager.getSessionUser(sessionId1);
        System.out.println("After invalidation, user is: " + retrievedUser2.orElse("Not Found"));
        System.out.println("Test Case 2 Passed: " + retrievedUser2.isEmpty());
        System.out.println();
        
        System.out.println("--- Test Case 3: Validate a non-existent session ---");
        Optional<String> retrievedUser3 = sessionManager.getSessionUser("non-existent-session-id-that-is-64-chars-long-so-it-is-valid-format");
        System.out.println("Validating non-existent session, user is: " + retrievedUser3.orElse("Not Found"));
        System.out.println("Test Case 3 Passed: " + retrievedUser3.isEmpty());
        System.out.println();

        System.out.println("--- Test Case 4: Session expiration ---");
        Task123 shortLivedManager = new Task123() {
             private final Duration shortTimeout = Duration.ofMillis(100);
             @Override
             public String createSession(String userId) {
                 if (userId == null || userId.isEmpty()) {
                     throw new IllegalArgumentException("User ID cannot be null or empty.");
                 }
                 String sessionId = generateSessionId();
                 Instant expiration = Instant.now().plus(shortTimeout);
                 sessions.put(sessionId, new SessionData(userId, expiration));
                 return sessionId;
             }
        };
        String userId4 = "user-456";
        String sessionId4 = shortLivedManager.createSession(userId4);
        System.out.println("Created short-lived session for " + userId4);
        System.out.println("Waiting for session to expire...");
        TimeUnit.MILLISECONDS.sleep(150);
        Optional<String> retrievedUser4 = shortLivedManager.getSessionUser(sessionId4);
        System.out.println("After expiration, user is: " + retrievedUser4.orElse("Not Found"));
        System.out.println("Test Case 4 Passed: " + retrievedUser4.isEmpty());
        System.out.println();
        
        System.out.println("--- Test Case 5: Handle null/empty inputs ---");
        boolean test5Passed = true;
        try {
            sessionManager.createSession(null);
            test5Passed = false; // Should have thrown
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception for createSession(null)");
        }
        Optional<String> retrievedUser5 = sessionManager.getSessionUser(null);
        System.out.println("getSessionUser(null) returned empty: " + retrievedUser5.isEmpty());
        if (!retrievedUser5.isEmpty()) test5Passed = false;
        System.out.println("Test Case 5 Passed: " + test5Passed);
        System.out.println();
    }
}