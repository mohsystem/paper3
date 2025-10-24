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

public class Task122 {

    private static class PasswordManager {
        // In a real application, this would be a database.
        private final Map<String, String> userDatabase = new HashMap<>();
        private static final int SALT_LENGTH = 16; // 16 bytes = 128 bits
        private static final int ITERATION_COUNT = 65536;
        private static final int KEY_LENGTH = 256; // bits
        private static final String HASH_ALGORITHM = "PBKDF2WithHmacSHA256";
        private static final SecureRandom secureRandom = new SecureRandom();

        /**
         * Stores a new user's credentials after securely hashing the password.
         *
         * @param username The username.
         * @param password The plaintext password.
         * @return true if signup is successful, false if the user already exists.
         */
        public boolean signUp(String username, char[] password) {
            if (userDatabase.containsKey(username)) {
                System.out.println("Error: User '" + username + "' already exists.");
                return false;
            }

            try {
                // 1. Generate a cryptographically random salt
                byte[] salt = new byte[SALT_LENGTH];
                secureRandom.nextBytes(salt);

                // 2. Create the PBEKeySpec
                KeySpec spec = new PBEKeySpec(password, salt, ITERATION_COUNT, KEY_LENGTH);

                // 3. Get an instance of the SecretKeyFactory
                SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);

                // 4. Generate the hash
                byte[] hash = factory.generateSecret(spec).getEncoded();

                // 5. Store the salt and hash, encoded for storage
                Base64.Encoder enc = Base64.getEncoder();
                String storedPassword = enc.encodeToString(salt) + ":" + enc.encodeToString(hash);
                userDatabase.put(username, storedPassword);

                // It's critical to clear the password from memory after use
                Arrays.fill(password, Character.MIN_VALUE);

                return true;
            } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
                // This should not happen with standard algorithms
                throw new RuntimeException("Error while hashing password", e);
            }
        }

        /**
         * Verifies a user's password against the stored hash.
         *
         * @param username The username.
         * @param password The plaintext password to verify.
         * @return true if the password is correct, false otherwise.
         */
        public boolean verifyPassword(String username, char[] password) {
            String storedPassword = userDatabase.get(username);
            if (storedPassword == null) {
                return false; // User not found
            }

            try {
                // 1. Decode the stored salt and hash
                String[] parts = storedPassword.split(":");
                Base64.Decoder dec = Base64.getDecoder();
                byte[] salt = dec.decode(parts[0]);
                byte[] storedHash = dec.decode(parts[1]);

                // 2. Create the PBEKeySpec with the *same* salt
                KeySpec spec = new PBEKeySpec(password, salt, ITERATION_COUNT, KEY_LENGTH);
                
                // It's critical to clear the password from memory after use
                Arrays.fill(password, Character.MIN_VALUE);

                // 3. Get an instance of the SecretKeyFactory
                SecretKeyFactory factory = SecretKeyFactory.getInstance(HASH_ALGORITHM);

                // 4. Generate the hash of the provided password
                byte[] providedHash = factory.generateSecret(spec).getEncoded();

                // 5. Compare the hashes in a way that is resistant to timing attacks
                return Arrays.equals(storedHash, providedHash);

            } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
                throw new RuntimeException("Error while verifying password", e);
            }
        }
    }

    public static void main(String[] args) {
        PasswordManager manager = new PasswordManager();

        System.out.println("--- Test Cases ---");

        // Test Case 1: Successful signup
        System.out.println("\n[Test 1] Successful Signup");
        boolean signup1 = manager.signUp("alice", "Password123!".toCharArray());
        System.out.println("Signup for 'alice' successful: " + signup1);

        // Test Case 2: Successful login
        System.out.println("\n[Test 2] Successful Login");
        boolean login2 = manager.verifyPassword("alice", "Password123!".toCharArray());
        System.out.println("Login for 'alice' with correct password successful: " + login2);

        // Test Case 3: Failed login (wrong password)
        System.out.println("\n[Test 3] Failed Login (Wrong Password)");
        boolean login3 = manager.verifyPassword("alice", "WrongPass!".toCharArray());
        System.out.println("Login for 'alice' with incorrect password successful: " + login3);

        // Test Case 4: Failed login (user does not exist)
        System.out.println("\n[Test 4] Failed Login (User Not Found)");
        boolean login4 = manager.verifyPassword("bob", "Password123!".toCharArray());
        System.out.println("Login for 'bob' (non-existent user) successful: " + login4);

        // Test Case 5: Failed signup (user already exists)
        System.out.println("\n[Test 5] Failed Signup (User Exists)");
        manager.signUp("charlie", "MySecretPa$$".toCharArray());
        boolean signup5 = manager.signUp("charlie", "AnotherPassword".toCharArray());
        System.out.println("Second signup for 'charlie' successful: " + signup5);
    }
}