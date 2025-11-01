
import java.io.IOException;
import java.io.StringWriter;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

// Simple template engine that safely escapes HTML
class TemplateEngine {
    private static final Pattern TEMPLATE_VAR = Pattern.compile("\\\\{\\\\{\\\\s*name\\\\s*\\\\}\\\\}");
    
    public static String render(String template, Map<String, String> context) {
        if (template == null || context == null) {
            return "";
        }
        // Escape HTML to prevent XSS
        String name = context.getOrDefault("name", "");
        String safeName = escapeHtml(name);
        return TEMPLATE_VAR.matcher(template).replaceAll(safeName);
    }
    
    // HTML escape to prevent XSS attacks
    private static String escapeHtml(String input) {
        if (input == null) return "";
        StringBuilder escaped = new StringBuilder();
        for (char c : input.toCharArray()) {
            switch (c) {
                case '<': escaped.append("&lt;"); break;
                case '>': escaped.append("&gt;"); break;
                case '&': escaped.append("&amp;"); break;
                case '"': escaped.append("&quot;"); break;
                case '\\'': escaped.append("&#x27;"); break;
                case '/': escaped.append("&#x2F;"); break;
                default: escaped.append(c);
            }
        }
        return escaped.toString();
    }
}

// Simple request object
class Request {
    private Map<String, String> params;
    
    public Request(Map<String, String> params) {
        this.params = params != null ? params : new HashMap<>();
    }
    
    public String getParameter(String key) {
        if (key == null) return null;
        String value = params.get(key);
        // Validate and sanitize input - limit length to prevent DoS
        if (value != null && value.length() > 1000) {
            value = value.substring(0, 1000);
        }
        return value;
    }
}

// Environment holding request and template
class Environment {
    private Request request;
    private String template;
    
    public Environment(Request request, String template) {
        this.request = request;
        // Validate template is not null and has reasonable size
        if (template != null && template.length() > 100000) {
            throw new IllegalArgumentException("Template size exceeds maximum limit");
        }
        this.template = template;
    }
    
    public Request getRequest() {
        return request;
    }
    
    public String getTemplate() {
        return template != null ? template : "";
    }
}

// Response object
class Response {
    private String content;
    private int statusCode;
    
    public Response(String content, int statusCode) {
        this.content = content != null ? content : "";
        this.statusCode = statusCode;
    }
    
    public String getContent() {
        return content;
    }
    
    public int getStatusCode() {
        return statusCode;
    }
}

public class Lse130 {
    
    // Renders response from environment
    // Security: Validates input, escapes output to prevent XSS
    public static Response renderResponseFromEnv(Environment env) {
        if (env == null) {
            return new Response("Invalid environment", 400);
        }
        
        try {
            // Get name parameter from request - treat as untrusted input
            Request request = env.getRequest();
            if (request == null) {
                return new Response("Invalid request", 400);
            }
            
            String name = request.getParameter("name");
            // Default to empty string if name is null
            if (name == null) {
                name = "";
            }
            
            // Get template from environment
            String template = env.getTemplate();
            if (template == null || template.isEmpty()) {
                return new Response("Template not found", 404);
            }
            
            // Render template with sanitized name parameter
            Map<String, String> context = new HashMap<>();
            context.put("name", name);
            String rendered = TemplateEngine.render(template, context);
            
            return new Response(rendered, 200);
            
        } catch (Exception e) {
            // Log error internally, return generic message to user
            System.err.println("Error rendering template: " + e.getMessage());
            return new Response("Internal server error", 500);
        }
    }
    
    // Home function that calls renderResponseFromEnv
    public static Response home(Environment env) {
        return renderResponseFromEnv(env);
    }
    
    // Test cases
    public static void main(String[] args) {
        // Test case 1: Normal input
        Map<String, String> params1 = new HashMap<>();
        params1.put("name", "John");
        Request req1 = new Request(params1);
        Environment env1 = new Environment(req1, "Hello, {{ name }}!");
        Response resp1 = home(env1);
        System.out.println("Test 1: " + resp1.getContent() + " (Status: " + resp1.getStatusCode() + ")");
        
        // Test case 2: XSS attempt - should be escaped
        Map<String, String> params2 = new HashMap<>();
        params2.put("name", "<script>alert('XSS')</script>");
        Request req2 = new Request(params2);
        Environment env2 = new Environment(req2, "Hello, {{ name }}!");
        Response resp2 = home(env2);
        System.out.println("Test 2: " + resp2.getContent() + " (Status: " + resp2.getStatusCode() + ")");
        
        // Test case 3: Empty name parameter
        Map<String, String> params3 = new HashMap<>();
        params3.put("name", "");
        Request req3 = new Request(params3);
        Environment env3 = new Environment(req3, "Hello, {{ name }}!");
        Response resp3 = home(env3);
        System.out.println("Test 3: " + resp3.getContent() + " (Status: " + resp3.getStatusCode() + ")");
        
        // Test case 4: Missing name parameter
        Map<String, String> params4 = new HashMap<>();
        Request req4 = new Request(params4);
        Environment env4 = new Environment(req4, "Hello, {{ name }}!");
        Response resp4 = home(env4);
        System.out.println("Test 4: " + resp4.getContent() + " (Status: " + resp4.getStatusCode() + ")");
        
        // Test case 5: Null environment
        Response resp5 = home(null);
        System.out.println("Test 5: " + resp5.getContent() + " (Status: " + resp5.getStatusCode() + ")");
    }
}
