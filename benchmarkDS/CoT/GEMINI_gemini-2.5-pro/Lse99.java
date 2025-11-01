import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;
import java.util.Formatter;

public class Lse99 {

    // Simulate a database table for users using a HashMap
    private static final Map<String, String> userDatabase = new HashMap<>();

    /**
     * Converts a byte array to a hexadecimal string.
     * @param bytes The byte array to convert.
     * @return The hexadecimal string representation.
     */
    private static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);
        try (Formatter formatter = new Formatter(sb)) {
            for (byte b : bytes) {
                formatter.format("%02x", b);
            }
            return formatter.toString();
        }
    }

    /**
     * Registers a new user by hashing their password and storing it.
     * @param username The username to register.
     * @param password The plain-text password to hash and store.
     */
    public static void registerUser(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            System.out.println("Error: Username or password cannot be empty.");
            return;
        }
        if (userDatabase.containsKey(username)) {
            System.out.println("Error: Username '" + username + "' already exists.");
            return;
        }

        try {
            // 1. Generate a cryptographic salt. A new salt should be generated for each password.
            SecureRandom random = new SecureRandom();
            byte[] salt = new byte[16];
            random.nextBytes(salt);

            // 2. Use a strong hashing algorithm like SHA-256.
            // For password storage, algorithms like bcrypt, scrypt, or Argon2 are recommended
            // as they are computationally intensive, but SHA-256 with a salt is a secure minimum.
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            
            // 3. Add the salt to the digest before the password.
            md.update(salt);
            
            // 4. Hash the password.
            byte[] hashedPassword = md.digest(password.getBytes());

            // 5. Store the salt and the hash. A common practice is to concatenate them.
            String saltHex = bytesToHex(salt);
            String hashedPasswordHex = bytesToHex(hashedPassword);
            String storedPasswordFormat = saltHex + ":" + hashedPasswordHex;

            // 6. "Insert" the username and the formatted password hash into the database.
            userDatabase.put(username, storedPasswordFormat);
            System.out.println("User '" + username + "' registered successfully.");

        } catch (NoSuchAlgorithmException e) {
            // This should not happen if SHA-256 is a standard algorithm
            System.err.println("Fatal Error: Hashing algorithm not found.");
            // In a real application, log this and gracefully shutdown or notify admin.
        }
    }

    /**
     * Prints the contents of the simulated user database.
     */
    public static void printDatabase() {
        System.out.println("\n--- Simulated User Database ---");
        if (userDatabase.isEmpty()) {
            System.out.println("Database is empty.");
        } else {
            for (Map.Entry<String, String> entry : userDatabase.entrySet()) {
                System.out.println("Username: " + entry.getKey() + ", Stored Hash: " + entry.getValue());
            }
        }
        System.out.println("-----------------------------\n");
    }

    public static void main(String[] args) {
        // --- 5 Test Cases ---
        System.out.println("--- Running Java Test Cases ---");
        registerUser("alice", "Password123!");
        registerUser("bob", "my$ecretP@ss");
        registerUser("charlie", "Ch@rlieBrown");
        registerUser("david", "d@v1d_the_k1ng");
        registerUser("eve", "EveIsAwesome#2024");
        
        // Test case for existing user
        registerUser("alice", "AnotherPassword");

        // Print the final state of the database
        printDatabase();
    }
}