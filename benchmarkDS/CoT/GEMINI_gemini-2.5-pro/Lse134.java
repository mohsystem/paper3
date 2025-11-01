import java.util.HashMap;
import java.util.Map;
import java.util.function.Function;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Lse134 {

    // Simulates a simple web framework router
    private static final Map<Pattern, Function<Matcher, String>> routes = new HashMap<>();

    /**
     * A helper method to simulate adding a route. In a real framework, this would be done
     * by scanning for annotations at startup.
     * @param urlPattern The regex pattern for the URL.
     * @param handler The function to handle the request.
     */
    public static void addRoute(String urlPattern, Function<Matcher, String> handler) {
        routes.put(Pattern.compile(urlPattern), handler);
    }

    /**
     * Simulates dispatching a request to the correct handler based on the URL.
     * @param url The incoming request URL.
     * @return The response string or a 404 message.
     */
    public static String handleRequest(String url) {
        for (Map.Entry<Pattern, Function<Matcher, String>> entry : routes.entrySet()) {
            Matcher matcher = entry.getKey().matcher(url);
            if (matcher.matches()) {
                return entry.getValue().apply(matcher);
            }
        }
        return "404 Not Found";
    }

    /**
     * Converts special characters to HTML-safe sequences to prevent XSS.
     * @param s The input string.
     * @return The escaped string.
     */
    public static String escape(String s) {
        if (s == null) {
            return "";
        }
        StringBuilder out = new StringBuilder(Math.max(16, s.length()));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c > 127 || c == '"' || c == '\'' || c == '<' || c == '>' || c == '&') {
                out.append("&#");
                out.append((int) c);
                out.append(';');
            } else {
                out.append(c);
            }
        }
        return out.toString();
    }

    /**
     * The handler function for the /hello/<username> route.
     * @param username The username extracted from the URL.
     * @return A greeting string.
     */
    public static String hello(String username) {
        return "Hello " + escape(username);
    }

    public static void main(String[] args) {
        // In a real framework, this would be done with annotations like @Route("/hello/(?<username>[^/]+)")
        // We simulate the decorator/annotation mapping here.
        addRoute("^/hello/(?<username>[^/]+)$", matcher -> hello(matcher.group("username")));

        // 5 test cases
        String[] testUrls = {
            "/hello/World",
            "/hello/Jane Doe",
            "/hello/<script>alert('XSS')</script>",
            "/hello/user&'\"<>",
            "/hello/" // Test with an empty username part
        };
        
        // Simulating the URL pattern with an empty username part for the last test case
        addRoute("^/hello/$", matcher -> hello(""));


        System.out.println("Java Test Cases:");
        for (String url : testUrls) {
            System.out.println("Request: " + url);
            System.out.println("Response: " + handleRequest(url));
            System.out.println("---");
        }
    }
}