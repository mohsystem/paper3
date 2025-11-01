import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;

public class Lse97 {

    // In-memory map to simulate a database
    private static final Map<String, User> database = new HashMap<>();

    // User class to hold user data
    private static class User {
        String username;
        String hashedPassword;
        String firstName;
        String lastName;
        String email;

        public User(String username, String hashedPassword, String firstName, String lastName, String email) {
            this.username = username;
            this.hashedPassword = hashedPassword;
            this.firstName = firstName;
            this.lastName = lastName;
            this.email = email;
        }

        @Override
        public String toString() {
            return "User{" +
                   "username='" + username + '\'' +
                   ", hashedPassword='" + hashedPassword + '\'' +
                   ", firstName='" + firstName + '\'' +
                   ", lastName='" + lastName + '\'' +
                   ", email='" + email + '\'' +
                   '}';
        }
    }

    /**
     * Hashes a password using SHA-256.
     * @param password The password to hash.
     * @return The hex string representation of the hashed password.
     */
    public static String hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] encodedhash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
            StringBuilder hexString = new StringBuilder(2 * encodedhash.length);
            for (byte b : encodedhash) {
                String hex = Integer.toHexString(0xff & b);
                if (hex.length() == 1) {
                    hexString.append('0');
                }
                hexString.append(hex);
            }
            return hexString.toString();
        } catch (NoSuchAlgorithmException e) {
            // This should not happen with SHA-256
            throw new RuntimeException(e);
        }
    }

    /**
     * Registers a new user.
     * @param username The username.
     * @param password The plain text password.
     * @param firstName The user's first name.
     * @param lastName The user's last name.
     * @param email The user's email.
     * @return A message indicating success or failure.
     */
    public static String registerUser(String username, String password, String firstName, String lastName, String email) {
        if (database.containsKey(username)) {
            return "Username '" + username + "' already exists.";
        } else {
            String hashedPassword = hashPassword(password);
            User newUser = new User(username, hashedPassword, firstName, lastName, email);
            database.put(username, newUser);
            return "Registration succeeded for username '" + username + "'.";
        }
    }

    public static void main(String[] args) {
        // --- Test Cases ---

        // Test Case 1: Successful registration
        String result1 = registerUser("jdoe", "Password123", "John", "Doe", "jdoe@example.com");
        System.out.println("Test Case 1: " + result1);

        // Test Case 2: Another successful registration
        String result2 = registerUser("asmith", "SecurePass!", "Alice", "Smith", "asmith@example.com");
        System.out.println("Test Case 2: " + result2);

        // Test Case 3: Attempt to register with an existing username
        String result3 = registerUser("jdoe", "AnotherPass", "Jane", "Doe", "jane.d@example.com");
        System.out.println("Test Case 3: " + result3);

        // Test Case 4: A third successful registration
        String result4 = registerUser("peterpan", "neverland", "Peter", "Pan", "peter.p@example.com");
        System.out.println("Test Case 4: " + result4);

        // Test Case 5: Another failed registration with a duplicate username
        String result5 = registerUser("asmith", "MyPass123", "Adam", "Smithy", "a.smith@example.com");
        System.out.println("Test Case 5: " + result5);
        
        // Optional: Print the database content to verify
        // System.out.println("\nCurrent database state:");
        // database.forEach((key, value) -> System.out.println(key + " -> " + value));
    }
}