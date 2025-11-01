import java.security.MessageDigest;
import java.util.HashMap;
import java.util.Map;
import java.util.Formatter;

public class Lse98 {

    // Simulating a database table with a HashMap
    private static final Map<String, String> userDatabase = new HashMap<>();

    /**
     * Hashes a password using the SHA-256 algorithm.
     * @param password The plain text password.
     * @return A hex string representation of the hashed password.
     */
    public static String hashPassword(String password) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] hashBytes = md.digest(password.getBytes("UTF-8"));
            
            // Convert byte array to hex string
            StringBuilder hexString = new StringBuilder();
            for (byte b : hashBytes) {
                hexString.append(String.format("%02x", b));
            }
            return hexString.toString();
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Registers a new user if the username doesn't already exist.
     * @param username The username for the new account.
     * @param password The password for the new account.
     * @return true if registration is successful, false otherwise.
     */
    public static boolean registerUser(String username, String password) {
        if (userDatabase.containsKey(username)) {
            System.out.println("Registration failed: Username '" + username + "' already exists.");
            return false;
        }

        String hashedPassword = hashPassword(password);
        userDatabase.put(username, hashedPassword);
        System.out.println("User '" + username + "' registered successfully.");
        return true;
    }

    public static void main(String[] args) {
        System.out.println("--- Java Registration Test ---");

        // Test Case 1: Successful registration
        registerUser("alice", "password123");

        // Test Case 2: Successful registration for another user
        registerUser("bob", "securePass!@#");

        // Test Case 3: Attempting to register with an existing username
        registerUser("alice", "anotherPassword");

        // Test Case 4: Successful registration with a complex username
        registerUser("charlie_1990", "my-secret-pw");
        
        // Test Case 5: Successful registration
        registerUser("david", "d@v1dP@$$");
        
        System.out.println("\n--- Current Database State ---");
        for (Map.Entry<String, String> entry : userDatabase.entrySet()) {
            System.out.println("Username: " + entry.getKey() + ", Hashed Password: " + entry.getValue());
        }
    }
}