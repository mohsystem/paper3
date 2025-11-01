import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Formatter;

public class Lse96 {

    private static final int SALT_LENGTH_BYTES = 16;
    private static final String HASHING_ALGORITHM = "SHA-256";

    /**
     * Generates a cryptographically secure random salt.
     *
     * @return A byte array containing the salt.
     */
    public static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[SALT_LENGTH_BYTES];
        random.nextBytes(salt);
        return salt;
    }

    /**
     * Hashes the password with the given salt using SHA-256.
     *
     * @param password The plain-text password.
     * @param salt The salt to use.
     * @return The hashed password as a byte array.
     */
    public static byte[] hashPassword(String password, byte[] salt) {
        try {
            MessageDigest md = MessageDigest.getInstance(HASHING_ALGORITHM);
            
            // Add salt to the digest
            md.update(salt);
            
            // Hash the password and return the bytes
            byte[] hashedPassword = md.digest(password.getBytes());
            return hashedPassword;
        } catch (NoSuchAlgorithmException e) {
            // This should never happen with a standard algorithm like SHA-256
            throw new RuntimeException("Could not find hashing algorithm: " + HASHING_ALGORITHM, e);
        }
    }

    /**
     * Converts a byte array to its hexadecimal string representation.
     *
     * @param bytes The byte array to convert.
     * @return The hexadecimal string.
     */
    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        try (Formatter formatter = new Formatter(sb)) {
            for (byte b : bytes) {
                formatter.format("%02x", b);
            }
        }
        return sb.toString();
    }
    
    /**
     * Simulates user registration by generating a salt, hashing the password,
     * and creating a string for database storage.
     *
     * @param username The username.
     * @param password The plain-text password.
     * @return A string representing the salt concatenated with the hash, for storage.
     */
    public static String registerUser(String username, String password) {
        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password, salt);
        
        String saltHex = bytesToHex(salt);
        String hashedPasswordHex = bytesToHex(hashedPassword);
        
        // In a real DB, you might store salt and hash in separate columns.
        // Here, we concatenate them for a single-string representation.
        return saltHex + hashedPasswordHex;
    }

    public static void main(String[] args) {
        // --- 5 Test Cases ---
        String[][] testUsers = {
            {"alice", "Password123!"},
            {"bob", "my$ecr3tP@ssw0rd"},
            {"charlie", "correct-horse-battery-staple"},
            {"david", "12345"},
            {"eve", "eve_password"}
        };

        System.out.println("--- Java User Registration Simulation ---");
        for (String[] user : testUsers) {
            String username = user[0];
            String password = user[1];
            String storedCredential = registerUser(username, password);
            
            System.out.println("Registering user: " + username);
            System.out.println("Stored credential: " + storedCredential);
            System.out.println();
        }
    }
}