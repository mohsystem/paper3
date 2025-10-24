import java.security.SecureRandom;
import java.util.Base64;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

public class Task53 {

    /**
     * Manages user sessions in a thread-safe manner.
     * Session IDs are generated using a cryptographically secure random number generator.
     */
    public static class SessionManager {
        private static final int SESSION_ID_BYTES = 32;
        private final Map<String, SessionData> sessions = new ConcurrentHashMap<>();
        private final long sessionTimeoutMillis;
        private final SecureRandom secureRandom = new SecureRandom();

        private static class SessionData {
            private final String username;
            private final long creationTimeMillis;

            SessionData(String username, long creationTimeMillis) {
                this.username = username;
                this.creationTimeMillis = creationTimeMillis;
            }

            public String getUsername() {
                return username;
            }

            public long getCreationTimeMillis() {
                return creationTimeMillis;
            }
        }

        public SessionManager(long sessionTimeout, TimeUnit unit) {
            if (sessionTimeout <= 0) {
                throw new IllegalArgumentException("Session timeout must be positive.");
            }
            this.sessionTimeoutMillis = unit.toMillis(sessionTimeout);
        }

        /**
         * Creates a new session for the given username.
         * @param username The username to associate with the session. Cannot be null or empty.
         * @return A unique, secure session ID.
         */
        public String createSession(String username) {
            if (username == null || username.trim().isEmpty()) {
                throw new IllegalArgumentException("Username cannot be null or empty.");
            }

            byte[] randomBytes = new byte[SESSION_ID_BYTES];
            secureRandom.nextBytes(randomBytes);
            String sessionId = Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);

            SessionData data = new SessionData(username, System.currentTimeMillis());
            sessions.put(sessionId, data);
            return sessionId;
        }

        private Optional<SessionData> getValidSession(String sessionId) {
            if (sessionId == null || sessionId.isEmpty()) {
                return Optional.empty();
            }

            SessionData data = sessions.get(sessionId);
            if (data == null) {
                return Optional.empty();
            }

            long now = System.currentTimeMillis();
            if (now >= data.getCreationTimeMillis() + sessionTimeoutMillis) {
                sessions.remove(sessionId, data); // Eagerly remove expired session
                return Optional.empty();
            }

            return Optional.of(data);
        }
        
        /**
         * Checks if a session ID is valid and not expired.
         * @param sessionId The session ID to validate.
         * @return true if the session is valid, false otherwise.
         */
        public boolean isValidSession(String sessionId) {
            return getValidSession(sessionId).isPresent();
        }

        /**
         * Retrieves the username associated with a valid session.
         * @param sessionId The session ID.
         * @return An Optional containing the username if the session is valid, or an empty Optional.
         */
        public Optional<String> getSessionUser(String sessionId) {
            return getValidSession(sessionId).map(SessionData::getUsername);
        }
        
        /**
         * Ends and removes a session.
         * @param sessionId The session ID to end.
         */
        public void endSession(String sessionId) {
            if (sessionId != null) {
                sessions.remove(sessionId);
            }
        }
    }
    
    public static void main(String[] args) throws InterruptedException {
        // Test case 1: Create and validate a new session
        System.out.println("--- Test Case 1: Create and Validate Session ---");
        SessionManager manager = new SessionManager(30, TimeUnit.MINUTES);
        String aliceSessionId = manager.createSession("alice");
        System.out.println("Created session for alice: " + aliceSessionId);
        System.out.println("Is alice's session valid? " + manager.isValidSession(aliceSessionId));
        System.out.println("User for session: " + manager.getSessionUser(aliceSessionId).orElse("Not found"));
        System.out.println();

        // Test case 2: Validate a non-existent session
        System.out.println("--- Test Case 2: Validate Non-existent Session ---");
        String fakeSessionId = "fakeSessionId12345";
        System.out.println("Is fake session valid? " + manager.isValidSession(fakeSessionId));
        System.out.println("User for fake session: " + manager.getSessionUser(fakeSessionId).orElse("Not found"));
        System.out.println();

        // Test case 3: Invalidate a session by ending it
        System.out.println("--- Test Case 3: End Session ---");
        String bobSessionId = manager.createSession("bob");
        System.out.println("Created session for bob: " + bobSessionId);
        System.out.println("Is bob's session valid before ending? " + manager.isValidSession(bobSessionId));
        manager.endSession(bobSessionId);
        System.out.println("Ended bob's session.");
        System.out.println("Is bob's session valid after ending? " + manager.isValidSession(bobSessionId));
        System.out.println();
        
        // Test case 4: Session expiration
        System.out.println("--- Test Case 4: Session Expiration ---");
        SessionManager shortLivedManager = new SessionManager(2, TimeUnit.SECONDS);
        String charlieSessionId = shortLivedManager.createSession("charlie");
        System.out.println("Created short-lived session for charlie: " + charlieSessionId);
        System.out.println("Is charlie's session valid immediately? " + shortLivedManager.isValidSession(charlieSessionId));
        System.out.println("Waiting for 3 seconds...");
        Thread.sleep(3000);
        System.out.println("Is charlie's session valid after 3 seconds? " + shortLivedManager.isValidSession(charlieSessionId));
        System.out.println();

        // Test case 5: Attempt to create session with invalid username
        System.out.println("--- Test Case 5: Invalid Username ---");
        try {
            manager.createSession(null);
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception for null username: " + e.getMessage());
        }
        try {
            manager.createSession("   ");
        } catch (IllegalArgumentException e) {
            System.out.println("Caught expected exception for empty username: " + e.getMessage());
        }
    }
}