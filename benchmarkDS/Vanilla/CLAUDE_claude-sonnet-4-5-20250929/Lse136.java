
import java.util.HashMap;
import java.util.Map;

public class Lse136 {
    private static final Map<String, String> validCredentials = new HashMap<>();
    
    static {
        validCredentials.put("admin", "admin123");
        validCredentials.put("user1", "password1");
        validCredentials.put("moderator", "mod123");
        validCredentials.put("testuser", "test456");
        validCredentials.put("john", "john789");
    }
    
    public static String checkMod(String username, String password) {
        if (username == null || password == null) {
            return "false";
        }
        
        if (validCredentials.containsKey(username) && 
            validCredentials.get(username).equals(password)) {
            return "true";
        }
        
        return "false";
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid credentials - admin
        System.out.println("Test 1: " + checkMod("admin", "admin123"));
        
        // Test case 2: Valid credentials - user1
        System.out.println("Test 2: " + checkMod("user1", "password1"));
        
        // Test case 3: Invalid password
        System.out.println("Test 3: " + checkMod("admin", "wrongpassword"));
        
        // Test case 4: Invalid username
        System.out.println("Test 4: " + checkMod("nonexistent", "password"));
        
        // Test case 5: Null values
        System.out.println("Test 5: " + checkMod(null, null));
    }
}
