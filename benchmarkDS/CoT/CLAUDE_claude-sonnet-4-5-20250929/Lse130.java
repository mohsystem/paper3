
import java.util.*;
import java.util.regex.Pattern;

public class Lse130 {
    private static final Pattern SAFE_NAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_\\\\s-]{1,50}$");
    
    static class Request {
        private Map<String, String> parameters;
        
        public Request(Map<String, String> parameters) {
            this.parameters = parameters;
        }
        
        public String getParameter(String key) {
            return parameters.get(key);
        }
    }
    
    static class Environment {
        private String template;
        private Request request;
        
        public Environment(String template, Request request) {
            this.template = template;
            this.request = request;
        }
        
        public String getTemplate() {
            return template;
        }
        
        public Request getRequest() {
            return request;
        }
    }
    
    static class Response {
        private String content;
        
        public Response(String content) {
            this.content = content;
        }
        
        public String getContent() {
            return content;
        }
    }
    
    // Security: Sanitize input to prevent injection attacks
    private static String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        // Validate against allowed pattern
        if (!SAFE_NAME_PATTERN.matcher(input).matches()) {
            return "";
        }
        // HTML escape to prevent XSS
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    public static Response renderResponseFromEnv(Environment env) {
        if (env == null || env.getRequest() == null) {
            return new Response("Error: Invalid environment");
        }
        
        // Security: Validate and sanitize name parameter
        String name = env.getRequest().getParameter("name");
        String safeName = sanitizeInput(name);
        
        if (safeName.isEmpty()) {
            safeName = "Guest";
        }
        
        // Security: Use safe template rendering (no direct string interpolation)
        String template = env.getTemplate();
        if (template == null) {
            template = "Hello, {name}!";
        }
        
        // Security: Safe template rendering with escaped content
        String renderedTemplate = template.replace("{name}", safeName);
        
        return new Response(renderedTemplate);
    }
    
    public static Response home(Environment env) {
        return renderResponseFromEnv(env);
    }
    
    public static void main(String[] args) {
        // Test Case 1: Normal valid name
        Map<String, String> params1 = new HashMap<>();
        params1.put("name", "John");
        Environment env1 = new Environment("Hello, {name}!", new Request(params1));
        System.out.println("Test 1: " + home(env1).getContent());
        
        // Test Case 2: Name with XSS attempt
        Map<String, String> params2 = new HashMap<>();
        params2.put("name", "<script>alert('XSS')</script>");
        Environment env2 = new Environment("Welcome, {name}!", new Request(params2));
        System.out.println("Test 2: " + home(env2).getContent());
        
        // Test Case 3: Null name parameter
        Map<String, String> params3 = new HashMap<>();
        Environment env3 = new Environment("Hi, {name}!", new Request(params3));
        System.out.println("Test 3: " + home(env3).getContent());
        
        // Test Case 4: Name with special characters
        Map<String, String> params4 = new HashMap<>();
        params4.put("name", "O'Brien");
        Environment env4 = new Environment("Greetings, {name}!", new Request(params4));
        System.out.println("Test 4: " + home(env4).getContent());
        
        // Test Case 5: Name exceeding length limit
        Map<String, String> params5 = new HashMap<>();
        params5.put("name", "A".repeat(100));
        Environment env5 = new Environment("Hello, {name}!", new Request(params5));
        System.out.println("Test 5: " + home(env5).getContent());
    }
}
