import java.util.Set;
import java.util.HashSet;
import java.util.Arrays;

public class Task125 {

    private static final Set<String> ALLOWED_RESOURCES = new HashSet<>(Arrays.asList(
        "PROFILE", "DASHBOARD", "SETTINGS", "REPORTS"
    ));

    /**
     * Checks if a user has access to a specific resource based on a predefined list.
     * This function validates the input and compares it against a safe list of
     * allowed resources to prevent unauthorized access.
     *
     * @param resourceId The ID of the resource being requested. Must not be null or empty.
     * @return A string indicating whether access is granted or denied.
     */
    public static String getResourceAccess(String resourceId) {
        // Rule#6: Ensure all input is validated and sanitized.
        // Check for null or empty/whitespace-only strings.
        if (resourceId == null || resourceId.trim().isEmpty()) {
            return "Access Denied: Invalid resource ID.";
        }
        
        // The check against a predefined set acts as a form of sanitization.
        if (ALLOWED_RESOURCES.contains(resourceId)) {
            // Use safe string formatting to construct the output message.
            return String.format("Access Granted to resource: %s", resourceId);
        } else {
            // Return a generic error message to avoid disclosing information
            // about which resources exist or don't exist.
            return "Access Denied: Resource not found or permission denied.";
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: A valid, allowed resource.
        String resource1 = "DASHBOARD";
        System.out.printf("Requesting '%s': %s%n", resource1, getResourceAccess(resource1));

        // Test Case 2: Another valid, allowed resource.
        String resource2 = "SETTINGS";
        System.out.printf("Requesting '%s': %s%n", resource2, getResourceAccess(resource2));

        // Test Case 3: An invalid/unauthorized resource.
        String resource3 = "ADMIN_PANEL";
        System.out.printf("Requesting '%s': %s%n", resource3, getResourceAccess(resource3));

        // Test Case 4: An empty string input.
        String resource4 = "";
        System.out.printf("Requesting '%s': %s%n", "empty string", getResourceAccess(resource4));

        // Test Case 5: A null input.
        String resource5 = null;
        System.out.printf("Requesting null: %s%n", getResourceAccess(resource5));
        
        System.out.println("--- Test Cases Finished ---");
    }
}