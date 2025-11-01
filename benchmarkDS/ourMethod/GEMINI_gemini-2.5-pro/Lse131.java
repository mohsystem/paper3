import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

class Lse131 {

    // Represents a user request
    static class UserRequest {
        private final Map<String, String> parameters;

        public UserRequest() {
            this.parameters = new HashMap<>();
        }

        public void setParameter(String key, String value) {
            this.parameters.put(key, value);
        }

        public String getParameter(String key) {
            return this.parameters.get(key);
        }
    }

    // Represents a response to be sent
    static class Response {
        private String body;

        public String getBody() {
            return body;
        }

        public void setBody(String body) {
            this.body = body;
        }

        @Override
        public String toString() {
            return "Response{body='" + body + "'}";
        }
    }

    // Represents the environment containing request and template
    static class Environment {
        private final UserRequest request;
        private final String template;

        public Environment(UserRequest request, String template) {
            this.request = request;
            this.template = template;
        }

        public UserRequest getRequest() {
            return request;
        }

        public String getTemplate() {
            return template;
        }
    }

    /**
     * Sanitizes a string for HTML output to prevent XSS.
     *
     * @param input The string to escape.
     * @return The escaped string.
     */
    public static String htmlEscape(String input) {
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
                    escaped.append("&#x27;");
                    break;
                default:
                    escaped.append(c);
                    break;
            }
        }
        return escaped.toString();
    }

    /**
     * Renders a response from a template and request found in the environment.
     *
     * @param env The environment containing the request and template.
     * @return A Response object with the rendered content.
     */
    public static Response render_response_from_env(Environment env) {
        // Get the name parameter from the user request. Default to "World" if not present.
        String name = env.getRequest().getParameter("name");
        if (name == null || name.isEmpty()) {
            name = "World";
        }

        // Sanitize the input to prevent XSS attacks.
        String sanitizedName = htmlEscape(name);

        // Get the template from the environment.
        String template = env.getTemplate();

        // Render the template with the sanitized name parameter.
        String renderedTemplate = template.replace("{{name}}", sanitizedName);

        // Return a response with the rendered template.
        Response response = new Response();
        response.setBody(renderedTemplate);
        return response;
    }

    /**
     * Main handler that simulates a web request.
     *
     * @param env The environment for the request.
     * @return The generated response.
     */
    public static Response home(Environment env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        String template = "<h1>Hello, {{name}}!</h1>";

        // Test Case 1: Normal name
        UserRequest request1 = new UserRequest();
        request1.setParameter("name", "Alice");
        Environment env1 = new Environment(request1, template);
        System.out.println("Test Case 1 (Normal): " + home(env1).getBody());

        // Test Case 2: XSS attack attempt
        UserRequest request2 = new UserRequest();
        request2.setParameter("name", "<script>alert('xss')</script>");
        Environment env2 = new Environment(request2, template);
        System.out.println("Test Case 2 (XSS): " + home(env2).getBody());

        // Test Case 3: Name with special HTML characters
        UserRequest request3 = new UserRequest();
        request3.setParameter("name", "Bob & \"Charlie\"");
        Environment env3 = new Environment(request3, template);
        System.out.println("Test Case 3 (Special Chars): " + home(env3).getBody());

        // Test Case 4: Empty name parameter
        UserRequest request4 = new UserRequest();
        request4.setParameter("name", "");
        Environment env4 = new Environment(request4, template);
        System.out.println("Test Case 4 (Empty Name): " + home(env4).getBody());
        
        // Test Case 5: Missing name parameter
        UserRequest request5 = new UserRequest();
        Environment env5 = new Environment(request5, template);
        System.out.println("Test Case 5 (Missing Name): " + home(env5).getBody());
    }
}