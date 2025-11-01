import java.util.HashMap;
import java.util.Map;
import java.util.function.Function;

public class Lse134 {

    // Simulates a URL-to-function mapping, like a decorator would create.
    private static final Map<String, Function<String, String>> routes = new HashMap<>();

    /**
     * Converts special characters to HTML-safe sequences.
     * @param str The string to escape.
     * @return The escaped string.
     */
    public static String escape(String str) {
        if (str == null) {
            return null;
        }
        StringBuilder sb = new StringBuilder();
        for (char c : str.toCharArray()) {
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
                    sb.append("&#39;");
                    break;
                default:
                    sb.append(c);
            }
        }
        return sb.toString();
    }

    /**
     * The function mapped to the /hello/username URL.
     * @param username The username from the URL.
     * @return A greeting string.
     */
    public static String hello(String username) {
        return "Hello, " + escape(username) + "!";
    }

    /**
     * Handles an incoming URL request, routing it to the correct function.
     * @param url The URL to handle.
     * @return The result from the mapped function or a 404 message.
     */
    public static String handleRequest(String url) {
        String prefix = "/hello/";
        if (url != null && url.startsWith(prefix)) {
            // Extract the username part of the URL
            String username = url.substring(prefix.length());
            // Get the function from the routing map and apply it
            Function<String, String> handler = routes.get(prefix);
            if (handler != null) {
                return handler.apply(username);
            }
        }
        return "404 Not Found";
    }

    public static void main(String[] args) {
        // Create the mapping between the URL pattern and the function.
        routes.put("/hello/", Lse134::hello);

        // --- 5 Test Cases ---
        System.out.println("Test Case 1: Simple username");
        System.out.println("Input: /hello/Alice");
        System.out.println("Output: " + handleRequest("/hello/Alice"));
        System.out.println();

        System.out.println("Test Case 2: Username with HTML characters");
        System.out.println("Input: /hello/<script>alert('XSS')</script>");
        System.out.println("Output: " + handleRequest("/hello/<script>alert('XSS')</script>"));
        System.out.println();
        
        System.out.println("Test Case 3: Empty username");
        System.out.println("Input: /hello/");
        System.out.println("Output: " + handleRequest("/hello/"));
        System.out.println();

        System.out.println("Test Case 4: Mismatched URL");
        System.out.println("Input: /goodbye/Bob");
        System.out.println("Output: " + handleRequest("/goodbye/Bob"));
        System.out.println();

        System.out.println("Test Case 5: Username with various special characters");
        System.out.println("Input: /hello/Tom & Jerry > \"The Movie\"");
        System.out.println("Output: " + handleRequest("/hello/Tom & Jerry > \"The Movie\""));
        System.out.println();
    }
}