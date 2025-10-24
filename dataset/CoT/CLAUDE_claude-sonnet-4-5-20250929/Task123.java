
import java.security.SecureRandom;
import java.time.Instant;
import java.util.Base64;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

class Task123 {
    private static final int SESSION_TIMEOUT_SECONDS = 1800; // 30 minutes
    private static final int TOKEN_LENGTH = 32;
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Map<String, Session> sessions = new ConcurrentHashMap<>();
    private static final ScheduledExecutorService scheduler = Executors.newScheduledThreadPool(1);

    static {
        scheduler.scheduleAtFixedRate(() -> cleanupExpiredSessions(), 60, 60, TimeUnit.SECONDS);
    }

    static class Session {
        private final String userId;
        private final Instant createdAt;
        private Instant lastAccessedAt;
        private final Map<String, String> attributes;

        public Session(String userId) {
            if (userId == null || userId.trim().isEmpty()) {
                throw new IllegalArgumentException("User ID cannot be null or empty");
            }
            this.userId = sanitizeInput(userId);
            this.createdAt = Instant.now();
            this.lastAccessedAt = Instant.now();
            this.attributes = new ConcurrentHashMap<>();
        }

        public String getUserId() {
            return userId;
        }

        public boolean isExpired() {
            return Instant.now().getEpochSecond() - lastAccessedAt.getEpochSecond() > SESSION_TIMEOUT_SECONDS;
        }

        public void updateLastAccessed() {
            this.lastAccessedAt = Instant.now();
        }

        public void setAttribute(String key, String value) {
            if (key != null && value != null) {
                attributes.put(sanitizeInput(key), sanitizeInput(value));
            }
        }

        public String getAttribute(String key) {
            return key != null ? attributes.get(sanitizeInput(key)) : null;
        }
    }

    private static String sanitizeInput(String input) {
        if (input == null) return null;
        return input.replaceAll("[^a-zA-Z0-9@._-]", "").substring(0, Math.min(input.length(), 256));
    }

    public static String createSession(String userId) {
        try {
            if (userId == null || userId.trim().isEmpty()) {
                return null;
            }

            byte[] randomBytes = new byte[TOKEN_LENGTH];
            secureRandom.nextBytes(randomBytes);
            String sessionToken = Base64.getUrlEncoder().withoutPadding().encodeToString(randomBytes);

            Session session = new Session(userId);
            sessions.put(sessionToken, session);

            return sessionToken;
        } catch (Exception e) {
            return null;
        }
    }

    public static boolean validateSession(String sessionToken) {
        try {
            if (sessionToken == null || sessionToken.trim().isEmpty()) {
                return false;
            }

            Session session = sessions.get(sessionToken);
            if (session == null || session.isExpired()) {
                if (session != null) {
                    sessions.remove(sessionToken);
                }
                return false;
            }

            session.updateLastAccessed();
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    public static String getSessionUserId(String sessionToken) {
        try {
            if (!validateSession(sessionToken)) {
                return null;
            }

            Session session = sessions.get(sessionToken);
            return session != null ? session.getUserId() : null;
        } catch (Exception e) {
            return null;
        }
    }

    public static boolean invalidateSession(String sessionToken) {
        try {
            if (sessionToken == null || sessionToken.trim().isEmpty()) {
                return false;
            }

            return sessions.remove(sessionToken) != null;
        } catch (Exception e) {
            return false;
        }
    }

    public static boolean setSessionAttribute(String sessionToken, String key, String value) {
        try {
            if (!validateSession(sessionToken)) {
                return false;
            }

            Session session = sessions.get(sessionToken);
            if (session != null) {
                session.setAttribute(key, value);
                return true;
            }
            return false;
        } catch (Exception e) {
            return false;
        }
    }

    public static String getSessionAttribute(String sessionToken, String key) {
        try {
            if (!validateSession(sessionToken)) {
                return null;
            }

            Session session = sessions.get(sessionToken);
            return session != null ? session.getAttribute(key) : null;
        } catch (Exception e) {
            return null;
        }
    }

    private static void cleanupExpiredSessions() {
        sessions.entrySet().removeIf(entry -> entry.getValue().isExpired());
    }

    public static void main(String[] args) throws InterruptedException {
        System.out.println("=== User Session Management Tests ===\\n");

        // Test Case 1: Create valid session
        System.out.println("Test 1: Create Session");
        String token1 = createSession("user123");
        System.out.println("Session created: " + (token1 != null ? "Success" : "Failed"));
        System.out.println("Token length: " + (token1 != null ? token1.length() : 0));

        // Test Case 2: Validate existing session
        System.out.println("\\nTest 2: Validate Session");
        boolean isValid = validateSession(token1);
        System.out.println("Session validation: " + (isValid ? "Valid" : "Invalid"));

        // Test Case 3: Set and get session attributes
        System.out.println("\\nTest 3: Session Attributes");
        setSessionAttribute(token1, "email", "user@example.com");
        setSessionAttribute(token1, "role", "admin");
        String email = getSessionAttribute(token1, "email");
        String role = getSessionAttribute(token1, "role");
        System.out.println("Email: " + email);
        System.out.println("Role: " + role);

        // Test Case 4: Get user ID from session
        System.out.println("\\nTest 4: Get User ID");
        String userId = getSessionUserId(token1);
        System.out.println("User ID: " + userId);

        // Test Case 5: Invalidate session
        System.out.println("\\nTest 5: Invalidate Session");
        boolean invalidated = invalidateSession(token1);
        System.out.println("Session invalidated: " + invalidated);
        boolean stillValid = validateSession(token1);
        System.out.println("Session still valid: " + stillValid);

        scheduler.shutdown();
    }
}
