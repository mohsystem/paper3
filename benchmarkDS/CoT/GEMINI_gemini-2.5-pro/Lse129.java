import java.util.Map;
import java.util.HashMap;

public class Lse129 {

    /**
     * Escapes HTML special characters in a string to prevent XSS.
     * @param str The string to escape.
     * @return The escaped string.
     */
    private static String escapeHtml(String str) {
        if (str == null) {
            return "";
        }
        return str.replace("&", "&amp;")
                  .replace("<", "&lt;")
                  .replace(">", "&gt;")
                  .replace("\"", "&quot;")
                  .replace("'", "&#39;");
    }

    /**
     * Renders a template from an environment with a name parameter.
     * This simulates a web framework's template rendering.
     * @param env A map representing the environment, expected to contain a "template"
     *            and a "request" which in turn contains "params" with a "name".
     * @return A response string with the rendered template.
     */
    public static String render_response_from_env(Map<String, Object> env) {
        // Default to a guest user if name is not provided
        String name = "Guest";
        try {
            // Safely get the nested name parameter
            Map<String, Object> request = (Map<String, Object>) env.get("request");
            if (request != null) {
                Map<String, String> params = (Map<String, String>) request.get("params");
                if (params != null && params.containsKey("name")) {
                    String providedName = params.get("name");
                    if (providedName != null && !providedName.trim().isEmpty()) {
                        name = providedName;
                    }
                }
            }
        } catch (ClassCastException e) {
            // Handle cases where the env structure is incorrect
            System.err.println("Warning: Invalid environment structure. Using default name.");
        }

        // Get the template from the environment
        String template = (String) env.getOrDefault("template", "<h1>Hello, {{ name }}!</h1>");

        // SECURITY: Escape the user-provided name to prevent XSS attacks
        String safeName = escapeHtml(name);

        // Render the template by replacing the placeholder
        String renderedTemplate = template.replace("{{ name }}", safeName);

        // Return a response with the rendered template
        // In a real web app, this would be an HTTP Response object
        return renderedTemplate;
    }

    /**
     * Simulates a "home" endpoint controller.
     * @param env The environment for the request.
     * @return The response generated from the environment.
     */
    public static String home(Map<String, Object> env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        // --- Test Cases ---
        String template = "<h1>Hello, {{ name }}!</h1>";

        // Test Case 1: Normal name
        Map<String, Object> env1 = new HashMap<>();
        Map<String, Object> request1 = new HashMap<>();
        Map<String, String> params1 = new HashMap<>();
        params1.put("name", "Alice");
        request1.put("params", params1);
        env1.put("request", request1);
        env1.put("template", template);
        System.out.println("Test Case 1 (Normal): " + home(env1));

        // Test Case 2: XSS attack attempt
        Map<String, Object> env2 = new HashMap<>();
        Map<String, Object> request2 = new HashMap<>();
        Map<String, String> params2 = new HashMap<>();
        params2.put("name", "<script>alert('XSS')</script>");
        request2.put("params", params2);
        env2.put("request", request2);
        env2.put("template", template);
        System.out.println("Test Case 2 (XSS): " + home(env2));

        // Test Case 3: Empty name parameter
        Map<String, Object> env3 = new HashMap<>();
        Map<String, Object> request3 = new HashMap<>();
        Map<String, String> params3 = new HashMap<>();
        params3.put("name", "");
        request3.put("params", params3);
        env3.put("request", request3);
        env3.put("template", template);
        System.out.println("Test Case 3 (Empty): " + home(env3));

        // Test Case 4: Null name parameter
        Map<String, Object> env4 = new HashMap<>();
        Map<String, Object> request4 = new HashMap<>();
        Map<String, String> params4 = new HashMap<>();
        params4.put("name", null);
        request4.put("params", params4);
        env4.put("request", request4);
        env4.put("template", template);
        System.out.println("Test Case 4 (Null): " + home(env4));

        // Test Case 5: Missing 'name' key in params
        Map<String, Object> env5 = new HashMap<>();
        Map<String, Object> request5 = new HashMap<>();
        Map<String, String> params5 = new HashMap<>();
        // 'name' key is missing
        request5.put("params", params5);
        env5.put("request", request5);
        env5.put("template", template);
        System.out.println("Test Case 5 (Missing): " + home(env5));
    }
}