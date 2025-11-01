import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class Lse137 {

    // Mock database for user credentials and roles
    // In a real application, this would be a secure database.
    private static final Map<String, User> userDatabase = new HashMap<>();
    // Mock store for active sessions
    // In a real application, this would be a distributed cache like Redis.
    private static final Map<String, String> sessionStore = new HashMap<>();

    // User record to store credentials and role
    private static class User {
        final String username;
        final byte[] passwordHash;
        final byte[] salt;
        final String role;

        User(String username, byte[] passwordHash, byte[] salt, String role) {
            this.username = username;
            this.passwordHash = passwordHash;
            this.salt = salt;
            this.role = role;
        }
    }

    /**
     * Generates a salt for password hashing.
     *
     * @return A 16-byte random salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes a password with a given salt using SHA-256.
     * In a real-world scenario, a stronger key derivation function like Argon2id or
     * PBKDF2WithHmacSHA256 should be used.
     *
     * @param password The password to hash.
     * @param salt     The salt to use.
     * @return The resulting hash.
     */
    private static byte[] hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            return md.digest(password.getBytes(StandardCharsets.UTF_8));
        } catch (NoSuchAlgorithmException e) {
            // This should not happen for SHA-256
            throw new RuntimeException("Could not find SHA-256 algorithm", e);
        }
    }
    
    /**
     * Initializes the mock database with some users.
     */
    private static void setupMockData() {
        // Moderator user
        byte[] modSalt = generateSalt();
        byte[] modHash = hashPassword("ModPassword123!", modSalt);
        userDatabase.put("moderator", new User("moderator", modHash, modSalt, "moderator"));
        sessionStore.put("SESSION_MOD_VALID", "moderator");

        // Regular user
        byte[] userSalt = generateSalt();
        byte[] userHash = hashPassword("UserPassword456$", userSalt);
        userDatabase.put("regular_user", new User("regular_user", userHash, userSalt, "user"));
        sessionStore.put("SESSION_USER_VALID", "regular_user");
    }

    /**
     * Authenticates a user and checks if they are a moderator.
     *
     * @param username The username.
     * @param password The password.
     * @param session  The session token.
     * @return true if the user is an authenticated moderator, false otherwise.
     */
    public static boolean auth_mod(String username, String password, String session) {
        // 1. Input validation
        if (username == null || username.isEmpty() || password == null || password.isEmpty() || session == null || session.isEmpty()) {
            return false;
        }

        // 2. Check if session is valid for the user
        if (!sessionStore.containsKey(session) || !sessionStore.get(session).equals(username)) {
            return false;
        }

        // 3. Look up user in the database
        User user = userDatabase.get(username);
        if (user == null) {
            return false;
        }

        // 4. Hash the provided password with the user's salt
        byte[] providedPasswordHash = hashPassword(password, user.salt);

        // 5. Compare hashes in a constant-time manner to prevent timing attacks
        if (!MessageDigest.isEqual(providedPasswordHash, user.passwordHash)) {
            return false;
        }

        // 6. Check if the user has the 'moderator' role
        return "moderator".equals(user.role);
    }

    /**
     * Simulates a controller/handler for a URL endpoint like /check_mod.
     * It extracts parameters and calls the authentication logic.
     *
     * @param username The username from the URL.
     * @param password The password from the URL.
     * @param session  The session token from the URL.
     */
    public static void check_mod(String username, String password, String session) {
        boolean isModerator = auth_mod(username, password, session);
        System.out.println("User '" + username + "' is moderator: " + isModerator);
    }

    public static void main(String[] args) {
        setupMockData();

        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Successful moderator login
        System.out.print("Test 1 (Success - Moderator): ");
        check_mod("moderator", "ModPassword123!", "SESSION_MOD_VALID");

        // Test Case 2: Successful login, but user is not a moderator
        System.out.print("Test 2 (Failure - Not a Moderator): ");
        check_mod("regular_user", "UserPassword456$", "SESSION_USER_VALID");

        // Test Case 3: Login with a wrong password
        System.out.print("Test 3 (Failure - Wrong Password): ");
        check_mod("moderator", "WrongPassword", "SESSION_MOD_VALID");

        // Test Case 4: Login with a non-existent user
        System.out.print("Test 4 (Failure - User Not Found): ");
        check_mod("unknown_user", "any_password", "SESSION_MOD_VALID");
        
        // Test Case 5: Login with an invalid session token
        System.out.print("Test 5 (Failure - Invalid Session): ");
        check_mod("moderator", "ModPassword123!", "INVALID_SESSION");
        
        System.out.println("--- Test Cases Finished ---");
    }
}