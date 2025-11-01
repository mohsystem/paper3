import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.HashMap;
import java.util.Map;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

public class Lse37 {
    private static class UserCredentials {
        final byte[] salt;
        final byte[] hashedPassword;

        UserCredentials(byte[] salt, byte[] hashedPassword) {
            this.salt = salt;
            this.hashedPassword = hashedPassword;
        }
    }

    // In a real application, this would be a database connection.
    private static final Map<String, UserCredentials> userDatabase = new HashMap<>();
    private static final int ITERATIONS = 600000;
    private static final int KEY_LENGTH = 256;
    private static final String ALGORITHM = "PBKDF2WithHmacSHA256";

    /**
     * Utility to pre-populate the database for the example.
     * This simulates user registration where a password is securely hashed.
     */
    public static void setupDatabase() {
        try {
            // User 1: admin
            String username = "admin";
            String password = "password123";
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);

            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] hashedPassword = factory.generateSecret(spec).getEncoded();

            userDatabase.put(username, new UserCredentials(salt, hashedPassword));

            // Add a user entry that represents corrupted data to trigger a simulated error
            userDatabase.put("error_user", null);

        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // In a real app, this would be a fatal startup error.
            System.err.println("Critical error setting up database: " + e.getMessage());
            System.exit(1);
        }
    }

    /**
     * Checks if the username and password are valid.
     *
     * @param username The username to check.
     * @param password The password to check.
     * @return "success", "cant_login", or "internal_server_error".
     */
    public static String login(String username, String password) {
        // Rule #1: Ensure all input is validated
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            return "cant_login";
        }

        // To prevent DoS with overly long inputs
        if (username.length() > 100 || password.length() > 100) {
            return "cant_login";
        }
        
        try {
            // User lookup
            if (!userDatabase.containsKey(username)) {
                // To prevent username enumeration via timing analysis, a real system
                // might perform a dummy hash calculation here.
                return "cant_login";
            }

            UserCredentials credentials = userDatabase.get(username);
            
            // Simulating an internal error case (e.g., corrupted data from DB)
            if (credentials == null) {
                throw new IllegalStateException("User data is corrupted for user: " + username);
            }

            byte[] salt = credentials.salt;

            // Hash the provided password with the stored salt
            KeySpec spec = new PBEKeySpec(password.toCharArray(), salt, ITERATIONS, KEY_LENGTH);
            SecretKeyFactory factory = SecretKeyFactory.getInstance(ALGORITHM);
            byte[] providedPasswordHash = factory.generateSecret(spec).getEncoded();

            // Rule #16: Use constant-time comparison to prevent timing attacks
            if (MessageDigest.isEqual(credentials.hashedPassword, providedPasswordHash)) {
                return "success";
            } else {
                return "cant_login";
            }
        // Rule #15: Ensure that all exceptions are caught and handled
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            // Log the error securely in a real application
            // e.printStackTrace(); 
            return "internal_server_error";
        } catch (Exception e) {
            // Catch other potential runtime issues
            // e.printStackTrace();
            return "internal_server_error";
        }
    }

    public static void main(String[] args) {
        setupDatabase();

        // 5 Test Cases
        System.out.println("--- 5 Test Cases ---");

        // 1. Correct login
        String result1 = login("admin", "password123");
        System.out.println("Test 1 (Correct login): " + result1); // Expected: success

        // 2. Incorrect password
        String result2 = login("admin", "wrongpassword");
        System.out.println("Test 2 (Incorrect password): " + result2); // Expected: cant_login

        // 3. Incorrect username
        String result3 = login("unknownuser", "password123");
        System.out.println("Test 3 (Incorrect username): " + result3); // Expected: cant_login

        // 4. Empty/null input
        String result4 = login("", null);
        System.out.println("Test 4 (Empty/null input): " + result4); // Expected: cant_login

        // 5. Internal error simulation
        String result5 = login("error_user", "any_password");
        System.out.println("Test 5 (Internal error simulation): " + result5); // Expected: internal_server_error
    }
}