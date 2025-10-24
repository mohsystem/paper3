import java.util.Map;
import java.util.HashMap;

public class Task125 {

    // A whitelist of allowed resources and their internal paths.
    // This prevents path traversal as user input is only used as a key.
    private static final Map<String, String> allowedResources = new HashMap<>();

    static {
        allowedResources.put("profile", "data/user_profile.dat");
        allowedResources.put("settings", "config/app_settings.json");
        allowedResources.put("dashboard", "pages/dashboard.html");
    }

    /**
     * Securely accesses a resource based on a predefined whitelist.
     *
     * @param resourceKey The user-provided key for the resource.
     * @return A string indicating the result of the access attempt.
     */
    public static String accessResource(String resourceKey) {
        // Input validation: Check for null or empty input.
        if (resourceKey == null || resourceKey.trim().isEmpty()) {
            return "Error: Resource key cannot be empty.";
        }

        // Securely check if the key exists in our whitelist map.
        if (allowedResources.containsKey(resourceKey)) {
            String resourcePath = allowedResources.get(resourceKey);
            // In a real application, you would now use 'resourcePath' to read the file.
            // For this example, we just simulate the access.
            return "Success: Accessing resource at " + resourcePath;
        } else {
            // Generic error message to avoid leaking information about valid resources.
            return "Error: Resource not found or access denied.";
        }
    }

    public static void main(String[] args) {
        System.out.println("Java Test Cases:");
        
        String[] testCases = {
            "profile",          // Valid case 1
            "settings",         // Valid case 2
            "logs",             // Invalid case
            "../etc/passwd",    // Path traversal attempt
            ""                  // Empty input
        };

        for (String test : testCases) {
            System.out.println("Input: \"" + test + "\" -> Output: " + accessResource(test));
        }
    }
}