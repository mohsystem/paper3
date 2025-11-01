import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

public class Lse129 {

    /**
     * Escapes special HTML characters in a string to prevent Cross-Site Scripting (XSS).
     *
     * @param input The string to escape.
     * @return The escaped string.
     */
    public static String htmlEscape(String input) {
        if (input == null || input.isEmpty()) {
            return "";
        }
        StringBuilder escaped = new StringBuilder();
        for (char c : input.toCharArray()) {
            switch (c) {
                case '&':
                    escaped.append("&amp;");
                    break;
                case '<':
                    escaped.append("&lt;");
                    break;
                case '>':
                    escaped.append("&gt;");
                    break;
                case '"':
                    escaped.append("&quot;");
                    break;
                case '\'':
                    escaped.append("&#39;");
                    break;
                default:
                    escaped.append(c);
                    break;
            }
        }
        return escaped.toString();
    }

    /**
     * Renders a template from an environment with a given name parameter from a request.
     *
     * @param env           A map representing the environment, containing the template.
     * @param requestParams A map representing request parameters, containing the name.
     * @return A string representing the rendered response.
     */
    public static String render_response_from_env(Map<String, String> env, Map<String, String> requestParams) {
        String name = requestParams.getOrDefault("name", "");
        String template = env.getOrDefault("template", "");

        // Secure: Escape the user-provided name to prevent XSS attacks.
        String escapedName = htmlEscape(name);

        // Render the template by replacing the placeholder with the escaped name.
        return template.replace("{{ name }}", escapedName);
    }

    /**
     * Simulates a home route handler that renders a response.
     *
     * @param env           The environment map.
     * @param requestParams The request parameters map.
     * @return The response from the renderer.
     */
    public static String home(Map<String, String> env, Map<String, String> requestParams) {
        return render_response_from_env(env, requestParams);
    }

    public static void main(String[] args) {
        // 1. Setup the environment with a template.
        Map<String, String> env = new HashMap<>();
        env.put("template", "<h1>Hello, {{ name }}!</h1>");

        // 2. Define test cases.
        List<Map<String, String>> testCases = new ArrayList<>();
        
        // Test case 1: Normal name
        testCases.add(Map.of("name", "World"));
        
        // Test case 2: Name with spaces
        testCases.add(Map.of("name", "Jane Doe"));
        
        // Test case 3: Empty name
        testCases.add(Map.of("name", ""));
        
        // Test case 4: Malicious script (XSS attempt)
        testCases.add(Map.of("name", "<script>alert('XSS')</script>"));

        // Test case 5: Name with HTML characters
        testCases.add(Map.of("name", "<b>Bold</b>"));

        // 3. Run test cases.
        for (int i = 0; i < testCases.size(); i++) {
            Map<String, String> request = testCases.get(i);
            String response = home(env, request);
            System.out.println("Test Case " + (i + 1) + " (name=" + request.get("name") + "):");
            System.out.println("Output: " + response);
            System.out.println();
        }
    }
}