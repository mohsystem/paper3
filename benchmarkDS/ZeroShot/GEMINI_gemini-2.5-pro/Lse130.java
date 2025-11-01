import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse130 {

    /**
     * Performs HTML entity escaping on a string.
     * This is a crucial security measure to prevent Cross-Site Scripting (XSS).
     * @param input The string to escape.
     * @return The escaped string, safe to embed in HTML.
     */
    private static String secureHtmlEscape(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\"", "&quot;")
                    .replace("'", "&#x27;");
    }

    /**
     * Parses the query string of a URL.
     * @param query The query string (e.g., "name=Alice&age=30").
     * @return A map of key-value pairs from the query.
     */
    private static Map<String, String> parseQuery(String query) {
        if (query == null || query.isEmpty()) {
            return new HashMap<>();
        }
        return Stream.of(query.split("&"))
                .map(param -> param.split("=", 2))
                .filter(pair -> pair.length == 2)
                .collect(Collectors.toMap(
                    pair -> {
                        try {
                            return URLDecoder.decode(pair[0], StandardCharsets.UTF_8.name());
                        } catch (UnsupportedEncodingException e) {
                            return pair[0]; // Should not happen with UTF-8
                        }
                    },
                    pair -> {
                        try {
                            return URLDecoder.decode(pair[1], StandardCharsets.UTF_8.name());
                        } catch (UnsupportedEncodingException e) {
                            return pair[1]; // Should not happen with UTF-8
                        }
                    }
                ));
    }

    /**
     * Renders a response based on the request environment.
     * @param exchange The HttpExchange object representing the request and response.
     * @return A string containing the full HTTP response body.
     */
    public static String render_response_from_env(HttpExchange exchange) {
        // The "environment" provides the request details.
        URI requestUri = exchange.getRequestURI();
        Map<String, String> params = parseQuery(requestUri.getQuery());

        // Get the name parameter from the request, default to "World".
        String name = params.getOrDefault("name", "World");

        // Get the template from the "environment" (in this case, a hardcoded string).
        String template = "<html><body><h1>Hello, {{name}}!</h1></body></html>";

        // Securely render the template with the name parameter.
        // The secureHtmlEscape function prevents XSS attacks.
        String renderedTemplate = template.replace("{{name}}", secureHtmlEscape(name));

        return renderedTemplate;
    }

    /**
     * Handler for the home ("/") route.
     * @param exchange The HttpExchange object.
     * @throws IOException
     */
    public static void home(HttpExchange exchange) throws IOException {
        String response = render_response_from_env(exchange);

        exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
        exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(response.getBytes(StandardCharsets.UTF_8));
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        // The home function is the handler for the "/" context.
        server.createContext("/", Lse130::home);
        
        server.setExecutor(null); // creates a default executor
        server.start();
        
        System.out.println("Server started on port " + port);
        System.out.println("--- Test Cases ---");
        System.out.println("1. Normal case: Open http://localhost:8080/?name=Alice in your browser.");
        System.out.println("2. Default case: Open http://localhost:8080/ in your browser.");
        System.out.println("3. Empty param: Open http://localhost:8080/?name= in your browser.");
        System.out.println("4. Special chars: Open http://localhost:8080/?name=Bob%20&%20Charlie in your browser.");
        System.out.println("5. XSS attack test (should be safely escaped): Open http://localhost:8080/?name=%3Cscript%3Ealert('XSS')%3C/script%3E in your browser.");
    }
}