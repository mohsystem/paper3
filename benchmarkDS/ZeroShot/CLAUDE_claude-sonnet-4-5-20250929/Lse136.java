
import java.util.HashMap;
import java.util.Map;

public class Lse136 {
    private static final Map<String, String> validCredentials = new HashMap<>();
    
    static {
        // Test credentials
        validCredentials.put("admin", "admin123");
        validCredentials.put("user1", "pass1");
        validCredentials.put("user2", "pass2");
        validCredentials.put("testuser", "testpass");
        validCredentials.put("moderator", "modpass");
    }
    
    public static String checkMod(String username, String password) {
        if (username == null || password == null) {
            return "false";
        }
        
        // Check if username exists and password matches
        if (validCredentials.containsKey(username) && 
            validCredentials.get(username).equals(password)) {
            return "true";
        }
        
        return "false";
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid credentials
        System.out.println("Test 1: " + checkMod("admin", "admin123"));
        
        // Test case 2: Valid credentials
        System.out.println("Test 2: " + checkMod("user1", "pass1"));
        
        // Test case 3: Invalid password
        System.out.println("Test 3: " + checkMod("admin", "wrongpass"));
        
        // Test case 4: Invalid username
        System.out.println("Test 4: " + checkMod("invaliduser", "pass1"));
        
        // Test case 5: Null inputs
        System.out.println("Test 5: " + checkMod(null, null));
    }
}
