import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

public class Lse137 {

    // --- Simulated Database ---
    private static class User {
        String username;
        String hashedPassword;
        String role;

        User(String username, String hashedPassword, String role) {
            this.username = username;
            this.hashedPassword = hashedPassword;
            this.role = role;
        }
    }

    private static final Map<String, User> userDatabase = new HashMap<>();
    private static final Map<String, String> sessionDatabase = new HashMap<>(); // SessionToken -> Username

    // --- Security Utilities ---
    
    // In a real application, use a strong, salted hashing algorithm like BCrypt or Argon2.
    // SHA-256 is used here for demonstration purposes.
    private static String hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
            return Base64.getEncoder().encodeToString(hash);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("Hashing algorithm not found", e);
        }
    }

    // --- Simulated Authentication and Authorization Logic ---

    /**
     * Simulates a user login. Authenticates credentials and returns a session token on success.
     * @return A session token string, or null if authentication fails.
     */
    public static String login(String username, String password) {
        User user = userDatabase.get(username);
        if (user != null) {
            String providedPasswordHash = hashPassword(password);
            // Securely compare password hashes to prevent timing attacks
            if (MessageDigest.isEqual(user.hashedPassword.getBytes(StandardCharsets.UTF_8), providedPasswordHash.getBytes(StandardCharsets.UTF_8))) {
                String sessionToken = UUID.randomUUID().toString();
                sessionDatabase.put(sessionToken, username);
                return sessionToken;
            }
        }
        return null;
    }

    /**
     * Securely checks if the user associated with a session token is a moderator.
     * This is the core authorization function.
     * @param session The session token provided by the user.
     * @return true if the user is a moderator, false otherwise.
     */
    public static boolean auth_mod(String session) {
        if (session == null || session.isEmpty() || !sessionDatabase.containsKey(session)) {
            return false;
        }
        String username = sessionDatabase.get(session);
        User user = userDatabase.get(username);
        return user != null && "moderator".equalsIgnoreCase(user.role);
    }

    /**
     * This function simulates being called from a URL like /check_mod.
     * As a secure practice, it should ignore username/password and rely solely on the session token
     * for authorization after a user has logged in.
     * @param username The username from the URL (ignored for security).
     * @param password The password from the URL (ignored for security).
     * @param session The session token from the URL.
     * @return The result of the auth_mod check.
     */
    public static boolean check_mod(String username, String password, String session) {
        // SECURITY NOTE: After login, authorization should be based on a session token,
        // not by re-submitting credentials. The username and password parameters are
        // ignored here to follow secure design principles.
        return auth_mod(session);
    }

    // --- Main Method with Test Cases ---
    public static void main(String[] args) {
        // 1. Setup: Populate user database
        userDatabase.put("admin", new User("admin", hashPassword("adminpass123"), "moderator"));
        userDatabase.put("bob", new User("bob", hashPassword("bobspassword"), "user"));
        userDatabase.put("charlie", new User("charlie", hashPassword("charP@ss!"), "moderator"));

        // 2. Simulate logins to get session tokens
        String adminSession = login("admin", "adminpass123");
        String bobSession = login("bob", "bobspassword");
        String failedLoginSession = login("admin", "wrongpassword");

        System.out.println("--- Test Cases for check_mod() ---");

        // Test Case 1: Valid moderator user
        boolean test1 = check_mod("admin", "adminpass123", adminSession);
        System.out.println("Test Case 1 (Admin is moderator): " + (test1 ? "Passed" : "Failed"));

        // Test Case 2: Valid non-moderator user
        boolean test2 = check_mod("bob", "bobspassword", bobSession);
        System.out.println("Test Case 2 (Bob is not moderator): " + (!test2 ? "Passed" : "Failed"));

        // Test Case 3: Invalid session token
        boolean test3 = check_mod("guest", "pass", "invalid-session-token-12345");
        System.out.println("Test Case 3 (Invalid session): " + (!test3 ? "Passed" : "Failed"));

        // Test Case 4: Null session token (user not logged in)
        boolean test4 = check_mod("admin", "wrongpassword", failedLoginSession); // failedLoginSession is null
        System.out.println("Test Case 4 (Null session): " + (!test4 ? "Passed" : "Failed"));
        
        // Test Case 5: Empty session token
        boolean test5 = check_mod("user", "pass", "");
        System.out.println("Test Case 5 (Empty session): " + (!test5 ? "Passed" : "Failed"));
    }
}