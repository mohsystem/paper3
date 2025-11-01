import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;

public class Lse96 {

    // Simulates a user database (username -> stored_value)
    private static final Map<String, String> userDatabase = new HashMap<>();

    /**
     * Generates a cryptographically secure salt.
     * @return A 16-byte salt.
     */
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Converts a byte array into a hexadecimal string.
     * @param bytes The byte array to convert.
     * @return The hexadecimal string representation.
     */
    private static String bytesToHex(byte[] bytes) {
        StringBuilder hexString = new StringBuilder(2 * bytes.length);
        for (byte b : bytes) {
            String hex = Integer.toHexString(0xff & b);
            if (hex.length() == 1) {
                hexString.append('0');
            }
            hexString.append(hex);
        }
        return hexString.toString();
    }

    /**
     * Hashes a password using a given salt with the SHA-256 algorithm.
     * @param password The password string to hash.
     * @param salt The salt to use for hashing.
     * @return The hexadecimal string of the hashed password.
     */
    public static String hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            // Combine salt and password before hashing
            md.update(salt);
            byte[] hashedPassword = md.digest(password.getBytes(StandardCharsets.UTF_8));
            return bytesToHex(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            // This should never happen with SHA-256
            throw new RuntimeException("SHA-256 algorithm not found", e);
        }
    }

    /**
     * Registers a new user by generating a salt, hashing the password,
     * and storing the result in the simulated database.
     * @param username The username for the new user.
     * @param password The password for the new user.
     * @return The stored value (salt + hashed_password) or an error message.
     */
    public static String registerUser(String username, String password) {
        if (userDatabase.containsKey(username)) {
            return "Error: Username already exists.";
        }
        if (password == null || password.isEmpty()) {
            return "Error: Password cannot be empty.";
        }

        byte[] saltBytes = generateSalt();
        String hashedPassword = hashPassword(password, saltBytes);
        String saltHex = bytesToHex(saltBytes);

        // Concatenate hex representation of salt and the hashed password for storage
        String storedValue = saltHex + hashedPassword;
        userDatabase.put(username, storedValue);

        return storedValue;
    }

    public static void main(String[] args) {
        System.out.println("--- Java Registration Simulation ---");

        // Test Case 1
        System.out.println("\n1) Registering user 'alice'...");
        String aliceData = registerUser("alice", "password123");
        System.out.println("   Stored data for alice: " + aliceData);

        // Test Case 2
        System.out.println("\n2) Registering user 'bob'...");
        String bobData = registerUser("bob", "mySecret!@#");
        System.out.println("   Stored data for bob: " + bobData);

        // Test Case 3
        System.out.println("\n3) Registering user 'charlie'...");
        String charlieData = registerUser("charlie", "P@ssw0rd_");
        System.out.println("   Stored data for charlie: " + charlieData);
        
        // Test Case 4
        System.out.println("\n4) Attempting to register 'alice' again...");
        String aliceDuplicateData = registerUser("alice", "anotherPassword");
        System.out.println("   Result: " + aliceDuplicateData);

        // Test Case 5
        System.out.println("\n5) Registering user 'dave'...");
        String daveData = registerUser("dave", "davespassword");
        System.out.println("   Stored data for dave: " + daveData);
        
        System.out.println("\n--- Final Database State ---");
        userDatabase.forEach((key, value) -> System.out.println("User: " + key + ", Stored: " + value));
    }
}