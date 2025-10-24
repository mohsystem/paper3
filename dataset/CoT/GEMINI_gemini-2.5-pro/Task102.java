import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;

public class Task102 {

    // In-memory map to simulate a user database
    // In a real application, this would be a connection to a database (e.g., SQL, NoSQL).
    private static final Map<String, User> userDatabase = new HashMap<>();

    // User class to store user data, including hashed password and salt
    private static class User {
        private final String username;
        private byte[] passwordHash;
        private byte[] salt;

        public User(String username, byte[] passwordHash, byte[] salt) {
            this.username = username;
            this.passwordHash = passwordHash;
            this.salt = salt;
        }

        // Getters and Setters
        public void setPasswordHash(byte[] passwordHash) {
            this.passwordHash = passwordHash;
        }

        public void setSalt(byte[] salt) {
            this.salt = salt;
        }
        
        @Override
        public String toString() {
            return "User{" +
                   "username='" + username + '\'' +
                   ", passwordHash=" + Base64.getEncoder().encodeToString(passwordHash) +
                   ", salt=" + Base64.getEncoder().encodeToString(salt) +
                   '}';
        }
    }
    
    // --- Hashing Configuration ---
    private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final int SALT_SIZE_BYTES = 16;
    private static final int HASH_SIZE_BYTES = 32;
    private static final int ITERATION_COUNT = 65536;

    /**
     * Hashes a password using a randomly generated salt with PBKDF2.
     *
     * @param password The plain-text password to hash.
     * @param salt The salt to use for hashing.
     * @return The resulting password hash as a byte array.
     * @throws NoSuchAlgorithmException If the hashing algorithm is not available.
     * @throws InvalidKeySpecException If the key specification is invalid.
     */
    private static byte[] hashPassword(String password, byte[] salt)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATION_COUNT, HASH_SIZE_BYTES * 8);
        SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);
        return factory.generateSecret(spec).getEncoded();
    }

    /**
     * Generates a cryptographically secure random salt.
     *
     * @return A byte array containing the salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_SIZE_BYTES];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Updates a user's password in the simulated database.
     * This function hashes the new password and stores the new hash and salt.
     *
     * @param username The username of the user to update.
     * @param newPassword The new plain-text password.
     * @return true if the update was successful, false if the user does not exist.
     */
    public static boolean updateUserPassword(String username, String newPassword) {
        if (username == null || username.trim().isEmpty() || newPassword == null || newPassword.isEmpty()) {
            System.out.println("Error: Username and password cannot be null or empty.");
            return false;
        }

        User user = userDatabase.get(username);
        if (user == null) {
            System.out.println("Error: User '" + username + "' not found.");
            return false;
        }

        try {
            // Generate a new salt for each password change for security
            byte[] newSalt = generateSalt();
            byte[] newPasswordHash = hashPassword(newPassword, newSalt);

            // Update the user's record in the "database"
            user.setSalt(newSalt);
            user.setPasswordHash(newPasswordHash);
            userDatabase.put(username, user); // Re-put to update the map value

            System.out.println("Successfully updated password for user '" + username + "'.");
            return true;
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            System.err.println("Critical error during password hashing: " + e.getMessage());
            // In a real application, this should be logged seriously.
            return false;
        }
    }
    
    public static void main(String[] args) {
        // --- Setup: Pre-populate the database with some users ---
        try {
            byte[] salt1 = generateSalt();
            byte[] hash1 = hashPassword("oldPassword123", salt1);
            userDatabase.put("user1", new User("user1", hash1, salt1));
            
            byte[] salt2 = generateSalt();
            byte[] hash2 = hashPassword("anotherOldPwd", salt2);
            userDatabase.put("user2", new User("user2", hash2, salt2));
            
            System.out.println("Initial Database State:");
            userDatabase.forEach((key, value) -> System.out.println(key + ": " + value));
            System.out.println("----------------------------------------");

        } catch (Exception e) {
            e.printStackTrace();
            return;
        }
        
        // --- Test Cases ---
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Update password for an existing user
        System.out.println("\n[Test Case 1]: Update password for existing user 'user1'");
        updateUserPassword("user1", "newStrongPassword!@#");
        System.out.println("Current state for user1: " + userDatabase.get("user1"));

        // Test Case 2: Attempt to update password for a non-existent user
        System.out.println("\n[Test Case 2]: Attempt to update password for non-existent user 'unknownUser'");
        updateUserPassword("unknownUser", "somePassword");

        // Test Case 3: Attempt to update with a null or empty password
        System.out.println("\n[Test Case 3]: Attempt to update 'user2' with an empty password");
        updateUserPassword("user2", "");
        System.out.println("Current state for user2: " + userDatabase.get("user2"));


        // Test Case 4: Update password for the second user
        System.out.println("\n[Test Case 4]: Update password for existing user 'user2'");
        updateUserPassword("user2", "updatedP@ssword456");
        System.out.println("Current state for user2: " + userDatabase.get("user2"));
        
        // Test Case 5: Update password for the first user again
        System.out.println("\n[Test Case 5]: Update password for 'user1' again");
        updateUserPassword("user1", "evenNewerPassword$");
        System.out.println("Current state for user1: " + userDatabase.get("user1"));
        
        System.out.println("\n----------------------------------------");
        System.out.println("Final Database State:");
        userDatabase.forEach((key, value) -> System.out.println(key + ": " + value));
    }
}