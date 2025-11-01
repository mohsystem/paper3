
import java.util.*;

public class Lse56 {
    private static Map<String, User> users = new HashMap<>();
    private static String currentLoggedInUser = null;
    
    static class User {
        String username;
        String email;
        String password;
        
        User(String username, String email, String password) {
            this.username = username;
            this.email = email;
            this.password = password;
        }
    }
    
    public static boolean login(String username, String password) {
        if (users.containsKey(username)) {
            User user = users.get(username);
            if (user.password.equals(password)) {
                currentLoggedInUser = username;
                return true;
            }
        }
        return false;
    }
    
    public static void logout() {
        currentLoggedInUser = null;
    }
    
    public static boolean registerUser(String username, String email, String password) {
        if (!users.containsKey(username)) {
            users.put(username, new User(username, email, password));
            return true;
        }
        return false;
    }
    
    public static String changeEmail(String oldEmail, String newEmail, String confirmPassword) {
        // Check if user is logged in
        if (currentLoggedInUser == null) {
            return "Error: User must be logged in to change email";
        }
        
        User user = users.get(currentLoggedInUser);
        
        // Check if old email matches
        if (!user.email.equals(oldEmail)) {
            return "Error: Old email does not match";
        }
        
        // Check if password is correct
        if (!user.password.equals(confirmPassword)) {
            return "Error: Incorrect password";
        }
        
        // Check if new email is different from old email
        if (oldEmail.equals(newEmail)) {
            return "Error: New email must be different from old email";
        }
        
        // Change email
        user.email = newEmail;
        return "Success: Email changed successfully";
    }
    
    public static void main(String[] args) {
        // Test Case 1: Change email successfully
        System.out.println("Test Case 1:");
        registerUser("user1", "old@example.com", "password123");
        login("user1", "password123");
        System.out.println(changeEmail("old@example.com", "new@example.com", "password123"));
        logout();
        System.out.println();
        
        // Test Case 2: Try to change email without logging in
        System.out.println("Test Case 2:");
        System.out.println(changeEmail("old@example.com", "new2@example.com", "password123"));
        System.out.println();
        
        // Test Case 3: Wrong old email
        System.out.println("Test Case 3:");
        registerUser("user2", "user2@example.com", "pass456");
        login("user2", "pass456");
        System.out.println(changeEmail("wrong@example.com", "new@example.com", "pass456"));
        logout();
        System.out.println();
        
        // Test Case 4: Wrong password
        System.out.println("Test Case 4:");
        registerUser("user3", "user3@example.com", "mypass");
        login("user3", "mypass");
        System.out.println(changeEmail("user3@example.com", "newuser3@example.com", "wrongpass"));
        logout();
        System.out.println();
        
        // Test Case 5: New email same as old email
        System.out.println("Test Case 5:");
        registerUser("user4", "user4@example.com", "secure123");
        login("user4", "secure123");
        System.out.println(changeEmail("user4@example.com", "user4@example.com", "secure123"));
        logout();
    }
}
