import java.util.HashMap;
import java.util.Map;

class User {
    private String name;
    private String email;
    private String password;

    public User(String name, String email, String password) {
        this.name = name;
        this.email = email;
        this.password = password;
    }

    public String getName() {
        return name;
    }

    public String getEmail() {
        return email;
    }

    @Override
    public String toString() {
        return "User{name='" + name + "', email='" + email + "'}";
    }
}

public class Task46 {

    // Simulating a database table with a Map (email is the unique key)
    private static Map<String, User> userDatabase = new HashMap<>();

    /**
     * Registers a new user in the database.
     * @param name The user's name.
     * @param email The user's email (must be unique).
     * @param password The user's password.
     * @return true if registration is successful, false if the email already exists.
     */
    public static boolean registerUser(String name, String email, String password) {
        if (userDatabase.containsKey(email)) {
            System.out.println("Registration failed: Email '" + email + "' already exists.");
            return false;
        }
        User newUser = new User(name, email, password);
        userDatabase.put(email, newUser);
        System.out.println("Registration successful for: " + newUser);
        return true;
    }

    public static void main(String[] args) {
        System.out.println("--- User Registration System ---");

        // Test Case 1: Successful registration
        System.out.println("\nTest Case 1:");
        registerUser("Alice", "alice@example.com", "password123");

        // Test Case 2: Successful registration
        System.out.println("\nTest Case 2:");
        registerUser("Bob", "bob@example.com", "securepass");

        // Test Case 3: Failed registration (duplicate email)
        System.out.println("\nTest Case 3:");
        registerUser("Alice Smith", "alice@example.com", "anotherpass");

        // Test Case 4: Successful registration
        System.out.println("\nTest Case 4:");
        registerUser("Charlie", "charlie@example.com", "charlie!@#");
        
        // Test Case 5: Successful registration
        System.out.println("\nTest Case 5:");
        registerUser("Diana", "diana@example.com", "diana_pass");

        System.out.println("\n--- Current Database State ---");
        for (Map.Entry<String, User> entry : userDatabase.entrySet()) {
            System.out.println("Email: " + entry.getKey() + ", User: " + entry.getValue());
        }
    }
}