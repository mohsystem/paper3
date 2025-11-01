import java.util.HashMap;
import java.util.Map;

// Main class as per instruction
public class Lse131 {

    // Simulates an environment that holds templates
    static class Environment {
        private String template;

        public Environment(String template) {
            this.template = template;
        }

        public String getTemplate() {
            return this.template;
        }
    }

    // Simulates a user request containing parameters
    static class UserRequest {
        private Map<String, String> parameters = new HashMap<>();

        public void setParameter(String key, String value) {
            this.parameters.put(key, value);
        }

        public String getParameter(String key) {
            // In a real application, handle potential null values gracefully.
            return this.parameters.getOrDefault(key, "Guest");
        }
    }

    // Simulates a response to be sent back
    static class Response {
        private String body;

        public Response(String body) {
            this.body = body;
        }

        public String getBody() {
            return this.body;
        }
    }

    /**
     * Renders a response from a template in the environment using parameters from a request.
     *
     * @param env     The environment containing the template.
     * @param request The user request containing parameters.
     * @return A Response object with the rendered template.
     */
    public static Response render_response_from_env(Environment env, UserRequest request) {
        // 1. Get the name parameter from the user request.
        // Sanitizing user input is crucial in real web applications to prevent XSS.
        String name = request.getParameter("name");

        // 2. Get the template from the environment.
        String template = env.getTemplate();

        // 3. Render the template with the name parameter.
        // Using replace() is safe for simple text. For HTML, a proper escaping library is needed.
        String renderedTemplate = template.replace("{{name}}", name);

        // 4. Return a response with the rendered template.
        return new Response(renderedTemplate);
    }

    /**
     * Simulates a home route handler.
     *
     * @param env     The environment.
     * @param request The request.
     * @return The response from the rendering function.
     */
    public static Response home(Environment env, UserRequest request) {
        return render_response_from_env(env, request);
    }

    // Main method with 5 test cases
    public static void main(String[] args) {
        // Setup a shared environment for all requests
        Environment env = new Environment("Hello, {{name}}! Welcome to our site.");

        // --- Test Cases ---

        // Test Case 1: Standard name
        UserRequest request1 = new UserRequest();
        request1.setParameter("name", "Alice");
        Response response1 = home(env, request1);
        System.out.println("Test Case 1: " + response1.getBody());

        // Test Case 2: Another name
        UserRequest request2 = new UserRequest();
        request2.setParameter("name", "Bob");
        Response response2 = home(env, request2);
        System.out.println("Test Case 2: " + response2.getBody());

        // Test Case 3: Name with numbers
        UserRequest request3 = new UserRequest();
        request3.setParameter("name", "Charlie7");
        Response response3 = home(env, request3);
        System.out.println("Test Case 3: " + response3.getBody());

        // Test Case 4: No "name" parameter provided (defaults to "Guest")
        UserRequest request4 = new UserRequest();
        Response response4 = home(env, request4);
        System.out.println("Test Case 4: " + response4.getBody());
        
        // Test Case 5: Name that could be an XSS attempt in a real web app
        UserRequest request5 = new UserRequest();
        request5.setParameter("name", "<script>alert('xss')</script>");
        Response response5 = home(env, request5);
        System.out.println("Test Case 5: " + response5.getBody());
    }
}