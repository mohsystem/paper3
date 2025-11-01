import java.util.HashMap;
import java.util.Map;

class User {
    String password;
    boolean isLoggedIn;

    public User(String password) {
        this.password = password;
        this.isLoggedIn = false;
    }
}

public class Lse57 {
    private static Map<String, User> userDatabase = new HashMap<>();

    public static void addUser(String email, String password) {
        userDatabase.put(email, new User(password));
    }

    public static boolean login(String email, String password) {
        User user = userDatabase.get(email);
        if (user != null && user.password.equals(password)) {
            user.isLoggedIn = true;
            return true;
        }
        return false;
    }

    public static void logout(String email) {
        User user = userDatabase.get(email);
        if (user != null) {
            user.isLoggedIn = false;
        }
    }

    public static String changeEmail(String oldEmail, String newEmail, String confirmPassword) {
        User user = userDatabase.get(oldEmail);

        if (user == null) {
            return "Error: Old email does not exist.";
        }

        if (!user.isLoggedIn) {
            return "Error: User must be logged in to change email.";
        }

        if (!user.password.equals(confirmPassword)) {
            return "Error: Incorrect password.";
        }
        
        if (userDatabase.containsKey(newEmail)) {
            return "Error: New email is already taken.";
        }

        userDatabase.put(newEmail, user);
        userDatabase.remove(oldEmail);
        return "Success: Email changed from " + oldEmail + " to " + newEmail + ".";
    }

    public static void main(String[] args) {
        // Setup: Add some users to our database
        addUser("user1@example.com", "password123");
        addUser("user2@example.com", "securepass456");

        System.out.println("--- Test Case 1: Successful Email Change ---");
        login("user1@example.com", "password123");
        System.out.println("Login status for user1: " + userDatabase.get("user1@example.com").isLoggedIn);
        String result1 = changeEmail("user1@example.com", "new.user1@example.com", "password123");
        System.out.println(result1);
        System.out.println("Database contains new.user1@example.com: " + userDatabase.containsKey("new.user1@example.com"));
        System.out.println("Database contains user1@example.com: " + userDatabase.containsKey("user1@example.com"));
        System.out.println();

        System.out.println("--- Test Case 2: Failed Change (Incorrect Password) ---");
        login("user2@example.com", "securepass456");
        String result2 = changeEmail("user2@example.com", "another.user@example.com", "wrongpassword");
        System.out.println(result2);
        System.out.println();

        System.out.println("--- Test Case 3: Failed Change (User Not Logged In) ---");
        logout("user2@example.com"); // Make sure user is logged out
        String result3 = changeEmail("user2@example.com", "new.user2@example.com", "securepass456");
        System.out.println(result3);
        System.out.println();

        System.out.println("--- Test Case 4: Failed Change (Old Email Not Found) ---");
        String result4 = changeEmail("nonexistent@example.com", "anything@example.com", "anypassword");
        System.out.println(result4);
        System.out.println();

        System.out.println("--- Test Case 5: Logout and then Fail Change ---");
        System.out.println("User 'user2@example.com' attempts another change after logging out.");
        logout("user2@example.com");
        System.out.println("Login status for user2: " + userDatabase.get("user2@example.com").isLoggedIn);
        String result5 = changeEmail("user2@example.com", "final.user2@example.com", "securepass456");
        System.out.println(result5);
        System.out.println();
    }
}