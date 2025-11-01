import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Formatter;

public class Lse94 {

    /**
     * Generates a cryptographically secure random salt.
     *
     * @return A 16-byte salt.
     */
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes a password using a given salt and the SHA-256 algorithm.
     * Note: For production systems, use a stronger, adaptive hashing function
     * like PBKDF2, bcrypt, or scrypt.
     *
     * @param password The password to hash.
     * @param salt     The salt to use for hashing.
     * @return The resulting hash as a byte array, or null on error.
     */
    private static byte[] hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedPassword = md.digest(password.getBytes(StandardCharsets.UTF_8));
            return hashedPassword;
        } catch (NoSuchAlgorithmException e) {
            // This should never happen for SHA-256
            e.printStackTrace();
            return null;
        }
    }

    /**
     * Converts a byte array to its hexadecimal string representation.
     *
     * @param bytes The byte array to convert.
     * @return The hexadecimal string.
     */
    private static String bytesToHex(byte[] bytes) {
        if (bytes == null) {
            return "";
        }
        Formatter formatter = new Formatter();
        for (byte b : bytes) {
            formatter.format("%02x", b);
        }
        return formatter.toString();
    }

    /**
     * Registers a user by hashing their password and simulating a database insert.
     *
     * @param username The username to register.
     * @param password The plaintext password.
     */
    public static void registerUser(String username, String password) {
        System.out.println("Registering user: " + username);
        
        // 1. Generate a salt
        byte[] salt = generateSalt();
        
        // 2. Hash the password with the salt
        byte[] hashedPassword = hashPassword(password, salt);

        if (hashedPassword == null) {
            System.out.println("Error: Could not hash password for user: " + username);
            return;
        }

        // Convert salt and hash to hex strings for storage
        String saltHex = bytesToHex(salt);
        String hashedPasswordHex = bytesToHex(hashedPassword);

        // 3. Simulate inserting into the database
        System.out.println("  -> Simulating database insert...");
        System.out.println("     Username: " + username);
        System.out.println("     Salt: " + saltHex);
        System.out.println("     Hashed Password: " + hashedPasswordHex);
        System.out.println("Registration complete.\n");
    }

    public static void main(String[] args) {
        // 5 Test Cases
        registerUser("alice", "Password123!");
        registerUser("bob", "my$ecretP@ss");
        registerUser("charlie", "qwerty");
        registerUser("diana", "12345678");
        registerUser("eve", "pa$$w0rd");
    }
}