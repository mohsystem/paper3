
import java.util.HashMap;
import java.util.Map;

public class Lse135 {
    // Simulated user database
    private static final Map<String, String> userDatabase = new HashMap<>();
    
    static {
        // Initialize with some test users
        userDatabase.put("admin", "admin123");
        userDatabase.put("user1", "password1");
        userDatabase.put("moderator", "mod123");
        userDatabase.put("john", "doe456");
        userDatabase.put("alice", "wonderland");
    }
    
    /**
     * Checks if the provided username and password are correct
     * @param username The username from the form
     * @param password The password from the form
     * @return true if credentials are correct, false otherwise
     */
    public static boolean checkMod(String username, String password) {
        if (username == null || password == null) {
            return false;
        }
        
        String storedPassword = userDatabase.get(username);
        if (storedPassword == null) {
            return false;
        }
        
        return storedPassword.equals(password);
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid credentials
        System.out.println("Test 1 - Valid user (admin/admin123): " + checkMod("admin", "admin123"));
        
        // Test case 2: Valid credentials
        System.out.println("Test 2 - Valid user (user1/password1): " + checkMod("user1", "password1"));
        
        // Test case 3: Invalid password
        System.out.println("Test 3 - Invalid password (admin/wrong): " + checkMod("admin", "wrong"));
        
        // Test case 4: Non-existent user
        System.out.println("Test 4 - Non-existent user (hacker/hack): " + checkMod("hacker", "hack"));
        
        // Test case 5: Null values
        System.out.println("Test 5 - Null values (null/null): " + checkMod(null, null));
    }
}
