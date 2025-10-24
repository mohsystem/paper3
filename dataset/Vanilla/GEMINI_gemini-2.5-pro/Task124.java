// WARNING: This code is for educational purposes only and is NOT secure.
// It demonstrates a basic in-memory data storage concept.
// Do NOT use this for storing real sensitive data.
// Real-world applications require robust encryption, secure storage, and compliance with standards like PCI DSS.
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

class UserProfile {
    private String name;
    private String email;
    private String creditCardNumber;

    public UserProfile(String name, String email, String creditCardNumber) {
        this.name = name;
        this.email = email;
        this.creditCardNumber = creditCardNumber;
    }

    public String getName() {
        return name;
    }

    public String getEmail() {
        return email;
    }
    
    // This getter is intentionally omitted for security to prevent accidental exposure of raw data.
    // public String getCreditCardNumber() { return creditCardNumber; }

    public String getMaskedCreditCardNumber() {
        if (creditCardNumber == null || creditCardNumber.length() <= 4) {
            return creditCardNumber;
        }
        int len = creditCardNumber.length();
        StringBuilder masked = new StringBuilder();
        for (int i = 0; i < len - 4; i++) {
            char c = creditCardNumber.charAt(i);
            if (Character.isDigit(c)) {
                masked.append('*');
            } else {
                masked.append(c);
            }
        }
        masked.append(creditCardNumber.substring(len - 4));
        return masked.toString();
    }

    @Override
    public String toString() {
        return "UserProfile{" +
                "name='" + name + '\'' +
                ", email='" + email + '\'' +
                ", creditCardNumber='" + getMaskedCreditCardNumber() + '\'' +
                '}';
    }
}

public class Task124 {

    /**
     * Stores a user profile in the in-memory database.
     * @param database The map representing the database.
     * @param user The UserProfile object to store.
     */
    public static void storeUserProfile(Map<String, UserProfile> database, UserProfile user) {
        if (database != null && user != null && user.getName() != null) {
            database.put(user.getName(), user);
        }
    }

    /**
     * Retrieves a user profile from the in-memory database.
     * @param database The map representing the database.
     * @param username The name of the user to retrieve.
     * @return The UserProfile object, or null if not found.
     */
    public static UserProfile retrieveUserProfile(Map<String, UserProfile> database, String username) {
        if (database != null && username != null) {
            return database.get(username);
        }
        return null;
    }

    public static void main(String[] args) {
        // This map acts as our simple in-memory, non-persistent data store.
        Map<String, UserProfile> userDatabase = new HashMap<>();

        System.out.println("--- Storing 5 User Profiles ---");

        // Test Case 1
        UserProfile user1 = new UserProfile("Alice", "alice@example.com", "1111-2222-3333-4444");
        storeUserProfile(userDatabase, user1);
        System.out.println("Stored: " + user1.getName());

        // Test Case 2
        UserProfile user2 = new UserProfile("Bob", "bob@example.com", "5555-6666-7777-8888");
        storeUserProfile(userDatabase, user2);
        System.out.println("Stored: " + user2.getName());

        // Test Case 3
        UserProfile user3 = new UserProfile("Charlie", "charlie@example.com", "9999-8888-7777-6666");
        storeUserProfile(userDatabase, user3);
        System.out.println("Stored: " + user3.getName());

        // Test Case 4
        UserProfile user4 = new UserProfile("Diana", "diana@example.com", "1234-5678-9012-3456");
        storeUserProfile(userDatabase, user4);
        System.out.println("Stored: " + user4.getName());

        // Test Case 5
        UserProfile user5 = new UserProfile("Eve", "eve@example.com", "0000-1111-2222-3333");
        storeUserProfile(userDatabase, user5);
        System.out.println("Stored: " + user5.getName());
        
        System.out.println("\n--- Retrieving User Profiles ---");

        // Retrieval Test 1
        System.out.println("Retrieving 'Bob':");
        UserProfile retrievedUser = retrieveUserProfile(userDatabase, "Bob");
        if (retrievedUser != null) {
            System.out.println("Found: " + retrievedUser);
        } else {
            System.out.println("User 'Bob' not found.");
        }

        // Retrieval Test 2
        System.out.println("\nRetrieving 'Diana':");
        retrievedUser = retrieveUserProfile(userDatabase, "Diana");
        if (retrievedUser != null) {
            System.out.println("Found: " + retrievedUser);
        } else {
            System.out.println("User 'Diana' not found.");
        }

        // Retrieval Test 3 (User not found)
        System.out.println("\nRetrieving 'Frank':");
        retrievedUser = retrieveUserProfile(userDatabase, "Frank");
        if (retrievedUser != null) {
            System.out.println("Found: " + retrievedUser);
        } else {
            System.out.println("User 'Frank' not found.");
        }
    }
}