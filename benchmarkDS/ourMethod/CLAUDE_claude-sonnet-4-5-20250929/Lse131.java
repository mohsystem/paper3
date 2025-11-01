
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse131 {
    // Environment holds templates
    static class Environment {
        private final Map<String, String> templates = new HashMap<>();
        
        public Environment() {
            // Pre-register safe templates only
            templates.put("greeting", "Hello, {{name}}!");
            templates.put("welcome", "Welcome, {{name}}!");
        }
        
        // Get template by name - returns null if not found (safe default)
        public String getTemplate(String templateName) {
            // Validate template name: alphanumeric only, max 50 chars
            if (templateName == null || templateName.length() > 50 || 
                !Pattern.matches("^[a-zA-Z0-9_]+$", templateName)) {
                return null;
            }
            return templates.get(templateName);
        }
    }
    
    // Safely escape HTML special characters to prevent XSS
    private static String escapeHtml(String input) {
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
    
    // Validate name parameter: alphanumeric, spaces, hyphens only, max 100 chars
    private static String validateName(String name) {
        if (name == null || name.isEmpty()) {
            return "Guest"; // Safe default
        }
        // Trim and limit length
        name = name.trim();
        if (name.length() > 100) {
            name = name.substring(0, 100);
        }
        // Only allow safe characters
        if (!Pattern.matches("^[a-zA-Z0-9 \\\\-]+$", name)) {
            return "Guest"; // Reject invalid input
        }
        return name;
    }
    
    // Render template with name parameter - escapes output to prevent XSS
    private static String renderTemplate(String template, String name) {
        if (template == null) {
            return "Invalid template";
        }
        // Validate and sanitize name before rendering
        String safeName = validateName(name);
        // Escape the name for HTML context before substitution
        String escapedName = escapeHtml(safeName);
        // Simple template rendering with safe substitution
        return template.replace("{{name}}", escapedName);
    }
    
    // Extract query parameter from request
    private static String getParameter(HttpExchange exchange, String paramName) {
        String query = exchange.getRequestURI().getQuery();
        if (query == null || query.isEmpty()) {
            return null;
        }
        try {
            String[] pairs = query.split("&");
            for (String pair : pairs) {
                int idx = pair.indexOf("=");
                if (idx > 0) {
                    String key = URLDecoder.decode(pair.substring(0, idx), StandardCharsets.UTF_8.name());
                    if (paramName.equals(key)) {
                        return URLDecoder.decode(pair.substring(idx + 1), StandardCharsets.UTF_8.name());
                    }
                }
            }
        } catch (Exception e) {
            // Log error securely without exposing details
            return null;
        }
        return null;
    }
    
    // Render response from environment - secure implementation
    public static String renderResponseFromEnv(Environment env, HttpExchange exchange) {
        // Get name parameter from request (untrusted input)
        String name = getParameter(exchange, "name");
        
        // Get template from environment (always use "greeting" template)
        String template = env.getTemplate("greeting");
        if (template == null) {
            return "Error: Template not found"; // Generic error message
        }
        
        // Render template with validated and escaped name parameter
        return renderTemplate(template, name);
    }
    
    // Home handler that calls renderResponseFromEnv
    public static String home(Environment env, HttpExchange exchange) {
        return renderResponseFromEnv(env, exchange);
    }
    
    public static void main(String[] args) throws IOException {
        Environment env = new Environment();
        
        // Test cases
        System.out.println("Test Case 1 (valid name):");
        HttpExchange mockExchange1 = createMockExchange("name=Alice");
        System.out.println(home(env, mockExchange1));
        
        System.out.println("\\nTest Case 2 (XSS attempt):");
        HttpExchange mockExchange2 = createMockExchange("name=<script>alert('XSS')</script>");
        System.out.println(home(env, mockExchange2));
        
        System.out.println("\\nTest Case 3 (no name parameter):");
        HttpExchange mockExchange3 = createMockExchange("");
        System.out.println(home(env, mockExchange3));
        
        System.out.println("\\nTest Case 4 (valid name with spaces):");
        HttpExchange mockExchange4 = createMockExchange("name=John%20Doe");
        System.out.println(home(env, mockExchange4));
        
        System.out.println("\\nTest Case 5 (special characters):");
        HttpExchange mockExchange5 = createMockExchange("name=Test%26User");
        System.out.println(home(env, mockExchange5));
    }
    
    // Mock HttpExchange for testing
    private static HttpExchange createMockExchange(String query) {
        return new HttpExchange() {
            public java.net.URI getRequestURI() {
                try {
                    return new java.net.URI("http://localhost/?" + query);
                } catch (Exception e) {
                    return null;
                }
            }
            public com.sun.net.httpserver.Headers getRequestHeaders() { return null; }
            public com.sun.net.httpserver.Headers getResponseHeaders() { return null; }
            public String getRequestMethod() { return "GET"; }
            public java.io.InputStream getRequestBody() { return null; }
            public java.io.OutputStream getResponseBody() { return null; }
            public void sendResponseHeaders(int i, long l) {}
            public void close() {}
            public java.net.InetSocketAddress getRemoteAddress() { return null; }
            public java.net.InetSocketAddress getLocalAddress() { return null; }
            public String getProtocol() { return "HTTP/1.1"; }
            public Object getAttribute(String s) { return null; }
            public void setAttribute(String s, Object o) {}
            public void setStreams(java.io.InputStream in, java.io.OutputStream out) {}
            public com.sun.net.httpserver.HttpContext getHttpContext() { return null; }
            public com.sun.net.httpserver.HttpPrincipal getPrincipal() { return null; }
        };
    }
}
