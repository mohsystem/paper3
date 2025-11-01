/*
For this Java code to run, you need the spring-security-crypto library.
If you are using Maven, add this to your pom.xml:
<dependency>
    <groupId>org.springframework.security</groupId>
    <artifactId>spring-security-crypto</artifactId>
    <version>5.7.3</version>
</dependency>
If you are using Gradle, add this to your build.gradle:
implementation 'org.springframework.security:spring-security-crypto:5.7.3'
*/
import java.util.HashMap;
import java.util.Map;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;

public class Lse93 {

    // A simple in-memory map to simulate a database.
    // In a real application, this would be a database table.
    private static final Map<String, String> userDatabase = new HashMap<>();

    /**
     * Hashes a password using bcrypt and stores the username and hashed password.
     *
     * @param username The username to be registered.
     * @param password The plain-text password to be hashed and stored.
     */
    public static void registerUser(String username, String password) {
        if (username == null || username.trim().isEmpty() || password == null || password.isEmpty()) {
            System.out.println("Error: Username and password cannot be null or empty.");
            return;
        }

        if (userDatabase.containsKey(username)) {
            System.out.println("Registration failed: Username '" + username + "' already exists.");
            return;
        }

        // Using BCrypt, a strong password-hashing function.
        // The salt is automatically generated and included in the output hash.
        BCryptPasswordEncoder passwordEncoder = new BCryptPasswordEncoder();
        String hashedPassword = passwordEncoder.encode(password);

        // Store the username and the hashed password in the simulated database
        userDatabase.put(username, hashedPassword);

        System.out.println("User '" + username + "' registered successfully.");
    }

    /**
     * Main method with test cases.
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        System.out.println("--- Running 5 Registration Test Cases ---");

        // Test Case 1: Standard registration
        registerUser("alice", "Password123!");

        // Test Case 2: Another standard registration
        registerUser("bob", "my$ecretP@ss");

        // Test Case 3: Registration with a different password
        registerUser("charlie", "Ch@rlieR0cks");

        // Test Case 4: Attempting to register a user that already exists
        registerUser("alice", "another_password");

        // Test Case 5: Registration with special characters in password
        registerUser("david", "`~'|/<>.,?*&^%$#@!");

        System.out.println("\n--- Simulated Database Contents ---");
        for (Map.Entry<String, String> entry : userDatabase.entrySet()) {
            System.out.println("Username: " + entry.getKey() + ", Hashed Password: " + entry.getValue());
        }
    }
}