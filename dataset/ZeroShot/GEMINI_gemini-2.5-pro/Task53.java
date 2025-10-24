import java.security.SecureRandom;
import java.util.Base64;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

public class Task53 {

    /**
     * Represents a single user session.
     */
    static class Session {
        private final String sessionId;
        private final String username;
        private final long creationTime;
        private volatile long lastAccessedTime;

        public Session(String sessionId, String username, long creationTime) {
            this.sessionId = sessionId;
            this.username = username;
            this.creationTime = creationTime;
            this.lastAccessedTime = creationTime;
        }

        public String getSessionId() { return sessionId; }
        public String getUsername() { return username; }
        public long getCreationTime() { return creationTime; }
        public long getLastAccessedTime() { return lastAccessedTime; }

        public void updateLastAccessedTime() {
            this.lastAccessedTime = System.currentTimeMillis();
        }

        @Override
        public String toString() {
            return "Session{" +
                   "sessionId='" + sessionId + '\'' +
                   ", username='" + username + '\'' +
                   ", creationTime=" + creationTime +
                   ", lastAccessedTime=" + lastAccessedTime +
                   '}';
        }
    }

    /**
     * Manages the lifecycle of user sessions.
     */
    static class SessionManager {
        // Use ConcurrentHashMap for thread safety in a real web application
        private static final Map<String, Session> activeSessions = new ConcurrentHashMap<>();
        // Use a short timeout for demonstration purposes
        private static final long SESSION_TIMEOUT_MS = TimeUnit.SECONDS.toMillis(10);
        private static final SecureRandom secureRandom = new SecureRandom();
        private static final Base64.Encoder base64Encoder = Base64.getUrlEncoder();

        /**
         * Creates a new session for a user.
         * @param username The username to associate with the session.
         * @return A cryptographically secure session ID.
         */
        public static String createSession(String username) {
            if (username == null || username.trim().isEmpty()) {
                throw new IllegalArgumentException("Username cannot be null or empty.");
            }
            byte[] randomBytes = new byte[24];
            secureRandom.nextBytes(randomBytes);
            String sessionId = base64Encoder.encodeToString(randomBytes);

            long now = System.currentTimeMillis();
            Session session = new Session(sessionId, username, now);
            activeSessions.put(sessionId, session);
            return sessionId;
        }

        /**
         * Retrieves a session if it exists and is not expired.
         * @param sessionId The ID of the session to retrieve.
         * @return An Optional containing the Session if valid, otherwise an empty Optional.
         */
        public static Optional<Session> getSession(String sessionId) {
            if (sessionId == null) {
                return Optional.empty();
            }
            Session session = activeSessions.get(sessionId);
            if (session == null) {
                return Optional.empty();
            }

            long now = System.currentTimeMillis();
            if (now - session.getLastAccessedTime() > SESSION_TIMEOUT_MS) {
                // Session expired
                activeSessions.remove(sessionId);
                return Optional.empty();
            }

            session.updateLastAccessedTime();
            return Optional.of(session);
        }

        /**
         * Deletes/invalidates a session.
         * @param sessionId The ID of the session to invalidate.
         */
        public static void invalidateSession(String sessionId) {
            if (sessionId != null) {
                activeSessions.remove(sessionId);
            }
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Session Management Test ---");

        // Test Case 1: Create and validate a session
        System.out.println("\n--- Test Case 1: Create and Validate Session ---");
        String aliceSessionId = SessionManager.createSession("alice");
        System.out.println("Created session for 'alice': " + aliceSessionId);
        Optional<Session> sessionOpt = SessionManager.getSession(aliceSessionId);
        sessionOpt.ifPresentOrElse(
            s -> System.out.println("Session is valid. User: " + s.getUsername()),
            () -> System.out.println("Session is invalid.")
        );

        // Test Case 2: Invalidate a session
        System.out.println("\n--- Test Case 2: Invalidate Session ---");
        String bobSessionId = SessionManager.createSession("bob");
        System.out.println("Created session for 'bob': " + bobSessionId);
        SessionManager.invalidateSession(bobSessionId);
        System.out.println("Invalidated session for 'bob'.");
        sessionOpt = SessionManager.getSession(bobSessionId);
        if (sessionOpt.isPresent()) {
            System.out.println("Error: Session for 'bob' should be invalid.");
        } else {
            System.out.println("Session for 'bob' is correctly invalidated.");
        }

        // Test Case 3: Session timeout
        System.out.println("\n--- Test Case 3: Session Timeout ---");
        String charlieSessionId = SessionManager.createSession("charlie");
        System.out.println("Created session for 'charlie': " + charlieSessionId);
        System.out.println("Waiting for session to time out (more than 10 seconds)...");
        try {
            Thread.sleep(11000); // Wait for 11 seconds to exceed the 10-second timeout
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        sessionOpt = SessionManager.getSession(charlieSessionId);
        if (sessionOpt.isPresent()) {
            System.out.println("Error: Session for 'charlie' should have timed out.");
        } else {
            System.out.println("Session for 'charlie' correctly timed out.");
        }
        
        // Test Case 4: Accessing invalid/non-existent session
        System.out.println("\n--- Test Case 4: Access Invalid Session ---");
        sessionOpt = SessionManager.getSession("invalid-session-id");
        if (sessionOpt.isPresent()) {
            System.out.println("Error: A non-existent session was found.");
        } else {
            System.out.println("Correctly handled non-existent session ID.");
        }

        // Test Case 5: Session activity extends lifetime
        System.out.println("\n--- Test Case 5: Session Activity Extends Lifetime ---");
        String daveSessionId = SessionManager.createSession("dave");
        System.out.println("Created session for 'dave': " + daveSessionId);
        System.out.println("Waiting for 7 seconds...");
        try {
            Thread.sleep(7000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        sessionOpt = SessionManager.getSession(daveSessionId);
        if (sessionOpt.isPresent()) {
            System.out.println("Accessed session for 'dave' successfully. Lifetime should be extended.");
        } else {
            System.out.println("Error: Session for 'dave' expired prematurely.");
        }
        
        System.out.println("Waiting for another 7 seconds...");
        try {
            Thread.sleep(7000);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        sessionOpt = SessionManager.getSession(daveSessionId);
        if (sessionOpt.isPresent()) {
            System.out.println("Session for 'dave' is still valid after 14 seconds due to activity.");
        } else {
            System.out.println("Error: Session for 'dave' expired despite recent activity.");
        }
    }
}