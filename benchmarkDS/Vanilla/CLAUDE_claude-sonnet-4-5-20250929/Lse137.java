
import java.util.*;

public class Lse137 {
    // Function to check if user is a moderator
    public static boolean authMod(String username, String password, String session) {
        // Sample moderator credentials (in real application, this would be from a database)
        Map<String, String> moderators = new HashMap<>();
        moderators.put("admin", "admin123");
        moderators.put("mod1", "modpass1");
        moderators.put("mod2", "modpass2");
        
        // Sample valid sessions (in real application, this would be validated from session store)
        Set<String> validSessions = new HashSet<>(Arrays.asList("session123", "session456", "session789"));
        
        // Check if username exists, password matches, and session is valid
        if (moderators.containsKey(username) && 
            moderators.get(username).equals(password) && 
            validSessions.contains(session)) {
            return true;
        }
        return false;
    }
    
    // Function that would be called when visiting /check_mod URL
    public static boolean checkMod(String username, String password, String session) {
        return authMod(username, password, session);
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid moderator credentials and session
        System.out.println("Test 1: " + checkMod("admin", "admin123", "session123"));
        
        // Test case 2: Valid moderator but invalid session
        System.out.println("Test 2: " + checkMod("mod1", "modpass1", "invalidsession"));
        
        // Test case 3: Invalid username
        System.out.println("Test 3: " + checkMod("user1", "password", "session456"));
        
        // Test case 4: Valid username but wrong password
        System.out.println("Test 4: " + checkMod("mod2", "wrongpass", "session789"));
        
        // Test case 5: Another valid moderator with valid session
        System.out.println("Test 5: " + checkMod("mod2", "modpass2", "session456"));
    }
}
