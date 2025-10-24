import java.time.Instant;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

public class Task123 {

    /**
     * Securely manages user sessions in-memory.
     * Uses cryptographically strong random UUIDs for session IDs.
     * Sessions expire after a defined duration.
     * This implementation is thread-safe using ConcurrentHashMap.
     */
    public static class SessionManager {

        private static final long SESSION_DURATION_SECONDS = 1800; // 30 minutes
        private final Map<String, Session> activeSessions = new ConcurrentHashMap<>();

        private record Session(String userId, Instant expirationTime) {
            public boolean isExpired() {
                return Instant.now().isAfter(expirationTime);
            }
        }

        /**
         * Creates a new session for a given user.
         * @param userId The ID of the user to create a session for.
         * @return The generated session ID.
         */
        public String createSession(String userId) {
            if (userId == null || userId.trim().isEmpty()) {
                throw new IllegalArgumentException("User ID cannot be null or empty.");
            }
            String sessionId = generateSecureSessionId();
            Instant expirationTime = Instant.now().plusSeconds(SESSION_DURATION_SECONDS);
            Session session = new Session(userId, expirationTime);
            activeSessions.put(sessionId, session);
            return sessionId;
        }

        /**
         * Retrieves the user ID associated with a session, if the session is valid and not expired.
         * @param sessionId The session ID to validate.
         * @return An Optional containing the user ID if the session is valid, otherwise an empty Optional.
         */
        public Optional<String> getSessionUser(String sessionId) {
            if (sessionId == null) {
                return Optional.empty();
            }
            Session session = activeSessions.get(sessionId);

            if (session == null) {
                return Optional.empty(); // Session not found
            }

            if (session.isExpired()) {
                activeSessions.remove(sessionId); // Clean up expired session
                return Optional.empty();
            }

            return Optional.of(session.userId());
        }

        /**
         * Invalidates and removes a session.
         * @param sessionId The ID of the session to end.
         */
        public void endSession(String sessionId) {
            if (sessionId != null) {
                activeSessions.remove(sessionId);
            }
        }

        /**
         * Generates a cryptographically secure, unique session ID.
         * @return A new session ID string.
         */
        private String generateSecureSessionId() {
            return UUID.randomUUID().toString();
        }
    }

    public static void main(String[] args) {
        SessionManager sessionManager = new SessionManager();
        System.out.println("--- Session Management Test Cases ---");

        // Test Case 1: Create a new session for 'alice'
        System.out.println("\n1. Creating session for user 'alice'...");
        String aliceSessionId = sessionManager.createSession("alice");
        System.out.println("   Session created for alice with ID: " + aliceSessionId);

        // Test Case 2: Validate 'alice's' session
        System.out.println("\n2. Validating alice's session...");
        sessionManager.getSessionUser(aliceSessionId)
                .ifPresentOrElse(
                        userId -> System.out.println("   Success! Session is valid for user: " + userId),
                        () -> System.out.println("   Failure! Session is not valid.")
                );

        // Test Case 3: Validate a non-existent session ID
        System.out.println("\n3. Validating a fake session ID...");
        String fakeSessionId = "fake-session-id-12345";
        sessionManager.getSessionUser(fakeSessionId)
                .ifPresentOrElse(
                        userId -> System.out.println("   Failure! Fake session was validated for user: " + userId),
                        () -> System.out.println("   Success! Fake session was correctly identified as invalid.")
                );
        
        // Test Case 4: End 'alice's' session
        System.out.println("\n4. Ending alice's session...");
        sessionManager.endSession(aliceSessionId);
        System.out.println("   Session " + aliceSessionId + " has been ended.");
        
        // Test Case 5: Re-validate 'alice's' ended session
        System.out.println("\n5. Re-validating alice's ended session...");
        sessionManager.getSessionUser(aliceSessionId)
                .ifPresentOrElse(
                        userId -> System.out.println("   Failure! Ended session was validated for user: " + userId),
                        () -> System.out.println("   Success! Ended session is no longer valid.")
                );
    }
}