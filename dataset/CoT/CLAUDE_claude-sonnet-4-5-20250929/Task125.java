
import java.util.*;
import java.util.regex.Pattern;

public class Task125 {
    private static final Map<String, Map<String, String>> USER_RESOURCES = new HashMap<>();
    private static final Pattern VALID_USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{3,20}$");
    private static final Pattern VALID_RESOURCE_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{1,50}$");
    
    static {
        // Initialize user resources
        Map<String, String> user1Resources = new HashMap<>();
        user1Resources.put("document1", "Content of Document 1 for User1");
        user1Resources.put("report2", "Annual Report Data for User1");
        USER_RESOURCES.put("user1", user1Resources);
        
        Map<String, String> user2Resources = new HashMap<>();
        user2Resources.put("file3", "Private File Content for User2");
        user2Resources.put("data4", "Database Export for User2");
        USER_RESOURCES.put("user2", user2Resources);
        
        Map<String, String> adminResources = new HashMap<>();
        adminResources.put("config", "System Configuration Data");
        adminResources.put("logs", "System Logs and Monitoring");
        USER_RESOURCES.put("admin", adminResources);
    }
    
    public static String accessResource(String username, String resourceName) {
        // Input validation
        if (username == null || resourceName == null) {
            return "Error: Invalid input - null values not allowed";
        }
        
        // Sanitize and validate username
        username = username.trim();
        if (!VALID_USERNAME_PATTERN.matcher(username).matches()) {
            return "Error: Invalid username format";
        }
        
        // Sanitize and validate resource name
        resourceName = resourceName.trim();
        if (!VALID_RESOURCE_PATTERN.matcher(resourceName).matches()) {
            return "Error: Invalid resource name format";
        }
        
        // Check if user exists
        if (!USER_RESOURCES.containsKey(username)) {
            return "Error: User not found";
        }
        
        // Check if user has access to the resource
        Map<String, String> userResources = USER_RESOURCES.get(username);
        if (!userResources.containsKey(resourceName)) {
            return "Error: Access denied - Resource not found or unauthorized";
        }
        
        // Return the resource content
        return "Success: " + userResources.get(resourceName);
    }
    
    public static void main(String[] args) {
        System.out.println("=== Secure Resource Access Control System ===\\n");
        
        // Test Case 1: Valid access
        System.out.println("Test 1 - Valid Access:");
        System.out.println(accessResource("user1", "document1"));
        System.out.println();
        
        // Test Case 2: Valid access different user
        System.out.println("Test 2 - Valid Access (User2):");
        System.out.println(accessResource("user2", "file3"));
        System.out.println();
        
        // Test Case 3: Unauthorized access attempt
        System.out.println("Test 3 - Unauthorized Access:");
        System.out.println(accessResource("user1", "file3"));
        System.out.println();
        
        // Test Case 4: Path traversal attempt
        System.out.println("Test 4 - Path Traversal Attack Attempt:");
        System.out.println(accessResource("user1", "../admin/config"));
        System.out.println();
        
        // Test Case 5: Null input handling
        System.out.println("Test 5 - Null Input:");
        System.out.println(accessResource(null, "document1"));
        System.out.println();
    }
}
