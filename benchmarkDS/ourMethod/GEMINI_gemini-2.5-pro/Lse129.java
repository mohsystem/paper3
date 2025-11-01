import java.util.Map;
import java.util.HashMap;

public class Lse129 {

    /**
     * A utility method to escape HTML special characters.
     * This is crucial to prevent Cross-Site Scripting (XSS) attacks.
     * @param data The string to escape.
     * @return The escaped string.
     */
    public static String htmlEscape(String data) {
        if (data == null) {
            return "";
        }
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < data.length(); i++) {
            char c = data.charAt(i);
            switch (c) {
                case '&':
                    sb.append("&amp;");
                    break;
                case '<':
                    sb.append("&lt;");
                    break;
                case '>':
                    sb.append("&gt;");
                    break;
                case '"':
                    sb.append("&quot;");
                    break;
                case '\'':
                    sb.append("&#x27;"); // &apos; is not recommended in HTML4
                    break;
                default:
                    sb.append(c);
                    break;
            }
        }
        return sb.toString();
    }

    /**
     * Renders a response from a template and request data.
     * @param env A map representing the environment, containing templates.
     * @param request A map representing the incoming request parameters.
     * @return A string representing the rendered response.
     */
    public static String renderResponseFromEnv(Map<String, String> env, Map<String, String> request) {
        // Get the template from the environment
        String template = env.get("template");
        if (template == null) {
            return "Error: Template not found.";
        }

        // Get the name parameter from the request, with a default value
        String name = request.getOrDefault("name", "Guest");

        // Sanitize input to prevent XSS
        String sanitizedName = htmlEscape(name);

        // Render the template with the sanitized parameter
        return template.replace("{{name}}", sanitizedName);
    }

    /**
     * A home function that simulates a controller action.
     * It calls renderResponseFromEnv and returns its result.
     * @param env A map representing the environment.
     * @param request A map representing the request.
     * @return The response from the renderer.
     */
    public static String home(Map<String, String> env, Map<String, String> request) {
        return renderResponseFromEnv(env, request);
    }

    public static void main(String[] args) {
        // Setup a mock environment with a template
        Map<String, String> env = new HashMap<>();
        env.put("template", "<h1>Hello, {{name}}!</h1>");

        // --- Test Cases ---
        System.out.println("--- 5 Test Cases ---");

        // Test Case 1: Normal name
        Map<String, String> request1 = new HashMap<>();
        request1.put("name", "Alice");
        System.out.println("Test 1 (Normal): " + home(env, request1));

        // Test Case 2: Name with HTML characters (potential XSS)
        Map<String, String> request2 = new HashMap<>();
        request2.put("name", "<script>alert('xss')</script>");
        System.out.println("Test 2 (XSS): " + home(env, request2));

        // Test Case 3: Empty name parameter
        Map<String, String> request3 = new HashMap<>();
        request3.put("name", "");
        System.out.println("Test 3 (Empty): " + home(env, request3));

        // Test Case 4: Missing name parameter
        Map<String, String> request4 = new HashMap<>();
        System.out.println("Test 4 (Missing): " + home(env, request4));

        // Test Case 5: Name with other special characters
        Map<String, String> request5 = new HashMap<>();
        request5.put("name", "Bob & Charlie's \"Shop\"");
        System.out.println("Test 5 (Special Chars): " + home(env, request5));
    }
}