import java.security.MessageDigest;
import java.util.HashMap;
import java.util.Map;
import java.nio.charset.StandardCharsets;
import javax.xml.bind.DatatypeConverter;

public class Lse93 {

    // In-memory map to simulate a database
    private static Map<String, String> database = new HashMap<>();

    /**
     * Hashes a password using the SHA-256 algorithm.
     * @param password The plain-text password.
     * @return The SHA-256 hashed password as a hex string.
     */
    public static String hashPassword(String password) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(password.getBytes(StandardCharsets.UTF_8));
            // Convert byte array to a hex string
            return DatatypeConverter.printHexBinary(hash).toLowerCase();
        } catch (java.security.NoSuchAlgorithmException e) {
            System.err.println("Error: SHA-256 algorithm not found.");
            return null;
        }
    }

    /**
     * "Inserts" a new user into the database after hashing their password.
     * @param username The username.
     * @param password The plain-text password.
     */
    public static void registerUser(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            System.out.println("Registration failed: Username and password cannot be empty.");
            return;
        }
        
        if (database.containsKey(username)) {
            System.out.println("Registration failed: Username '" + username + "' already exists.");
            return;
        }

        String hashedPassword = hashPassword(password);
        if (hashedPassword != null) {
            database.put(username, hashedPassword);
            System.out.println("User '" + username + "' registered successfully.");
        } else {
            System.out.println("Registration failed for user '" + username + "' due to a hashing error.");
        }
    }
    
    /**
     * Prints the contents of the simulated database.
     */
    public static void printDatabase() {
        System.out.println("\n--- Database Contents ---");
        if (database.isEmpty()) {
            System.out.println("Database is empty.");
        } else {
            for (Map.Entry<String, String> entry : database.entrySet()) {
                System.out.println("Username: " + entry.getKey() + ", Hashed Password: " + entry.getValue());
            }
        }
        System.out.println("-------------------------");
    }

    public static void main(String[] args) {
        // 5 Test Cases
        System.out.println("--- Running Test Cases ---");
        registerUser("alice", "password123");
        registerUser("bob", "my$ecret#");
        registerUser("charlie", "CharlieBrown");
        registerUser("admin", "admin_pass");
        registerUser("alice", "new_password"); // Test existing user
        System.out.println("--------------------------\n");
        
        // Print the final state of the database
        printDatabase();
    }
}