public class Task125 {

    private static final java.util.Map<String, String> resources = new java.util.HashMap<>();

    static {
        resources.put("profile", "User Profile Data: { 'user': 'John Doe', 'email': 'john.doe@example.com' }");
        resources.put("settings", "Account Settings Page: [ 'Theme: Dark', 'Language: English' ]");
        resources.put("dashboard", "Main Dashboard View: Widgets and Stats");
        resources.put("logs", "System Activity Logs: [ 'INFO: User login', 'WARN: Low disk space' ]");
    }

    /**
     * Accesses a resource based on the provided resource name.
     * @param resourceName The name of the resource to access.
     * @return The resource content if found, otherwise an error message.
     */
    public static String getResource(String resourceName) {
        if (resourceName != null && resources.containsKey(resourceName.toLowerCase())) {
            return "Accessing " + resourceName + ": " + resources.get(resourceName.toLowerCase());
        } else {
            return "Access Denied: Resource '" + resourceName + "' not found.";
        }
    }

    public static void main(String[] args) {
        // Test Cases
        String[] testCases = {"profile", "dashboard", "admin", "settings", "data"};
        
        System.out.println("--- Java Test Cases ---");
        for (String testCase : testCases) {
            System.out.println("Input: " + testCase);
            String result = getResource(testCase);
            System.out.println("Output: " + result);
            System.out.println();
        }
    }
}