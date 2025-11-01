import java.util.Map;
import java.util.HashMap;

public class Lse130 {
    /**
     * Mock representation of a web request.
     */
    static class Request {
        private final Map<String, String> params;

        public Request(Map<String, String> params) {
            this.params = params != null ? new HashMap<>(params) : new HashMap<>();
        }

        public String getParameter(String key) {
            return params.get(key);
        }
    }

    /**
     * Mock representation of a template.
     */
    static class Template {
        private final String content;

        public Template(String content) {
            this.content = content;
        }

        public String getContent() {
            return this.content;
        }
    }

    /**
     * Mock representation of a web response.
     */
    static class Response {
        public final String body;

        public Response(String body) {
            this.body = body;
        }

        @Override
        public String toString() {
            return "Response{body='" + body + "'}";
        }
    }

    /**
     * Mock representation of the web environment containing the request and template.
     */
    static class Environment {
        public final Request request;
        public final Template template;

        public Environment(Request request, Template template) {
            this.request = request;
            this.template = template;
        }
    }

    /**
     * Escapes characters in a String that are sensitive in HTML.
     * This is a crucial security measure to prevent Cross-Site Scripting (XSS).
     * @param s The string to escape.
     * @return The escaped string.
     */
    public static String escapeHtml(String s) {
        if (s == null) {
            return "";
        }
        StringBuilder out = new StringBuilder(s.length());
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '&':
                    out.append("&amp;");
                    break;
                case '<':
                    out.append("&lt;");
                    break;
                case '>':
                    out.append("&gt;");
                    break;
                case '"':
                    out.append("&quot;");
                    break;
                case '\'':
                    out.append("&#39;");
                    break;
                default:
                    out.append(c);
                    break;
            }
        }
        return out.toString();
    }

    /**
     * Gets name from request, gets template from env, renders, and returns response.
     * @param env The environment containing the request and template.
     * @return A Response object with the rendered template.
     */
    public static Response render_response_from_env(Environment env) {
        if (env == null || env.request == null || env.template == null) {
            return new Response("Error: Invalid environment.");
        }

        String name = env.request.getParameter("name");

        // Rules#1: Ensure all input is validated.
        if (name == null || name.trim().isEmpty()) {
            name = "Guest";
        }

        String templateContent = env.template.getContent();
        if (templateContent == null) {
            return new Response("Error: Invalid template.");
        }
        
        // Rules#2: Sanitize by escaping before embedding in the template to prevent XSS.
        String escapedName = escapeHtml(name);

        String renderedTemplate = templateContent.replace("{{name}}", escapedName);
        return new Response(renderedTemplate);
    }

    /**
     * Home function that calls the render function.
     * @param env The environment variable.
     * @return The response from the render function.
     */
    public static Response home(Environment env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        Template template = new Template("<h1>Hello, {{name}}!</h1>");

        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Normal name
        runTest("Test 1: Normal name", new Environment(new Request(Map.of("name", "Alice")), template));

        // Test Case 2: Name with special characters that need escaping
        runTest("Test 2: Name with special characters", new Environment(new Request(Map.of("name", "Bob & Charlie")), template));

        // Test Case 3: A potential XSS payload
        runTest("Test 3: XSS payload", new Environment(new Request(Map.of("name", "<script>alert('xss')</script>")), template));

        // Test Case 4: Empty name
        runTest("Test 4: Empty name", new Environment(new Request(Map.of("name", "  ")), template));

        // Test Case 5: Null/missing name parameter
        runTest("Test 5: Null name parameter", new Environment(new Request(new HashMap<>()), template));
    }

    private static void runTest(String testName, Environment env) {
        System.out.println(testName);
        Response res = home(env);
        System.out.println("  Output: " + res.body);
        System.out.println();
    }
}