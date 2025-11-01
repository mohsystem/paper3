import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Lse99 {

    // A simple in-memory map to simulate a database.
    // In a real application, this would be a proper database table.
    private static final Map<String, StoredUser> userDatabase = new HashMap<>();
    
    // Constants for hashing
    private static final int SALT_LENGTH = 16; // 16 bytes = 128 bits
    private static final int HASH_STRENGTH = 65536; // Number of iterations
    private static final int KEY_LENGTH = 256; // Key length in bits
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Represents a user's data stored in the database.
     */
    private static class StoredUser {
        final String username;
        final String salt; // Stored as Base64 string
        final String hashedPassword; // Stored as Base64 string

        StoredUser(String username, String salt, String hashedPassword) {
            this.username = username;
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }

        @Override
        public String toString() {
            return "User(username=" + username + ", salt=" + salt + ", hashedPassword=" + hashedPassword + ")";
        }
    }

    /**
     * Generates a random salt.
     * @return a byte array containing the salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password using PBKDF2 with a given salt.
     * @param password The password to hash.
     * @param salt The salt to use.
     * @return The hashed password as a byte array.
     */
    private static byte[] hashPassword(char[] password, byte[] salt) {
        try {
            PBEKeySpec spec = new PBEKeySpec(password, salt, HASH_STRENGTH, KEY_LENGTH);
            SecretKeyFactory skf = SecretKeyFactory.getInstance(HASH_ALGORITHM);
            return skf.generateSecret(spec).getEncoded();
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // This should not happen with standard algorithms
            throw new RuntimeException("Error while hashing password: " + e.getMessage(), e);
        }
    }
    
    /**
     * "Inserts" a new user into the database after hashing their password.
     * @param username The username.
     * @param password The plaintext password.
     */
    public static void registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.err.println("Username and password cannot be empty.");
            return;
        }
        
        if (userDatabase.containsKey(username)) {
            System.err.println("Username '" + username + "' already exists.");
            return;
        }

        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password.toCharArray(), salt);

        // Encode salt and hash to Base64 for safe storage (e.g., in a text-based DB field)
        String saltString = Base64.getEncoder().encodeToString(salt);
        String hashedPasswordString = Base64.getEncoder().encodeToString(hashedPassword);
        
        StoredUser newUser = new StoredUser(username, saltString, hashedPasswordString);
        
        // Simulate inserting into the database
        userDatabase.put(username, newUser);
        
        System.out.println("User '" + username + "' registered successfully.");
    }

    public static void main(String[] args) {
        // --- 5 Test Cases ---
        System.out.println("--- Running 5 Test Cases ---");

        registerUser("alice", "Password123!");
        registerUser("bob", "My$trongP@ssw0rd");
        registerUser("charlie", "Ch@rlieBrown");
        registerUser("diana", "Secr3t-P@ss");
        registerUser("eve", "password"); // Example of a weak password
        
        System.out.println("\n--- Simulated Database State ---");
        for (Map.Entry<String, StoredUser> entry : userDatabase.entrySet()) {
            System.out.println(entry.getKey() + " -> " + entry.getValue());
        }
    }
}