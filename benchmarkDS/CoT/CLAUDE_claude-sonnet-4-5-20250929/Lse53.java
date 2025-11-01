
import java.util.*;

public class Lse53 {
    // Enum to represent user roles
    enum UserRole {
        GUEST,
        USER,
        ADMIN
    }
    
    // Class to represent a user session
    static class UserSession {
        private boolean isLoggedIn;
        private UserRole role;
        private String username;
        
        public UserSession(boolean isLoggedIn, UserRole role, String username) {
            this.isLoggedIn = isLoggedIn;
            this.role = role;
            this.username = username;
        }
        
        public boolean isLoggedIn() {
            return isLoggedIn;
        }
        
        public UserRole getRole() {
            return role;
        }
        
        public String getUsername() {
            return username;
        }
    }
    
    /**
     * Determines the redirect page based on user session
     * @param session The user session object
     * @return The page to redirect to
     */
    public static String determineRedirectPage(UserSession session) {
        // Input validation
        if (session == null) {
            return "/login";
        }
        
        // Check if user is not logged in
        if (!session.isLoggedIn()) {
            return "/login";
        }
        
        // Check if user is admin
        if (session.getRole() == UserRole.ADMIN) {
            return "/admin";
        }
        
        // Check if user is a regular user
        if (session.getRole() == UserRole.USER) {
            return "/user";
        }
        
        // Default fallback to login
        return "/login";
    }
    
    public static void main(String[] args) {
        // Test case 1: Not logged in
        UserSession session1 = new UserSession(false, UserRole.GUEST, null);
        System.out.println("Test 1 - Not logged in: " + determineRedirectPage(session1));
        
        // Test case 2: Logged in as regular user
        UserSession session2 = new UserSession(true, UserRole.USER, "john_doe");
        System.out.println("Test 2 - Regular user: " + determineRedirectPage(session2));
        
        // Test case 3: Logged in as admin
        UserSession session3 = new UserSession(true, UserRole.ADMIN, "admin_user");
        System.out.println("Test 3 - Admin user: " + determineRedirectPage(session3));
        
        // Test case 4: Null session
        UserSession session4 = null;
        System.out.println("Test 4 - Null session: " + determineRedirectPage(session4));
        
        // Test case 5: Logged in but with GUEST role
        UserSession session5 = new UserSession(true, UserRole.GUEST, "guest_user");
        System.out.println("Test 5 - Logged in guest: " + determineRedirectPage(session5));
    }
}
