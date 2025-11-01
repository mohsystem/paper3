
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;

public class Lse130 {
    private static class Environment {
        private Map<String, String> templates;
        
        public Environment() {
            templates = new HashMap<>();
            templates.put("default", "<html><body><h1>Hello, {{name}}!</h1></body></html>");
        }
        
        public String getTemplate(String templateName) {
            return templates.getOrDefault(templateName, templates.get("default"));
        }
    }
    
    private static class Response {
        private String content;
        private int statusCode;
        
        public Response(String content, int statusCode) {
            this.content = content;
            this.statusCode = statusCode;
        }
        
        public String getContent() {
            return content;
        }
        
        public int getStatusCode() {
            return statusCode;
        }
    }
    
    public static Response renderResponseFromEnv(Environment env, Map<String, String> params) {
        // Get name parameter from request (sanitize to prevent XSS)
        String name = params.getOrDefault("name", "Guest");
        name = sanitizeInput(name);
        
        // Get template from environment
        String template = env.getTemplate("default");
        
        // Render template with name parameter
        String renderedContent = template.replace("{{name}}", name);
        
        // Return response with rendered template
        return new Response(renderedContent, 200);
    }
    
    public static Response home(Environment env, Map<String, String> params) {
        return renderResponseFromEnv(env, params);
    }
    
    private static String sanitizeInput(String input) {
        if (input == null) return "";
        return input.replaceAll("<", "&lt;")
                   .replaceAll(">", "&gt;")
                   .replaceAll("\\"", "&quot;")
                   .replaceAll("'", "&#x27;")
                   .replaceAll("&", "&amp;");
    }
    
    private static Map<String, String> parseQueryParams(String query) {
        Map<String, String> params = new HashMap<>();
        if (query != null && !query.isEmpty()) {
            String[] pairs = query.split("&");
            for (String pair : pairs) {
                String[] keyValue = pair.split("=");
                if (keyValue.length == 2) {
                    try {
                        String key = URLDecoder.decode(keyValue[0], StandardCharsets.UTF_8.name());
                        String value = URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8.name());
                        params.put(key, value);
                    } catch (UnsupportedEncodingException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
        return params;
    }
    
    public static void main(String[] args) {
        Environment env = new Environment();
        
        // Test case 1: Normal name
        Map<String, String> params1 = new HashMap<>();
        params1.put("name", "John");
        Response response1 = home(env, params1);
        System.out.println("Test 1: " + response1.getContent());
        
        // Test case 2: Empty name (should default to Guest)
        Map<String, String> params2 = new HashMap<>();
        Response response2 = home(env, params2);
        System.out.println("Test 2: " + response2.getContent());
        
        // Test case 3: Name with special characters (XSS attempt)
        Map<String, String> params3 = new HashMap<>();
        params3.put("name", "<script>alert('XSS')</script>");
        Response response3 = home(env, params3);
        System.out.println("Test 3: " + response3.getContent());
        
        // Test case 4: Name with quotes
        Map<String, String> params4 = new HashMap<>();
        params4.put("name", "John \\"Doe\\"");
        Response response4 = home(env, params4);
        System.out.println("Test 4: " + response4.getContent());
        
        // Test case 5: Unicode name
        Map<String, String> params5 = new HashMap<>();
        params5.put("name", "José");
        Response response5 = home(env, params5);
        System.out.println("Test 5: " + response5.getContent());
    }
}
