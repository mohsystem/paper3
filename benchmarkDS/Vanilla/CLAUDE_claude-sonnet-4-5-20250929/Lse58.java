
import java.util.*;

public class Lse58 {
    private static class User {
        private String username;
        private String email;
        private String password;
        
        public User(String username, String email, String password) {
            this.username = username;
            this.email = email;
            this.password = password;
        }
        
        public String getEmail() {
            return email;
        }
        
        public void setEmail(String email) {
            this.email = email;
        }
        
        public String getPassword() {
            return password;
        }
        
        public String getUsername() {
            return username;
        }
    }
    
    private static Map<String, User> users = new HashMap<>();
    private static User loggedInUser = null;
    
    public static boolean register(String username, String email, String password) {
        if (users.containsKey(username)) {
            return false;
        }
        users.put(username, new User(username, email, password));
        return true;
    }
    
    public static boolean login(String username, String password) {
        User user = users.get(username);
        if (user != null && user.getPassword().equals(password)) {
            loggedInUser = user;
            return true;
        }
        return false;
    }
    
    public static void logout() {
        loggedInUser = null;
    }
    
    public static boolean changeEmail(String oldEmail, String newEmail, String password) {
        if (loggedInUser == null) {
            return false;
        }
        
        if (loggedInUser.getEmail().equals(oldEmail) && 
            loggedInUser.getPassword().equals(password)) {
            loggedInUser.setEmail(newEmail);
            return true;
        }
        return false;
    }
    
    public static String getCurrentEmail() {
        if (loggedInUser != null) {
            return loggedInUser.getEmail();
        }
        return null;
    }
    
    public static void main(String[] args) {
        // Test Case 1: Register and login, then change email successfully
        System.out.println("Test Case 1:");
        register("user1", "user1@example.com", "password123");
        login("user1", "password123");
        boolean result1 = changeEmail("user1@example.com", "newemail1@example.com", "password123");
        System.out.println("Change email result: " + result1 + ", New email: " + getCurrentEmail());
        logout();
        
        // Test Case 2: Try to change email without logging in
        System.out.println("\\nTest Case 2:");
        boolean result2 = changeEmail("user1@example.com", "newemail2@example.com", "password123");
        System.out.println("Change email without login: " + result2);
        
        // Test Case 3: Login but provide wrong old email
        System.out.println("\\nTest Case 3:");
        login("user1", "password123");
        boolean result3 = changeEmail("wrong@example.com", "newemail3@example.com", "password123");
        System.out.println("Change email with wrong old email: " + result3 + ", Current email: " + getCurrentEmail());
        logout();
        
        // Test Case 4: Login but provide wrong password
        System.out.println("\\nTest Case 4:");
        login("user1", "password123");
        boolean result4 = changeEmail("newemail1@example.com", "newemail4@example.com", "wrongpassword");
        System.out.println("Change email with wrong password: " + result4 + ", Current email: " + getCurrentEmail());
        logout();
        
        // Test Case 5: Successful email change with correct credentials
        System.out.println("\\nTest Case 5:");
        register("user2", "user2@example.com", "pass456");
        login("user2", "pass456");
        boolean result5 = changeEmail("user2@example.com", "user2new@example.com", "pass456");
        System.out.println("Change email result: " + result5 + ", New email: " + getCurrentEmail());
        logout();
    }
}
