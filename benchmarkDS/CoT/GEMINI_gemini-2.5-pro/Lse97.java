import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse97 {

    // In-memory map to act as a database
    private static final Map<String, User> database = new HashMap<>();

    // User class to store user details
    private static class User {
        String username;
        String hashedPassword; // Stores salt + hash
        String firstName;
        String lastName;
        String email;

        User(String username, String hashedPassword, String firstName, String lastName, String email) {
            this.username = username;
            this.hashedPassword = hashedPassword;
            this.firstName = firstName;
            this.lastName = lastName;
            this.email = email;
        }
    }

    /**
     * Hashes a password with a salt using SHA-256.
     * In a real-world application, use a stronger algorithm like Argon2 or bcrypt.
     *
     * @param password The password to hash.
     * @return A string containing the salt and hash, separated by a colon.
     */
    private static String hashPassword(String password) {
        try {
            // Generate a random salt
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);

            // Create MessageDigest instance for SHA-256
            MessageDigest md = MessageDigest.getInstance("SHA-256");

            // Add salt to the digest
            md.update(salt);

            // Add password bytes to the digest
            byte[] hashedPassword = md.digest(password.getBytes(StandardCharsets.UTF_8));

            // Combine salt and hash for storage
            String encodedSalt = Base64.getEncoder().encodeToString(salt);
            String encodedHash = Base64.getEncoder().encodeToString(hashedPassword);
            
            return encodedSalt + ":" + encodedHash;
        } catch (NoSuchAlgorithmException e) {
            // This should never happen for SHA-256
            throw new RuntimeException(e);
        }
    }

    /**
     * Registers a new user.
     *
     * @param username The username.
     * @param password The user's password.
     * @param firstName The user's first name.
     * @param lastName The user's last name.
     * @param email The user's email address.
     * @return A status message indicating success or failure.
     */
    public static String registerUser(String username, String password, String firstName, String lastName, String email) {
        // Input validation
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            return "Registration failed: Username and password cannot be empty.";
        }

        // Check if username already exists
        if (database.containsKey(username)) {
            return "Registration failed: Username '" + username + "' already exists.";
        }

        // Hash the password
        String hashedPassword = hashPassword(password);

        // Create a new user and add to the "database"
        User newUser = new User(username, hashedPassword, firstName, lastName, email);
        database.put(username, newUser);

        return "Registration for '" + username + "' succeeded.";
    }

    public static void main(String[] args) {
        System.out.println("--- User Registration System ---");

        // Test Case 1: Successful registration
        String result1 = registerUser("jdoe", "Password123", "John", "Doe", "jdoe@example.com");
        System.out.println("Test Case 1: " + result1);

        // Test Case 2: Attempt to register with an existing username
        String result2 = registerUser("jdoe", "AnotherPass", "Jane", "Doe", "jane.d@example.com");
        System.out.println("Test Case 2: " + result2);

        // Test Case 3: Successful registration for another user
        String result3 = registerUser("asmith", "SecureP@ss!", "Alice", "Smith", "asmith@example.com");
        System.out.println("Test Case 3: " + result3);
        
        // Test Case 4: Registration with an empty password
        String result4 = registerUser("bwhite", "", "Bob", "White", "bwhite@example.com");
        System.out.println("Test Case 4: " + result4);

        // Test Case 5: Another successful registration
        String result5 = registerUser("cgreen", "MyP@ssw0rd", "Charlie", "Green", "cgreen@example.com");
        System.out.println("Test Case 5: " + result5);

        // Print database state for verification
        System.out.println("\nCurrent database state:");
        database.forEach((key, user) -> 
            System.out.println("Username: " + user.username + ", Hashed Password: " + user.hashedPassword)
        );
    }
}