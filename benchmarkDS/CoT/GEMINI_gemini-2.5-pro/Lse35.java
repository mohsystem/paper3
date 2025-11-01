import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.HexFormat;

public class Lse35 {

    // In-memory map to simulate a user database.
    // In a real application, this would be a database table.
    // The value stores "salt:hashed_password"
    private static final Map<String, String> userDatabase = new HashMap<>();

    // A utility to convert byte arrays to hex strings and back.
    private static final HexFormat hexFormat = HexFormat.of();

    /**
     * Hashes a password with a given salt using SHA-256.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a hex string.
     */
    private static String hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedPassword = md.digest(password.getBytes(StandardCharsets.UTF_8));
            return hexFormat.formatHex(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            // This should never happen for SHA-256
            throw new RuntimeException("SHA-256 algorithm not found", e);
        }
    }

    /**
     * Generates a cryptographically strong random salt.
     * @return A 16-byte salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Simulates user registration by hashing a password and storing it.
     * @param username The username.
     * @param password The plaintext password.
     */
    private static void registerUser(String username, String password) {
        byte[] salt = generateSalt();
        String hashedPassword = hashPassword(password, salt);
        // Store the salt and hash together, separated by a colon.
        userDatabase.put(username, hexFormat.formatHex(salt) + ":" + hashedPassword);
    }

    /**
     * Authenticates a user by checking their username and password.
     *
     * @param username The username provided by the user.
     * @param password The password provided by the user.
     * @return true if authentication is successful, false otherwise.
     */
    public static boolean login(String username, String password) {
        // 1. Check if username exists. This prevents username enumeration timing attacks
        // if the subsequent steps take significantly different amounts of time.
        if (!userDatabase.containsKey(username)) {
            // We can perform a dummy hash calculation here to make the execution time
            // closer to a failed password attempt, further mitigating timing attacks.
            hashPassword(password, generateSalt());
            return false;
        }

        String storedSaltAndHash = userDatabase.get(username);
        String[] parts = storedSaltAndHash.split(":");
        if (parts.length != 2) {
            // Data integrity issue, should not happen.
            return false;
        }

        // 2. Extract salt and stored hash from the database entry.
        byte[] salt = hexFormat.parseHex(parts[0]);
        String storedHash = parts[1];

        // 3. Hash the provided password with the retrieved salt.
        String providedPasswordHash = hashPassword(password, salt);

        // 4. Compare the hashes in a way that is resistant to timing attacks.
        // Convert hex strings to byte arrays for secure comparison.
        byte[] storedHashBytes = hexFormat.parseHex(storedHash);
        byte[] providedHashBytes = hexFormat.parseHex(providedPasswordHash);
        
        return MessageDigest.isEqual(storedHashBytes, providedHashBytes);
    }

    public static void main(String[] args) {
        // Pre-populate the database with some users for testing.
        registerUser("admin", "AdminPass123!");
        registerUser("user1", "UserPass@456");
        registerUser("testuser", "password");

        // --- 5 Test Cases ---
        System.out.println("--- Test Cases ---");

        // Test Case 1: Successful login for admin
        System.out.println("\n1. Testing with correct admin credentials...");
        boolean isAdminLoggedIn = login("admin", "AdminPass123!");
        System.out.println("Login success: " + isAdminLoggedIn);
        System.out.println(isAdminLoggedIn ? "Redirecting to home page." : "Redirecting to login page.");

        // Test Case 2: Successful login for user1
        System.out.println("\n2. Testing with correct user1 credentials...");
        boolean isUser1LoggedIn = login("user1", "UserPass@456");
        System.out.println("Login success: " + isUser1LoggedIn);
        System.out.println(isUser1LoggedIn ? "Redirecting to home page." : "Redirecting to login page.");
        
        // Test Case 3: Failed login with wrong password
        System.out.println("\n3. Testing with incorrect password for admin...");
        boolean isWrongPassLoggedIn = login("admin", "wrongpassword");
        System.out.println("Login success: " + isWrongPassLoggedIn);
        System.out.println(isWrongPassLoggedIn ? "Redirecting to home page." : "Redirecting to login page.");

        // Test Case 4: Failed login with non-existent user
        System.out.println("\n4. Testing with non-existent username...");
        boolean isNonExistentUserLoggedIn = login("unknownuser", "somepassword");
        System.out.println("Login success: " + isNonExistentUserLoggedIn);
        System.out.println(isNonExistentUserLoggedIn ? "Redirecting to home page." : "Redirecting to login page.");

        // Test Case 5: Failed login with empty password
        System.out.println("\n5. Testing with empty password for user1...");
        boolean isEmptyPassLoggedIn = login("user1", "");
        System.out.println("Login success: " + isEmptyPassLoggedIn);
        System.out.println(isEmptyPassLoggedIn ? "Redirecting to home page." : "Redirecting to login page.");
    }
}