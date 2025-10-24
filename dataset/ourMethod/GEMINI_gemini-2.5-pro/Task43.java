import java.security.SecureRandom;
import java.time.Duration;
import java.time.Instant;
import java.util.Base64;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

public class Task43 {

    /**
     * Represents a user session.
     * This is an immutable data class.
     */
    private static final class Session {
        private final String userId;
        private final Instant creationTime;
        private final Instant expirationTime;

        public Session(String userId, Instant creationTime, Instant expirationTime) {
            this.userId = userId;
            this.creationTime = creationTime;
            this.expirationTime = expirationTime;
        }

        public String getUserId() {
            return userId;
        }

        public Instant getExpirationTime() {
            return expirationTime;
        }

        public boolean isExpired() {
            return Instant.now().isAfter(expirationTime);
        }
    }

    /**
     * Manages user sessions securely in memory.
     */
    public static class SessionManager {
        private static final int SESSION_ID_BYTES = 32;
        private static final Duration SESSION_LIFETIME = Duration.ofMinutes(30);
        private static final SecureRandom secureRandom = new SecureRandom();
        private static final Base64.Encoder urlEncoder = Base64.getUrlEncoder().withoutPadding();

        private final Map<String, Session> sessions = new ConcurrentHashMap<>();

        /**
         * Generates a cryptographically secure, URL-safe session ID.
         *
         * @return A new session ID string.
         */
        private String generateSessionId() {
            byte[] randomBytes = new byte[SESSION_ID_BYTES];
            secureRandom.nextBytes(randomBytes);
            return urlEncoder.encodeToString(randomBytes);
        }

        /**
         * Creates a new session for a given user.
         *
         * @param userId The identifier for the user.
         * @return The newly created session ID.
         */
        public String createSession(String userId) {
            if (userId == null || userId.isEmpty()) {
                throw new IllegalArgumentException("User ID cannot be null or empty.");
            }
            String sessionId = generateSessionId();
            Instant now = Instant.now();
            Instant expiration = now.plus(SESSION_LIFETIME);
            Session session = new Session(userId, now, expiration);
            sessions.put(sessionId, session);
            return sessionId;
        }

        /**
         * Overloaded version of createSession for testing with custom lifetime.
         */
        public String createSession(String userId, Duration lifetime) {
            if (userId == null || userId.isEmpty()) {
                throw new IllegalArgumentException("User ID cannot be null or empty.");
            }
            String sessionId = generateSessionId();
            Instant now = Instant.now();
            Instant expiration = now.plus(lifetime);
            Session session = new Session(userId, now, expiration);
            sessions.put(sessionId, session);
            return sessionId;
        }

        /**
         * Retrieves the user ID associated with a session ID, if the session is valid and not expired.
         *
         * @param sessionId The session ID to validate.
         * @return An Optional containing the user ID if the session is valid, otherwise an empty Optional.
         */
        public Optional<String> getUserIdFromSession(String sessionId) {
            if (sessionId == null) {
                return Optional.empty();
            }
            Session session = sessions.get(sessionId);
            if (session == null) {
                return Optional.empty();
            }

            if (session.isExpired()) {
                // Eagerly remove expired session
                sessions.remove(sessionId, session);
                return Optional.empty();
            }

            return Optional.of(session.getUserId());
        }

        /**
         * Terminates a session, effectively logging the user out.
         *
         * @param sessionId The session ID to terminate.
         */
        public void terminateSession(String sessionId) {
            if (sessionId != null) {
                sessions.remove(sessionId);
            }
        }
    }

    public static void main(String[] args) {
        SessionManager sessionManager = new SessionManager();

        System.out.println("--- Test Case 1: Session Creation and Validation ---");
        String sessionId1 = sessionManager.createSession("user123");
        System.out.println("Created session for user123: " + sessionId1);
        sessionManager.getUserIdFromSession(sessionId1)
                .ifPresent(userId -> System.out.println("Session is valid. User ID: " + userId));

        System.out.println("\n--- Test Case 2: Validation of Non-Existent Session ---");
        String fakeSessionId = "nonexistent-session-id";
        Optional<String> result2 = sessionManager.getUserIdFromSession(fakeSessionId);
        System.out.println("Validation for fake session '" + fakeSessionId + "': " +
                (result2.isPresent() ? "Valid" : "Invalid"));


        System.out.println("\n--- Test Case 3: Session Termination ---");
        String sessionId3 = sessionManager.createSession("user456");
        System.out.println("Created session for user456: " + sessionId3);
        sessionManager.terminateSession(sessionId3);
        System.out.println("Terminated session for user456.");
        Optional<String> result3 = sessionManager.getUserIdFromSession(sessionId3);
        System.out.println("Validation after termination: " + (result3.isPresent() ? "Valid" : "Invalid"));

        System.out.println("\n--- Test Case 4: Session Expiration ---");
        String sessionId4 = sessionManager.createSession("user789", Duration.ofSeconds(2));
        System.out.println("Created short-lived session for user789: " + sessionId4);
        System.out.println("Waiting for 3 seconds for session to expire...");
        try {
            TimeUnit.SECONDS.sleep(3);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        Optional<String> result4 = sessionManager.getUserIdFromSession(sessionId4);
        System.out.println("Validation after expiration: " + (result4.isPresent() ? "Valid" : "Invalid"));


        System.out.println("\n--- Test Case 5: Multiple Concurrent Sessions ---");
        String userA_session = sessionManager.createSession("userA");
        String userB_session = sessionManager.createSession("userB");
        System.out.println("Created session for userA: " + userA_session);
        System.out.println("Created session for userB: " + userB_session);
        Optional<String> userA_result = sessionManager.getUserIdFromSession(userA_session);
        Optional<String> userB_result = sessionManager.getUserIdFromSession(userB_session);
        System.out.println("User A session validation: " + userA_result.orElse("Invalid"));
        System.out.println("User B session validation: " + userB_result.orElse("Invalid"));
        sessionManager.terminateSession(userA_session);
        System.out.println("Terminated session for userA.");
        userA_result = sessionManager.getUserIdFromSession(userA_session);
        userB_result = sessionManager.getUserIdFromSession(userB_session);
        System.out.println("User A session validation after termination: " + userA_result.orElse("Invalid"));
        System.out.println("User B session validation (should still be valid): " + userB_result.orElse("Invalid"));
    }
}