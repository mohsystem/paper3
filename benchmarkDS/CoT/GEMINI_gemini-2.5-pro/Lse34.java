import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class Lse34 {

    // Simulates a database table of users.
    // In a real application, this would be a database, and passwords
    // would be hashed with a strong, salted algorithm like BCrypt or Argon2.
    private final Map<String, String> userDatabase = new HashMap<>();

    public Lse34() {
        // Pre-populate the database with some users and their hashed passwords.
        // Hashing passwords before storing them is a critical security practice.
        // These hashes are for "password123", "adminPass", and "securePwd!"
        registerUser("alice", "password123");
        registerUser("bob", "adminPass");
        registerUser("charlie", "securePwd!");
    }

    /**
     * Hashes a password using SHA-256.
     * NOTE: In a real-world scenario, use a stronger, salted hashing algorithm
     * like BCrypt, scrypt, or Argon2. SHA-256 alone is not sufficient.
     * @param password The plaintext password.
     * @return A hex string representation of the hashed password.
     */
    private String hashPassword(String password) {
        if (password == null) {
            return null;
        }
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] encodedhash = digest.digest(password.getBytes(java.nio.charset.StandardCharsets.UTF_8));
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
            // This should not happen for SHA-256
            throw new RuntimeException(e);
        }
    }

    /**
     * Registers a user by storing their username and a hash of their password.
     * @param username The username.
     * @param password The plaintext password.
     */
    public void registerUser(String username, String password) {
        if (username != null && !username.trim().isEmpty() && password != null) {
            String hashedPassword = hashPassword(password);
            userDatabase.put(username, hashedPassword);
        }
    }

    /**
     * Attempts to log a user in.
     * @param username The username entered by the user.
     * @param password The password entered by the user.
     * @return A string indicating the result.
     */
    public String login(String username, String password) {
        // 1. Basic input validation
        if (username == null || username.trim().isEmpty() || password == null) {
            return "Invalid username or password. Redirecting to login page...";
        }

        // 2. Retrieve the stored hash for the user
        String storedHashedPassword = userDatabase.get(username);

        // 3. If user does not exist, return a generic error message
        // to prevent username enumeration attacks.
        if (storedHashedPassword == null) {
            // We still hash the provided password to make the timing similar
            // to a failed password attempt, which can help mitigate timing attacks.
            hashPassword(password); 
            return "Invalid username or password. Redirecting to login page...";
        }

        // 4. Hash the provided password
        String providedHashedPassword = hashPassword(password);

        // 5. Compare the hashes in a way that is resistant to timing attacks.
        boolean passwordsMatch = MessageDigest.isEqual(
            storedHashedPassword.getBytes(),
            providedHashedPassword.getBytes()
        );

        if (passwordsMatch) {
            return "Login successful! Redirecting to dashboard page...";
        } else {
            return "Invalid username or password. Redirecting to login page...";
        }
    }

    public static void main(String[] args) {
        Lse34 authSystem = new Lse34();

        System.out.println("--- 5 Test Cases ---");

        // Test Case 1: Correct username and password
        System.out.println("1. Testing with correct credentials (alice):");
        System.out.println("   Result: " + authSystem.login("alice", "password123"));
        System.out.println();

        // Test Case 2: Correct username, incorrect password
        System.out.println("2. Testing with incorrect password (alice):");
        System.out.println("   Result: " + authSystem.login("alice", "wrongpassword"));
        System.out.println();

        // Test Case 3: Incorrect username
        System.out.println("3. Testing with non-existent user (david):");
        System.out.println("   Result: " + authSystem.login("david", "somepassword"));
        System.out.println();

        // Test Case 4: Null input for password
        System.out.println("4. Testing with null password (bob):");
        System.out.println("   Result: " + authSystem.login("bob", null));
        System.out.println();

        // Test Case 5: Correct credentials for another user
        System.out.println("5. Testing with correct credentials (charlie):");
        System.out.println("   Result: " + authSystem.login("charlie", "securePwd!"));
        System.out.println();
    }
}