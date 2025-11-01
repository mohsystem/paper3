import java.util.HashMap;
import java.util.Map;

public class Lse97 {

    static class User {
        String firstName;
        String lastName;
        String email;
        String passwordHash;

        User(String firstName, String lastName, String email, String passwordHash) {
            this.firstName = firstName;
            this.lastName = lastName;
            this.email = email;
            this.passwordHash = passwordHash;
        }
    }

    private static final Map<String, User> db = new HashMap<>();

    private static String simpleHash(String input) {
        long h = 5381L;
        for (int i = 0; i < input.length(); i++) {
            h = ((h << 5) + h) + input.charAt(i); // h * 33 + c
        }
        // Convert to unsigned hex string
        return Long.toUnsignedString(h, 16);
    }

    public static String registerUser(String username, String password, String firstName, String lastName, String email) {
        if (db.containsKey(username)) {
            return "Username already exists";
        }
        String hash = simpleHash(password);
        db.put(username, new User(firstName, lastName, email, hash));
        return "Registration succeeded";
    }

    public static void main(String[] args) {
        System.out.println(registerUser("alice", "p@ss1", "Alice", "Wonder", "alice@example.com"));
        System.out.println(registerUser("bob", "12345", "Bob", "Builder", "bob@example.com"));
        System.out.println(registerUser("alice", "newpass", "Alice", "W", "alice2@example.com"));
        System.out.println(registerUser("charlie", "pw", "Char", "Lie", "charlie@example.com"));
        System.out.println(registerUser("bob", "other", "Bobby", "B", "bob@other.com"));
    }
}