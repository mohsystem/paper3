import java.util.HashMap;
import java.util.Map;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;

public class Lse58 {

    private static class User {
        String username;
        // In a real application, never store the password directly.
        // Store a strong, salted hash (e.g., using Argon2, scrypt, bcrypt).
        String hashedPassword;
        String salt;
        String email;

        User(String username, String hashedPassword, String salt, String email) {
            this.username = username;
            this.hashedPassword = hashedPassword;
            this.salt = salt;
            this.email = email;
        }
    }

    private final Map<String, User> users = new HashMap<>();
    private User currentUser = null;

    /**
     * Generates a random salt.
     * @return A byte array containing the salt.
     */
    private byte[] getSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes a password with a given salt using SHA-256.
     * WARNING: For a real-world application, use a stronger, adaptive hashing function
     * like Argon2, scrypt, or bcrypt, which are more resistant to brute-force attacks.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The Base64 encoded hashed password.
     */
    private String hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedPasswordBytes = md.digest(password.getBytes());
            return Base64.getEncoder().encodeToString(hashedPasswordBytes);
        } catch (NoSuchAlgorithmException e) {
            // This should not happen for SHA-256
            throw new RuntimeException("Could not hash password", e);
        }
    }

    /**
     * Registers a new user.
     * @return true if registration is successful, false if the username already exists.
     */
    public boolean registerUser(String username, String password, String email) {
        if (username == null || username.isEmpty() || password == null || password.isEmpty() || users.containsKey(username)) {
            return false;
        }
        byte[] saltBytes = getSalt();
        String saltString = Base64.getEncoder().encodeToString(saltBytes);
        String hashedPassword = hashPassword(password, saltBytes);
        User newUser = new User(username, hashedPassword, saltString, email);
        users.put(username, newUser);
        return true;
    }

    /**
     * Logs in a user.
     * @return true if login is successful, false otherwise.
     */
    public boolean login(String username, String password) {
        User user = users.get(username);
        if (user != null) {
            byte[] saltBytes = Base64.getDecoder().decode(user.salt);
            String providedPasswordHash = hashPassword(password, saltBytes);
            // In a real application, use a constant-time comparison function
            // to prevent timing attacks.
            if (providedPasswordHash.equals(user.hashedPassword)) {
                this.currentUser = user;
                return true;
            }
        }
        return false;
    }

    /**
     * Logs out the current user.
     */
    public void logout() {
        this.currentUser = null;
    }

    /**
     * Changes the email for the currently logged-in user.
     * @param oldEmail The user's current email for confirmation.
     * @param password The user's password for confirmation.
     * @param newEmail The new email address.
     * @return true if the email was changed successfully, false otherwise.
     */
    public boolean changeEmail(String oldEmail, String password, String newEmail) {
        if (this.currentUser == null) {
            // User must be logged in.
            return false;
        }

        // Validate old email
        if (!this.currentUser.email.equals(oldEmail)) {
            return false;
        }

        // Validate password
        byte[] saltBytes = Base64.getDecoder().decode(this.currentUser.salt);
        String providedPasswordHash = hashPassword(password, saltBytes);
        
        // In a real application, use a constant-time comparison.
        if (!providedPasswordHash.equals(this.currentUser.hashedPassword)) {
            return false;
        }

        // All checks passed, change the email.
        this.currentUser.email = newEmail;
        return true;
    }

    public static void main(String[] args) {
        Lse58 system = new Lse58();
        String testUser = "testuser";
        String testPass = "Password123!";
        String testEmail = "test@example.com";
        
        System.out.println("Registering user: " + testUser);
        system.registerUser(testUser, testPass, testEmail);

        // Test Case 1: Successful login and email change.
        System.out.println("\n--- Test Case 1: Successful Change ---");
        boolean loggedIn = system.login(testUser, testPass);
        System.out.println("Login successful: " + loggedIn);
        if (loggedIn) {
            boolean emailChanged = system.changeEmail(testEmail, testPass, "new@example.com");
            System.out.println("Email change successful: " + emailChanged);
            System.out.println("User's new email: " + system.currentUser.email);
        }
        system.logout();
        
        // Test Case 2: Attempt to change email with the wrong password.
        System.out.println("\n--- Test Case 2: Wrong Password ---");
        loggedIn = system.login(testUser, testPass);
        System.out.println("Login successful: " + loggedIn);
        if (loggedIn) {
            boolean emailChanged = system.changeEmail("new@example.com", "WrongPassword!", "another@example.com");
            System.out.println("Email change successful: " + emailChanged);
            System.out.println("User's email remains: " + system.currentUser.email);
        }
        system.logout();

        // Test Case 3: Attempt to change email with the wrong old email.
        System.out.println("\n--- Test Case 3: Wrong Old Email ---");
        loggedIn = system.login(testUser, testPass);
        System.out.println("Login successful: " + loggedIn);
        if (loggedIn) {
            boolean emailChanged = system.changeEmail("wrong@example.com", testPass, "another@example.com");
            System.out.println("Email change successful: " + emailChanged);
            System.out.println("User's email remains: " + system.currentUser.email);
        }
        system.logout();
        
        // Test Case 4: Attempt to change email when not logged in.
        System.out.println("\n--- Test Case 4: Not Logged In ---");
        boolean emailChanged = system.changeEmail("new@example.com", testPass, "another@example.com");
        System.out.println("Email change successful: " + emailChanged);
        
        // Test Case 5: Successful change after a failed attempt.
        System.out.println("\n--- Test Case 5: Successful Change After Failed Attempt ---");
        loggedIn = system.login(testUser, testPass);
        System.out.println("Login successful: " + loggedIn);
        if (loggedIn) {
             // First a failed attempt
            boolean failedChange = system.changeEmail("new@example.com", "badpass", "final@example.com");
            System.out.println("First change attempt (should fail): " + failedChange);
            // Then a successful one
            boolean successChange = system.changeEmail("new@example.com", testPass, "final@example.com");
            System.out.println("Second change attempt (should succeed): " + successChange);
            System.out.println("User's final email: " + system.currentUser.email);
        }
        system.logout();
    }
}