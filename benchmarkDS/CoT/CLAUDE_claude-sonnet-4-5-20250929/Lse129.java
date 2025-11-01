
import java.util.*;
import java.util.regex.Pattern;

class Lse129 {
    private static final Pattern SAFE_NAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_\\\\s-]{1,50}$");
    
    static class Environment {
        private Map<String, String> templates = new HashMap<>();
        
        public Environment() {
            templates.put("greeting", "Hello, {{name}}! Welcome to our application.");
            templates.put("default", "Welcome, {{name}}!");
        }
        
        public String getTemplate(String templateName) {
            return templates.getOrDefault(templateName, templates.get("default"));
        }
    }
    
    static class Request {
        private Map<String, String> parameters = new HashMap<>();
        
        public void setParameter(String key, String value) {
            parameters.put(key, value);
        }
        
        public String getParameter(String key) {
            return parameters.get(key);
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
    
    public static String sanitizeInput(String input) {
        if (input == null) {
            return "Guest";
        }
        
        input = input.trim();
        
        if (!SAFE_NAME_PATTERN.matcher(input).matches()) {
            return "Guest";
        }
        
        return input.replaceAll("[<>\\"'&]", "");
    }
    
    public static Response renderResponseFromEnv(Environment env, Request request) {
        try {
            String name = request.getParameter("name");
            name = sanitizeInput(name);
            
            String template = env.getTemplate("greeting");
            
            String rendered = template.replace("{{name}}", escapeHtml(name));
            
            return new Response(rendered);
        } catch (Exception e) {
            return new Response("Error processing request");
        }
    }
    
    public static String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    public static Response home(Environment env, Request request) {
        return renderResponseFromEnv(env, request);
    }
    
    public static void main(String[] args) {
        Environment env = new Environment();
        
        // Test case 1: Valid name
        Request req1 = new Request();
        req1.setParameter("name", "John");
        System.out.println("Test 1: " + home(env, req1).getContent());
        
        // Test case 2: Name with spaces
        Request req2 = new Request();
        req2.setParameter("name", "Jane Doe");
        System.out.println("Test 2: " + home(env, req2).getContent());
        
        // Test case 3: Null name
        Request req3 = new Request();
        System.out.println("Test 3: " + home(env, req3).getContent());
        
        // Test case 4: Name with special characters (should be sanitized)
        Request req4 = new Request();
        req4.setParameter("name", "<script>alert('xss')</script>");
        System.out.println("Test 4: " + home(env, req4).getContent());
        
        // Test case 5: Name with invalid characters
        Request req5 = new Request();
        req5.setParameter("name", "User@123!");
        System.out.println("Test 5: " + home(env, req5).getContent());
    }
}
