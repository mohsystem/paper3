import java.util.HashMap;
import java.util.Map;
import java.util.function.Function;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Lse134 {

    private static class Router {
        // A map to hold URL patterns and their corresponding handler functions
        private final Map<Pattern, Function<Matcher, String>> routes = new HashMap<>();

        /**
         * Simulates a decorator by registering a URL pattern and its handler.
         *
         * @param pathPattern The regex pattern for the URL.
         * @param handler The function to execute when the URL matches.
         */
        public void addRoute(String pathPattern, Function<Matcher, String> handler) {
            routes.put(Pattern.compile("^" + pathPattern + "$"), handler);
        }

        /**
         * Handles an incoming URL request by finding a matching route and executing its handler.
         *
         * @param url The URL to handle.
         * @return The response string from the handler, or a 404 message.
         */
        public String handleRequest(String url) {
            for (Map.Entry<Pattern, Function<Matcher, String>> entry : routes.entrySet()) {
                Matcher matcher = entry.getKey().matcher(url);
                if (matcher.matches()) {
                    return entry.getValue().apply(matcher);
                }
            }
            return "404 Not Found";
        }
    }

    /**
     * Converts special characters in a string to HTML-safe sequences.
     * This is crucial to prevent Cross-Site Scripting (XSS) attacks.
     *
     * @param input The string to escape.
     * @return The HTML-escaped string.
     */
    private static String escape(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder escaped = new StringBuilder();
        for (char c : input.toCharArray()) {
            switch (c) {
                case '<':
                    escaped.append("&lt;");
                    break;
                case '>':
                    escaped.append("&gt;");
                    break;
                case '&':
                    escaped.append("&amp;");
                    break;
                case '"':
                    escaped.append("&quot;");
                    break;
                case '\'':
                    escaped.append("&#39;");
                    break;
                default:
                    escaped.append(c);
                    break;
            }
        }
        return escaped.toString();
    }

    /**
     * Handler function for the /hello/<username> route.
     *
     * @param username The username extracted from the URL.
     * @return A greeting string with the escaped username.
     */
    public static String hello(String username) {
        // The username is escaped before being included in the output.
        return "Hello " + escape(username);
    }

    public static void main(String[] args) {
        Router router = new Router();

        // This simulates the decorator mapping the URL to the hello function.
        // The pattern uses a named capturing group "username".
        router.addRoute("/hello/(?<username>[^/]+)", matcher -> {
            String username = matcher.group("username");
            return hello(username);
        });

        // --- Test Cases ---
        System.out.println("--- Java Test Cases ---");
        String[] testUrls = {
            "/hello/Alice",
            "/hello/Bob",
            "/hello/<script>alert('xss')</script>", // Security test case
            "/hello/John Doe",
            "/hello/another/path" // Should not match
        };

        for (String url : testUrls) {
            System.out.println("Request URL: " + url);
            String response = router.handleRequest(url);
            System.out.println("Response: " + response);
            System.out.println();
        }
    }
}