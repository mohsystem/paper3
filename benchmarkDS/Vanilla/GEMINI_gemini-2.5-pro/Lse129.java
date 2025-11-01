import java.util.Map;
import java.util.HashMap;

public class Lse129 {

    /**
     * Renders a template from an environment map.
     * @param env A map representing the environment. It should contain a "template" string
     *            and a "request" map. The "request" map should contain a "name" parameter.
     * @return The rendered string as a response.
     */
    public static String render_response_from_env(Map<String, Object> env) {
        // The function gets the name parameter from the request.
        @SuppressWarnings("unchecked")
        Map<String, String> request = (Map<String, String>) env.get("request");
        String name = request.get("name");

        // The function gets the template from the environment.
        String template = (String) env.get("template");

        // The function renders the template with the name parameter.
        String renderedTemplate = template.replace("{{name}}", name);

        // The function returns a response with the rendered template.
        return renderedTemplate;
    }

    /**
     * Calls the render_response_from_env function.
     * @param env The environment map.
     * @return The response from the render function.
     */
    public static String home(Map<String, Object> env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        // Test cases
        Map<String, Object>[] testCases = new HashMap[5];

        // Test Case 1
        testCases[0] = new HashMap<>();
        Map<String, String> request1 = new HashMap<>();
        request1.put("name", "Alice");
        testCases[0].put("request", request1);
        testCases[0].put("template", "Hello, {{name}}! Welcome to our site.");

        // Test Case 2
        testCases[1] = new HashMap<>();
        Map<String, String> request2 = new HashMap<>();
        request2.put("name", "Bob");
        testCases[1].put("request", request2);
        testCases[1].put("template", "Greetings, {{name}}!");

        // Test Case 3
        testCases[2] = new HashMap<>();
        Map<String, String> request3 = new HashMap<>();
        request3.put("name", "Charlie");
        testCases[2].put("request", request3);
        testCases[2].put("template", "Dear {{name}}, your profile is updated.");

        // Test Case 4: Name with spaces
        testCases[3] = new HashMap<>();
        Map<String, String> request4 = new HashMap<>();
        request4.put("name", "David Smith");
        testCases[3].put("request", request4);
        testCases[3].put("template", "User: {{name}}");

        // Test Case 5: Empty name
        testCases[4] = new HashMap<>();
        Map<String, String> request5 = new HashMap<>();
        request5.put("name", "");
        testCases[4].put("request", request5);
        testCases[4].put("template", "Hello, {{name}}! You are a guest.");

        for (int i = 0; i < testCases.length; i++) {
            System.out.println("Test Case " + (i + 1) + ":");
            String response = home(testCases[i]);
            System.out.println("  Input Env: " + testCases[i]);
            System.out.println("  Response: " + response);
            System.out.println();
        }
    }
}