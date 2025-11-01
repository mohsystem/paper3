import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse97 {

    // In-memory map to simulate a database. In a real application, use a proper database.
    private static final Map<String, User> userDatabase = new HashMap<>();

    // User class to hold user data
    private static class User {
        String username;
        byte[] hashedPassword;
        byte[] salt;
        String firstName;
        String lastName;
        String email;

        User(String username, byte[] hashedPassword, byte[] salt, String firstName, String lastName, String email) {
            this.username = username;
            this.hashedPassword = hashedPassword;
            this.salt = salt;
            this.firstName = firstName;
            this.lastName = lastName;
            this.email = email;
        }
    }

    /**
     * Generates a cryptographically strong salt.
     * @return A 16-byte salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password using PBKDF2 with HMAC SHA-256.
     * @param password The password to hash.
     * @param salt The salt to use for hashing.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(String password, byte[] salt) {
        // PBKDF2 is a secure choice for password hashing as it includes salting and key stretching (iterations).
        int iterations = 65536; // High iteration count to slow down brute-force attacks.
        int keyLength = 256; // 256-bit key length.
        try {
            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, iterations, keyLength);
            SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");
            return factory.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, log this exception and handle it gracefully.
            throw new RuntimeException("Error while hashing password", e);
        }
    }

    /**
     * Registers a new user.
     * @param username The username.
     * @param password The user's plain-text password.
     * @param firstName The user's first name.
     * @param lastName The user's last name.
     * @param email The user's email.
     * @return A status message indicating success or failure.
     */
    public static String registerUser(String username, String password, String firstName, String lastName, String email) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            return "Username and password cannot be empty.";
        }

        if (userDatabase.containsKey(username)) {
            return "Username already exists.";
        }

        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password, salt);

        User newUser = new User(username, hashedPassword, salt, firstName, lastName, email);
        userDatabase.put(username, newUser);

        return "Registration succeeded.";
    }
    
    // Helper to print byte arrays for verification
    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases for User Registration ---");

        // Test Case 1: Successful registration
        String result1 = registerUser("jdoe", "Password123!", "John", "Doe", "j.doe@example.com");
        System.out.println("Test 1 (New User): " + result1);
        if (userDatabase.containsKey("jdoe")) {
            System.out.println("   > User 'jdoe' found in DB. Hash: " + bytesToHex(userDatabase.get("jdoe").hashedPassword).substring(0, 16) + "...");
        }

        // Test Case 2: Attempting to register with an existing username
        String result2 = registerUser("jdoe", "AnotherPass", "Jane", "Doe", "jane.d@example.com");
        System.out.println("Test 2 (Existing User): " + result2);

        // Test Case 3: Successful registration of a different user
        String result3 = registerUser("asmith", "SecureP@ssw0rd", "Alice", "Smith", "a.smith@example.com");
        System.out.println("Test 3 (Another New User): " + result3);
        if (userDatabase.containsKey("asmith")) {
            System.out.println("   > User 'asmith' found in DB. Hash: " + bytesToHex(userDatabase.get("asmith").hashedPassword).substring(0, 16) + "...");
        }

        // Test Case 4: Registration with an empty password
        String result4 = registerUser("bwhite", "", "Bob", "White", "b.white@example.com");
        System.out.println("Test 4 (Empty Password): " + result4);

        // Test Case 5: Registration with a null username
        String result5 = registerUser(null, "Pass123", "Charlie", "Chaplin", "c.c@example.com");
        System.out.println("Test 5 (Null Username): " + result5);
        
        System.out.println("\n--- Final Database State ---");
        System.out.println("Total users registered: " + userDatabase.size());
        userDatabase.keySet().forEach(username -> System.out.println("- " + username));
    }
}