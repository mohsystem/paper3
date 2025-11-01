import java.util.HashMap;
import java.util.Map;
import java.util.function.Function;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Lse134 {

    // Simple router to map URL patterns to handler functions
    private static final Map<Pattern, Function<String, String>> routes = new HashMap<>();

    /**
     * A simple HTML escape function to prevent XSS.
     * It escapes <, >, &, ", and '.
     *
     * @param input The string to escape.
     * @return The HTML-escaped string.
     */
    public static String escape(String input) {
        if (input == null) {
            return "";
        }
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < input.length(); i++) {
            char c = input.charAt(i);
            switch (c) {
                case '<':
                    sb.append("&lt;");
                    break;
                case '>':
                    sb.append("&gt;");
                    break;
                case '&':
                    sb.append("&amp;");
                    break;
                case '"':
                    sb.append("&quot;");
                    break;
                case '\'':
                    sb.append("&#x27;");
                    break;
                default:
                    sb.append(c);
                    break;
            }
        }
        return sb.toString();
    }

    /**
     * Handler function for the /hello/<username> route.
     *
     * @param username The username from the URL.
     * @return A greeting string.
     */
    public static String hello(String username) {
        // Rule #1: Sanitize input before processing.
        String escapedUsername = escape(username);
        return "Hello, " + escapedUsername + "!";
    }

    /**
     * Simulates a decorator by mapping a URL pattern to a handler function.
     *
     * @param urlPattern The regex pattern for the URL.
     * @param handler    The function to handle the request.
     */
    public static void route(String urlPattern, Function<String, String> handler) {
        routes.put(Pattern.compile(urlPattern), handler);
    }

    /**
     * Dispatches a request to the appropriate handler based on the URL.
     *
     * @param url The request URL.
     * @return The response from the handler, or an error message.
     */
    public static String dispatch(String url) {
        for (Map.Entry<Pattern, Function<String, String>> entry : routes.entrySet()) {
            Matcher matcher = entry.getKey().matcher(url);
            if (matcher.matches() && matcher.groupCount() >= 1) {
                String username = matcher.group(1);
                return entry.getValue().apply(username);
            }
        }
        return "404 Not Found";
    }

    public static void main(String[] args) {
        // This simulates a decorator creating a mapping for the URL.
        route("^/hello/(.*)$", Lse134::hello);

        // --- 5 Test Cases ---
        String[] testUrls = {
            "/hello/World",
            "/hello/Java",
            "/hello/<script>alert('xss')</script>",
            "/hello/O'Malley",
            "/hello/a&b"
        };
        
        System.out.println("--- Java Test Cases ---");
        for (String url : testUrls) {
            String response = dispatch(url);
            System.out.println("Request URL: " + url);
            System.out.println("Response: " + response);
            System.out.println();
        }
    }
}