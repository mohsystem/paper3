
import java.util.HashMap;
import java.util.Map;

public class Lse41 {
    // Mock user data
    private static Map<String, UserInfo> userDatabase = new HashMap<>();
    
    static class UserInfo {
        String username;
        String email;
        String ssn; // Full SSN (should be stored securely in real applications)
        
        public UserInfo(String username, String email, String ssn) {
            this.username = username;
            this.email = email;
            this.ssn = ssn;
        }
        
        public String getLast4SSN() {
            if (ssn != null && ssn.length() >= 4) {
                return ssn.substring(ssn.length() - 4);
            }
            return "****";
        }
    }
    
    // Simulate /info endpoint
    public static String getInfoPage(String userId) {
        UserInfo user = userDatabase.get(userId);
        if (user == null) {
            return "User not found";
        }
        
        StringBuilder response = new StringBuilder();
        response.append("User Information\\n");
        response.append("================\\n");
        response.append("Username: ").append(user.username).append("\\n");
        response.append("Email: ").append(user.email).append("\\n");
        response.append("SSN (Last 4): ").append(user.getLast4SSN()).append("\\n");
        
        return response.toString();
    }
    
    public static void main(String[] args) {
        // Initialize test data
        userDatabase.put("user1", new UserInfo("john_doe", "john@example.com", "123456789"));
        userDatabase.put("user2", new UserInfo("jane_smith", "jane@example.com", "987654321"));
        userDatabase.put("user3", new UserInfo("bob_wilson", "bob@example.com", "555443333"));
        userDatabase.put("user4", new UserInfo("alice_brown", "alice@example.com", "111222333"));
        userDatabase.put("user5", new UserInfo("charlie_davis", "charlie@example.com", "999888777"));
        
        // Test cases
        System.out.println("Test Case 1:");
        System.out.println(getInfoPage("user1"));
        System.out.println();
        
        System.out.println("Test Case 2:");
        System.out.println(getInfoPage("user2"));
        System.out.println();
        
        System.out.println("Test Case 3:");
        System.out.println(getInfoPage("user3"));
        System.out.println();
        
        System.out.println("Test Case 4:");
        System.out.println(getInfoPage("user4"));
        System.out.println();
        
        System.out.println("Test Case 5:");
        System.out.println(getInfoPage("user5"));
    }
}
