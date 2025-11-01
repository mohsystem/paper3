
import java.io.StringWriter;
import java.util.HashMap;
import java.util.Map;

public class Lse131 {
    // Mock Environment class to simulate template environment
    static class Environment {
        private Map<String, String> templates;
        
        public Environment() {
            templates = new HashMap<>();
            templates.put("home.html", "Hello, {{name}}! Welcome to our secure application.");
        }
        
        public String getTemplate(String templateName) {
            return templates.getOrDefault(templateName, "Template not found");
        }
    }
    
    // Mock Request class
    static class Request {
        private Map<String, String> parameters;
        
        public Request(String name) {
            parameters = new HashMap<>();
            parameters.put("name", name);
        }
        
        public String getParameter(String key) {
            return parameters.get(key);
        }
    }
    
    // Mock Response class
    static class Response {
        private String content;
        
        public Response(String content) {
            this.content = content;
        }
        
        public String getContent() {
            return content;
        }
        
        @Override
        public String toString() {
            return content;
        }
    }
    
    // Secure HTML encoding to prevent XSS attacks
    private static String htmlEncode(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }
    
    // Render template with secure parameter substitution
    private static String renderTemplate(String template, Map<String, String> params) {
        String result = template;
        for (Map.Entry<String, String> entry : params.entrySet()) {
            String placeholder = "{{" + entry.getKey() + "}}";
            String safeValue = htmlEncode(entry.getValue());
            result = result.replace(placeholder, safeValue);
        }
        return result;
    }
    
    public static Response renderResponseFromEnv(Environment env, Request request) {
        // Get name parameter from request with validation
        String name = request.getParameter("name");
        
        // Input validation - prevent null and empty values
        if (name == null || name.trim().isEmpty()) {
            name = "Guest";
        }
        
        // Length validation to prevent DoS
        if (name.length() > 100) {
            name = name.substring(0, 100);
        }
        
        // Get template from environment
        String template = env.getTemplate("home.html");
        
        // Prepare parameters for rendering
        Map<String, String> params = new HashMap<>();
        params.put("name", name);
        
        // Render template with secure parameter substitution
        String renderedContent = renderTemplate(template, params);
        
        // Return response with rendered template
        return new Response(renderedContent);
    }
    
    public static Response home(Environment env, Request request) {
        return renderResponseFromEnv(env, request);
    }
    
    public static void main(String[] args) {
        Environment env = new Environment();
        
        // Test case 1: Normal input
        Request req1 = new Request("Alice");
        Response resp1 = home(env, req1);
        System.out.println("Test 1: " + resp1);
        
        // Test case 2: Input with special characters (XSS attempt)
        Request req2 = new Request("<script>alert('XSS')</script>");
        Response resp2 = home(env, req2);
        System.out.println("Test 2: " + resp2);
        
        // Test case 3: Empty input
        Request req3 = new Request("");
        Response resp3 = home(env, req3);
        System.out.println("Test 3: " + resp3);
        
        // Test case 4: Input with HTML tags
        Request req4 = new Request("<b>Bob</b>");
        Response resp4 = home(env, req4);
        System.out.println("Test 4: " + resp4);
        
        // Test case 5: Input with quotes
        Request req5 = new Request("O'Reilly & Sons");
        Response resp5 = home(env, req5);
        System.out.println("Test 5: " + resp5);
    }
}
