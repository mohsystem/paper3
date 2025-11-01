import java.util.HashMap;
import java.util.Map;

// Main class as per instruction
public class Lse130 {

    // A simple representation of a Request
    static class Request {
        private Map<String, String> parameters;

        public Request() {
            this.parameters = new HashMap<>();
        }

        public void setParameter(String key, String value) {
            this.parameters.put(key, value);
        }

        public String getParameter(String key) {
            return this.parameters.getOrDefault(key, "");
        }
    }

    // A simple representation of a Response
    static class Response {
        private String body;

        public Response(String body) {
            this.body = body;
        }

        public String getBody() {
            return this.body;
        }

        @Override
        public String toString() {
            return "Response{body='" + body + "'}";
        }
    }

    /**
     * The render_response_from_env function takes an environment as an argument
     * and gets the name parameter from the request. The function also gets the
     * template from the environment. The function renders the template with the
     * name parameter. The function returns a response with the rendered template.
     *
     * @param env A Map representing the environment, containing the 'request' and 'template'.
     * @return A Response object with the rendered content.
     */
    public static Response render_response_from_env(Map<String, Object> env) {
        Request request = (Request) env.get("request");
        String template = (String) env.get("template");

        String name = request.getParameter("name");

        // Simple rendering by replacing a placeholder
        String renderedTemplate = template.replace("{name}", name);

        return new Response(renderedTemplate);
    }

    /**
     * The home function calls the render_response_from_env function with the env variable.
     * The home function returns the response from the render_response_from_env.
     *
     * @param env The environment Map.
     * @return A Response object.
     */
    public static Response home(Map<String, Object> env) {
        return render_response_from_env(env);
    }

    public static void main(String[] args) {
        // Test Case 1
        Map<String, Object> env1 = new HashMap<>();
        Request req1 = new Request();
        req1.setParameter("name", "Alice");
        env1.put("request", req1);
        env1.put("template", "Hello, {name}!");
        System.out.println("Test Case 1: " + home(env1));

        // Test Case 2
        Map<String, Object> env2 = new HashMap<>();
        Request req2 = new Request();
        req2.setParameter("name", "Bob");
        env2.put("request", req2);
        env2.put("template", "Welcome, {name}. How are you?");
        System.out.println("Test Case 2: " + home(env2));
        
        // Test Case 3
        Map<String, Object> env3 = new HashMap<>();
        Request req3 = new Request();
        req3.setParameter("name", "Charlie");
        env3.put("request", req3);
        env3.put("template", "Goodbye, {name}.");
        System.out.println("Test Case 3: " + home(env3));

        // Test Case 4: No name parameter
        Map<String, Object> env4 = new HashMap<>();
        Request req4 = new Request();
        env4.put("request", req4);
        env4.put("template", "Hello, {name}!");
        System.out.println("Test Case 4: " + home(env4));

        // Test Case 5: Multiple placeholders
        Map<String, Object> env5 = new HashMap<>();
        Request req5 = new Request();
        req5.setParameter("name", "David");
        env5.put("request", req5);
        env5.put("template", "{name}, we are happy to see you, {name}.");
        System.out.println("Test Case 5: " + home(env5));
    }
}