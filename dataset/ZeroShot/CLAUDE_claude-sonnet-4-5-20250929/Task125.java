
import java.util.*;
import java.util.regex.Pattern;

public class Task125 {
    private static final Set<String> ALLOWED_RESOURCES = new HashSet<>(Arrays.asList(
        "document1.txt", "document2.txt", "image1.jpg", "image2.jpg", "data.csv"
    ));
    
    private static final Pattern VALID_RESOURCE_PATTERN = Pattern.compile("^[a-zA-Z0-9._-]+$");
    
    public static String accessResource(String username, String resourceName) {
        // Input validation
        if (username == null || username.trim().isEmpty()) {
            return "Error: Invalid username";
        }
        
        if (resourceName == null || resourceName.trim().isEmpty()) {
            return "Error: Invalid resource name";
        }
        
        // Sanitize input - remove whitespace
        resourceName = resourceName.trim();
        
        // Validate resource name format (prevent path traversal)
        if (!VALID_RESOURCE_PATTERN.matcher(resourceName).matches()) {
            return "Error: Invalid resource name format";
        }
        
        // Check for path traversal attempts
        if (resourceName.contains("..") || resourceName.contains("/") || resourceName.contains("\\\\")) {\n            return "Error: Path traversal attempt detected";\n        }\n        \n        // Check if resource is in whitelist\n        if (!ALLOWED_RESOURCES.contains(resourceName)) {\n            return "Error: Access denied - resource not found or unauthorized";\n        }\n        \n        // Simulate resource access\n        return "Success: User '" + username + "' accessed resource '" + resourceName + "'";\n    }\n    \n    public static void main(String[] args) {\n        System.out.println("=== Resource Access Control System ===\
");\n        \n        // Test case 1: Valid access\n        System.out.println("Test 1 - Valid access:");\n        System.out.println(accessResource("john_doe", "document1.txt"));\n        System.out.println();\n        \n        // Test case 2: Valid access to different resource\n        System.out.println("Test 2 - Valid access to image:");\n        System.out.println(accessResource("jane_smith", "image1.jpg"));\n        System.out.println();\n        \n        // Test case 3: Attempt to access non-existent resource\n        System.out.println("Test 3 - Non-existent resource:");\n        System.out.println(accessResource("user1", "secret.txt"));\n        System.out.println();\n        \n        // Test case 4: Path traversal attempt\n        System.out.println("Test 4 - Path traversal attempt:");\n        System.out.println(accessResource("attacker", "../../../etc/passwd"));\n        System.out.println();\n        \n        // Test case 5: Invalid input\n        System.out.println("Test 5 - Invalid input:");\n        System.out.println(accessResource("", "document1.txt"));\n        System.out.println(accessResource("user2", ""));
        System.out.println();
    }
}
