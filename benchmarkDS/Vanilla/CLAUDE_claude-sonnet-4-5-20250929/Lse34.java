
import java.sql.*;
import java.util.HashMap;
import java.util.Map;

public class Lse34 {
    private static Map<String, String> database = new HashMap<>();
    
    static {
        // Initialize mock database with test users
        database.put("admin", "admin123");
        database.put("user1", "password1");
        database.put("user2", "password2");
        database.put("testuser", "testpass");
        database.put("john", "doe123");
    }
    
    public static String login(String username, String password) {
        if (username == null || password == null) {
            return "login";
        }
        
        if (database.containsKey(username) && database.get(username).equals(password)) {
            return "dashboard";
        } else {
            return "login";
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid credentials
        System.out.println("Test 1 - Valid user (admin/admin123): " + login("admin", "admin123"));
        
        // Test case 2: Invalid password
        System.out.println("Test 2 - Invalid password (admin/wrongpass): " + login("admin", "wrongpass"));
        
        // Test case 3: Non-existent user
        System.out.println("Test 3 - Non-existent user (unknown/pass): " + login("unknown", "pass"));
        
        // Test case 4: Valid credentials (user1)
        System.out.println("Test 4 - Valid user (user1/password1): " + login("user1", "password1"));
        
        // Test case 5: Null credentials
        System.out.println("Test 5 - Null credentials: " + login(null, null));
    }
}
