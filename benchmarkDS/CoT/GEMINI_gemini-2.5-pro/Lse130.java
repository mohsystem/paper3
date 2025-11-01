import java.util.Map;
import java.util.HashMap;

public class Lse130 {

    /**
     * Escapes characters in a String to be safe for use in HTML, preventing XSS.
     * @param input The string to escape.
     * @return The escaped string.
     */
    private static String htmlEscape(String input) {
        if (input == null) {
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
            }
        }
        return escaped.toString();
    }

    /**
     * Renders a response from an environment object. It gets a name from the request,
     * securely renders it into a template, and returns an HTTP-like response.
     * @param env A map representing the environment, expected to contain "request" and "template".
     * @return A string representing the rendered response.
     */
    @SuppressWarnings("unchecked")
    public static String renderResponseFromEnv(Map<String, Object> env) {
        if (env == null || !env.containsKey("request") || !env.containsKey("template")) {
            return "Error: Invalid environment.";
        }
        Map<String, String> request = (Map<String, String>) env.get("request");
        String template = (String) env.get("template");

        if (request == null || template == null) {
            return "Error: Invalid environment components.";
        }

        // Get the name parameter from the request, default to "Guest"
        String name = request.getOrDefault("name", "Guest");

        // Securely escape the user-provided name to prevent XSS
        String safeName = htmlEscape(name);

        // Render the template with the sanitized name
        String renderedTemplate = template.replace("{name}", safeName);

        // Return a "response" with the rendered template
        return "HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + renderedTemplate;
    }

    /**
     * Home function that calls renderResponseFromEnv.
     * @param env A map representing the environment.
     * @return The response from renderResponseFromEnv.
     */
    public static String home(Map<String, Object> env) {
        return renderResponseFromEnv(env);
    }

    public static void main(String[] args) {
        String template = "<h1>Hello, {name}!</h1>";

        // Test Case 1: Simple name
        Map<String, Object> env1 = new HashMap<>();
        Map<String, String> req1 = new HashMap<>();
        req1.put("name", "Alice");
        env1.put("request", req1);
        env1.put("template", template);
        System.out.println("--- Test Case 1 ---");
        System.out.println(home(env1));
        System.out.println();

        // Test Case 2: Name with a space
        Map<String, Object> env2 = new HashMap<>();
        Map<String, String> req2 = new HashMap<>();
        req2.put("name", "Bob Smith");
        env2.put("request", req2);
        env2.put("template", template);
        System.out.println("--- Test Case 2 ---");
        System.out.println(home(env2));
        System.out.println();
        
        // Test Case 3: Name with characters that need escaping
        Map<String, Object> env3 = new HashMap<>();
        Map<String, String> req3 = new HashMap<>();
        req3.put("name", "O'Malley & Sons");
        env3.put("request", req3);
        env3.put("template", template);
        System.out.println("--- Test Case 3 ---");
        System.out.println(home(env3));
        System.out.println();

        // Test Case 4: XSS attack attempt
        Map<String, Object> env4 = new HashMap<>();
        Map<String, String> req4 = new HashMap<>();
        req4.put("name", "<script>alert('pwned')</script>");
        env4.put("request", req4);
        env4.put("template", template);
        System.out.println("--- Test Case 4 (XSS Prevention) ---");
        System.out.println(home(env4));
        System.out.println();
        
        // Test Case 5: Missing name parameter
        Map<String, Object> env5 = new HashMap<>();
        Map<String, String> req5 = new HashMap<>();
        // No "name" parameter here
        env5.put("request", req5);
        env5.put("template", template);
        System.out.println("--- Test Case 5 (Default Name) ---");
        System.out.println(home(env5));
        System.out.println();
    }
}