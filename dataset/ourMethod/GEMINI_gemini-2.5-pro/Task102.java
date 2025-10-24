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
import java.util.Optional;

public class Task102 {

    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";
    private static final int ITERATIONS = 210000;
    private static final int KEY_LENGTH = 256;
    private static final int SALT_SIZE = 16;
    private static final SecureRandom RANDOM = new SecureRandom();

    // In-memory map to simulate a user database
    private static final Map<String, String> userDatabase = new HashMap<>();

    /**
     * Generates a salted hash for a given password.
     * @param password The password to hash.
     * @return An Optional containing the salted hash string, or empty if hashing fails.
     */
    public static Optional<String> hashPassword(char[] password) {
        byte[] salt = new byte[SALT_SIZE];
        RANDOM.nextBytes(salt);

        KeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
        try {
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] hash = factory.generateSecret(spec).getEncoded();
            
            Base64.Encoder enc = Base64.getEncoder();
            // Store as "salt:hash"
            return Optional.of(enc.encodeToString(salt) + ":" + enc.encodeToString(hash));
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real application, log this error securely
            System.err.println("Error while hashing password: " + e.getMessage());
            return Optional.empty();
        } finally {
             // Zero out the password array for security
            Arrays.fill(password, Character.MIN_VALUE);
        }
    }

    /**
     * Verifies a password against a stored salted hash.
     * @param password The password to verify.
     * @param storedHash The stored salted hash string.
     * @return true if the password is correct, false otherwise.
     */
    public static boolean verifyPassword(char[] password, String storedHash) {
        try {
            String[] parts = storedHash.split(":");
            if (parts.length != 2) {
                return false;
            }
            Base64.Decoder dec = Base64.getDecoder();
            byte[] salt = dec.decode(parts[0]);
            byte[] hash = dec.decode(parts[1]);

            KeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] testHash = factory.generateSecret(spec).getEncoded();
            
            // Constant-time comparison
            return java.security.MessageDigest.isEqual(hash, testHash);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException | IllegalArgumentException e) {
            System.err.println("Error while verifying password: " + e.getMessage());
            return false;
        } finally {
            Arrays.fill(password, Character.MIN_VALUE);
        }
    }
    
    /**
     * Updates a user's password in the database.
     * @param username The username of the user.
     * @param newPassword The new password.
     * @param database The map representing the database.
     * @return true if the update was successful, false otherwise.
     */
    public static boolean updateUserPassword(String username, String newPassword, Map<String, String> database) {
        if (username == null || username.trim().isEmpty()) {
            System.out.println("Update failed: Username cannot be empty.");
            return false;
        }
        if (newPassword == null || newPassword.length() < 12) {
            System.out.println("Update failed: Password must be at least 12 characters long.");
            return false;
        }
        
        // In a real application, you would check if the user exists first.
        // For this example, we add or update.
        // if (!database.containsKey(username)) {
        //     System.out.println("Update failed: User '" + username + "' not found.");
        //     return false;
        // }

        Optional<String> newHashedPassword = hashPassword(newPassword.toCharArray());
        if (newHashedPassword.isPresent()) {
            database.put(username, newHashedPassword.get());
            System.out.println("Password for user '" + username + "' has been updated successfully.");
            return true;
        } else {
            System.out.println("Failed to hash the new password for user '" + username + "'.");
            return false;
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Create a new user with a strong password
        System.out.println("\n[Test Case 1: Create new user 'alice']");
        updateUserPassword("alice", "Str0ngP@ssw0rd123!", userDatabase);
        System.out.println("Current DB state: " + userDatabase);
        
        // Test Case 2: Verify the correct password
        System.out.println("\n[Test Case 2: Verify correct password for 'alice']");
        boolean isVerified = verifyPassword("Str0ngP@ssw0rd123!".toCharArray(), userDatabase.get("alice"));
        System.out.println("Verification result: " + (isVerified ? "Success" : "Failure"));

        // Test Case 3: Verify an incorrect password
        System.out.println("\n[Test Case 3: Verify incorrect password for 'alice']");
        isVerified = verifyPassword("WrongPassword!".toCharArray(), userDatabase.get("alice"));
        System.out.println("Verification result: " + (isVerified ? "Success" : "Failure"));
        
        // Test Case 4: Update the password for an existing user
        System.out.println("\n[Test Case 4: Update password for 'alice']");
        updateUserPassword("alice", "EvenM0reSecur3P@ss!", userDatabase);
        System.out.println("Current DB state: " + userDatabase);
        
        // Test Case 5: Verify the new password
        System.out.println("\n[Test Case 5: Verify the new password for 'alice']");
        isVerified = verifyPassword("EvenM0reSecur3P@ss!".toCharArray(), userDatabase.get("alice"));
        System.out.println("Verification result: " + (isVerified ? "Success" : "Failure"));

        System.out.println("\n--- Additional Tests ---");
        // Test Case 6: Attempt to update password with a weak password
        System.out.println("\n[Test Case 6: Attempt weak password for 'bob']");
        updateUserPassword("bob", "weak", userDatabase);

        // Test Case 7: Attempt to update password with an empty username
        System.out.println("\n[Test Case 7: Attempt update with empty username]");
        updateUserPassword("", "ValidPassword12345", userDatabase);
    }
}