
import java.util.*;

public class Lse53 {
    // User class to represent a user with credentials and role
    static class User {
        private String username;
        private String password;
        private boolean isAdmin;
        
        public User(String username, String password, boolean isAdmin) {
            this.username = username;
            this.password = password;
            this.isAdmin = isAdmin;
        }
        
        public String getUsername() {
            return username;
        }
        
        public boolean checkPassword(String password) {
            return this.password.equals(password);
        }
        
        public boolean isAdmin() {
            return isAdmin;
        }
    }
    
    // Session class to manage user sessions
    static class Session {
        private User currentUser;
        
        public void login(User user) {
            this.currentUser = user;
        }
        
        public void logout() {
            this.currentUser = null;
        }
        
        public boolean isLoggedIn() {
            return currentUser != null;
        }
        
        public User getCurrentUser() {
            return currentUser;
        }
    }
    
    // Method to redirect user based on authentication and authorization
    public static String redirectUser(Session session) {
        if (!session.isLoggedIn()) {
            return "Redirect to: /login";
        }
        
        User user = session.getCurrentUser();
        if (user.isAdmin()) {
            return "Redirect to: /admin";
        } else {
            return "Redirect to: /user";
        }
    }
    
    // Helper method for authentication
    public static boolean authenticate(String username, String password, Map<String, User> userDatabase) {
        User user = userDatabase.get(username);
        if (user != null && user.checkPassword(password)) {
            return true;
        }
        return false;
    }
    
    public static void main(String[] args) {
        // Create user database
        Map<String, User> userDatabase = new HashMap<>();
        userDatabase.put("admin", new User("admin", "admin123", true));
        userDatabase.put("john", new User("john", "john123", false));
        userDatabase.put("jane", new User("jane", "jane123", false));
        
        Session session = new Session();
        
        // Test Case 1: Not logged in
        System.out.println("Test Case 1 - Not logged in:");
        System.out.println(redirectUser(session));
        System.out.println();
        
        // Test Case 2: Logged in as regular user (john)
        System.out.println("Test Case 2 - Logged in as regular user (john):");
        if (authenticate("john", "john123", userDatabase)) {
            session.login(userDatabase.get("john"));
        }
        System.out.println(redirectUser(session));
        session.logout();
        System.out.println();
        
        // Test Case 3: Logged in as admin
        System.out.println("Test Case 3 - Logged in as admin:");
        if (authenticate("admin", "admin123", userDatabase)) {
            session.login(userDatabase.get("admin"));
        }
        System.out.println(redirectUser(session));
        session.logout();
        System.out.println();
        
        // Test Case 4: Failed login attempt then checking redirect
        System.out.println("Test Case 4 - Failed login attempt:");
        if (authenticate("john", "wrongpassword", userDatabase)) {
            session.login(userDatabase.get("john"));
        }
        System.out.println(redirectUser(session));
        System.out.println();
        
        // Test Case 5: Logged in as another regular user (jane)
        System.out.println("Test Case 5 - Logged in as regular user (jane):");
        if (authenticate("jane", "jane123", userDatabase)) {
            session.login(userDatabase.get("jane"));
        }
        System.out.println(redirectUser(session));
        System.out.println();
    }
}
