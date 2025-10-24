import java.security.SecureRandom;
import java.time.Duration;
import java.time.Instant;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;

public class Task43 {

    /**
     * Represents a user session.
     */
    private static class Session {
        private final String userId;
        private final Instant creationTime;
        private Instant lastAccessedTime;

        public Session(String userId) {
            this.userId = userId;
            this.creationTime = Instant.now();
            this.lastAccessedTime = this.creationTime;
        }

        public String getUserId() {
            return userId;
        }

        public Instant getCreationTime() {
            return creationTime;
        }

        public Instant getLastAccessedTime() {
            return lastAccessedTime;
        }

        public void updateLastAccessedTime() {
            this.lastAccessedTime = Instant.now();
        }
    }

    /**
     * Manages user sessions securely.
     */
    public static class SessionManager {
        // Use a thread-safe map to store sessions.
        private final Map<String, Session> sessionStore = new ConcurrentHashMap<>();
        private final SecureRandom secureRandom = new SecureRandom();
        private static final long ABSOLUTE_TIMEOUT_SECONDS = 5; // e.g., 30 minutes
        private static final long IDLE_TIMEOUT_SECONDS = 2;   // e.g., 5 minutes

        /**
         * Creates a new session for a given user.
         *
         * @param userId The ID of the user.
         * @return A cryptographically secure session ID.
         */
        public String createSession(String userId) {
            byte[] randomBytes = new byte[32];
            secureRandom.nextBytes(randomBytes);
            String sessionId = Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);

            Session session = new Session(userId);
            sessionStore.put(sessionId, session);
            return sessionId;
        }

        /**
         * Retrieves and validates a session.
         *
         * @param sessionId The session ID to validate.
         * @return The Session object if valid, null otherwise.
         */
        public Session getSession(String sessionId) {
            if (sessionId == null || sessionId.isEmpty()) {
                return null;
            }

            Session session = sessionStore.get(sessionId);
            if (session == null) {
                return null; // Session does not exist
            }

            Instant now = Instant.now();
            // Check for absolute timeout
            if (Duration.between(session.getCreationTime(), now).getSeconds() > ABSOLUTE_TIMEOUT_SECONDS) {
                invalidateSession(sessionId);
                return null; // Session expired
            }

            // Check for idle timeout
            if (Duration.between(session.getLastAccessedTime(), now).getSeconds() > IDLE_TIMEOUT_SECONDS) {
                invalidateSession(sessionId);
                return null; // Session expired due to inactivity
            }

            // If valid, update the last accessed time
            session.updateLastAccessedTime();
            return session;
        }

        /**
         * Invalidates (terminates) a session.
         *
         * @param sessionId The session ID to invalidate.
         */
        public void invalidateSession(String sessionId) {
            if (sessionId != null) {
                sessionStore.remove(sessionId);
            }
        }
    }

    public static void main(String[] args) {
        SessionManager manager = new SessionManager();
        System.out.println("Running Java Session Manager Test Cases...");
        System.out.println("Absolute Timeout: " + SessionManager.ABSOLUTE_TIMEOUT_SECONDS + "s, Idle Timeout: " + SessionManager.IDLE_TIMEOUT_SECONDS + "s\n");

        // Test Case 1: Create and validate a new session
        System.out.println("--- Test Case 1: Create and Validate Session ---");
        String sessionId1 = manager.createSession("user123");
        System.out.println("Created session for user123: " + sessionId1);
        Session s1 = manager.getSession(sessionId1);
        System.out.println("Session valid? " + (s1 != null ? "Yes, for user " + s1.getUserId() : "No"));
        System.out.println();

        // Test Case 2: Invalidate a session
        System.out.println("--- Test Case 2: Invalidate Session ---");
        manager.invalidateSession(sessionId1);
        System.out.println("Session invalidated.");
        s1 = manager.getSession(sessionId1);
        System.out.println("Session valid after invalidation? " + (s1 != null));
        System.out.println();

        // Test Case 3: Idle timeout
        System.out.println("--- Test Case 3: Idle Timeout ---");
        String sessionId2 = manager.createSession("user456");
        System.out.println("Created session for user456: " + sessionId2);
        try {
            System.out.println("Waiting for " + (SessionManager.IDLE_TIMEOUT_SECONDS + 1) + " seconds to trigger idle timeout...");
            TimeUnit.SECONDS.sleep(SessionManager.IDLE_TIMEOUT_SECONDS + 1);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        Session s2 = manager.getSession(sessionId2);
        System.out.println("Session valid after idle period? " + (s2 != null));
        System.out.println();

        // Test Case 4: Absolute timeout
        System.out.println("--- Test Case 4: Absolute Timeout ---");
        String sessionId3 = manager.createSession("user789");
        System.out.println("Created session for user789: " + sessionId3);
        try {
            System.out.println("Accessing session every second to prevent idle timeout...");
            for (int i = 0; i < SessionManager.ABSOLUTE_TIMEOUT_SECONDS - 1; i++) {
                 TimeUnit.SECONDS.sleep(1);
                 manager.getSession(sessionId3); // Keep it active
                 System.out.println("Accessed at second " + (i+1));
            }
            System.out.println("Waiting for absolute timeout...");
            TimeUnit.SECONDS.sleep(2);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        Session s3 = manager.getSession(sessionId3);
        System.out.println("Session valid after absolute timeout period? " + (s3 != null));
        System.out.println();
        
        // Test Case 5: Prevent idle timeout by activity
        System.out.println("--- Test Case 5: Prevent Idle Timeout ---");
        String sessionId4 = manager.createSession("userABC");
        System.out.println("Created session for userABC: " + sessionId4);
        try {
            TimeUnit.SECONDS.sleep(SessionManager.IDLE_TIMEOUT_SECONDS - 1);
            System.out.println("Accessing session just before idle timeout...");
            Session s4_active = manager.getSession(sessionId4);
            System.out.println("Session is still active? " + (s4_active != null));
            
            TimeUnit.SECONDS.sleep(SessionManager.IDLE_TIMEOUT_SECONDS - 1);
            s4_active = manager.getSession(sessionId4);
            System.out.println("Accessing again. Session still active? " + (s4_active != null));

        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        }
        System.out.println();
    }
}