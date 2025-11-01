
import java.io.IOException;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.function.Function;
import org.apache.commons.text.StringEscapeUtils;

public class Lse134 {
    private Map<String, Function<String, String>> routes = new HashMap<>();
    
    public Lse134() {
        // Register the route
        registerRoute("/hello/{username}", this::hello);
    }
    
    public void registerRoute(String pattern, Function<String, String> handler) {
        routes.put(pattern, handler);
    }
    
    public String hello(String username) {
        // Escape special characters to prevent XSS attacks
        String safeUsername = escapeHtml(username);
        return "Hello " + safeUsername;
    }
    
    public String escapeHtml(String input) {
        if (input == null) {
            return "";
        }
        return StringEscapeUtils.escapeHtml4(input);
    }
    
    public String handleRequest(String path) {
        // Extract username from path
        if (path.startsWith("/hello/")) {
            String username = path.substring("/hello/".length());
            try {
                username = URLDecoder.decode(username, StandardCharsets.UTF_8.toString());
            } catch (Exception e) {
                return "Invalid request";
            }
            return hello(username);
        }
        return "Not Found";
    }
    
    public static void main(String[] args) {
        Lse134 app = new Lse134();
        
        // Test cases
        System.out.println("Test 1: " + app.handleRequest("/hello/John"));
        System.out.println("Test 2: " + app.handleRequest("/hello/Alice"));
        System.out.println("Test 3: " + app.handleRequest("/hello/<script>alert('XSS')</script>"));
        System.out.println("Test 4: " + app.handleRequest("/hello/User&Name"));
        System.out.println("Test 5: " + app.handleRequest("/hello/Test%20User"));
    }
}
