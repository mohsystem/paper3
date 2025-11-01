import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Base64;

public class Lse92 {

    /**
     * Hashes a password using PBKDF2 with a random salt.
     *
     * @param password The password to hash.
     * @return A string containing the salt and hash, suitable for storage.
     *         Returns null if an error occurs.
     */
    public static String hashPassword(String password) {
        try {
            // Generate a random salt
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);

            // Create PBEKeySpec
            // 10000 iterations is a common baseline, more is better.
            PBEKeySpec spec = new PBEKeySpec(password.toCharArray(), salt, 10000, 256);

            // Get instance of SecretKeyFactory for PBKDF2
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA256");

            // Generate the hash
            byte[] hash = skf.generateSecret(spec).getEncoded();

            // Combine salt and hash and encode to Base64 for storage
            // This format makes it easy to retrieve the salt for verification later
            Base64.Encoder enc = Base64.getEncoder();
            return enc.encodeToString(salt) + ":" + enc.encodeToString(hash);

        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            e.printStackTrace();
            return null;
        }
    }

    /**
     * Simulates a user registration.
     *
     * @param username The username.
     * @param password The user's plain-text password.
     */
    public static void registerUser(String username, String password) {
        System.out.println("Registering user: " + username);
        String hashedPassword = hashPassword(password);
        if (hashedPassword != null) {
            System.out.println("Stored information for " + username + ":");
            System.out.println("  Username: " + username);
            System.out.println("  Hashed Password (salt:hash): " + hashedPassword);
        } else {
            System.out.println("Failed to hash password for user: " + username);
        }
        System.out.println("------------------------------------");
    }

    public static void main(String[] args) {
        // 5 Test Cases
        registerUser("alice", "Password123!");
        registerUser("bob", "My$trongP@ssw0rd");
        registerUser("charlie", "Secr3t#Key");
        registerUser("david", "123456");
        registerUser("eve", "password");
    }
}