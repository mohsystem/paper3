import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Task38 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new RootHandler());
        server.createContext("/submit", new SubmitHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Open http://localhost:8080/ in your browser.");
        System.out.println("The main method starts the server. Manual testing is required.");
        System.out.println("Test Case 1: Enter 'Hello World'.");
        System.out.println("Test Case 2: Enter '12345'.");
        System.out.println("Test Case 3: Enter an empty string.");
        System.out.println("Test Case 4: Enter 'Special Chars: &<>\"'.");
        System.out.println("Test Case 5: Enter '<script>alert(\"XSS Test\")</script>'.");
    }

    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("GET".equals(exchange.getRequestMethod())) {
                String response = "<!DOCTYPE html>"
                        + "<html>"
                        + "<head><title>User Input Form</title></head>"
                        + "<body>"
                        + "<h2>Enter some text:</h2>"
                        + "<form action=\"/submit\" method=\"post\">"
                        + "<input type=\"text\" name=\"user_input\" size=\"50\">"
                        + "<br><br>"
                        + "<input type=\"submit\" value=\"Submit\">"
                        + "</form>"
                        + "</body>"
                        + "</html>";
                sendResponse(exchange, 200, response);
            } else {
                sendResponse(exchange, 405, "Method Not Allowed");
            }
        }
    }

    static class SubmitHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equals(exchange.getRequestMethod())) {
                InputStream is = exchange.getRequestBody();
                String body = new String(is.readAllBytes(), StandardCharsets.UTF_8);
                Map<String, String> params = parseFormData(body);
                String userInput = params.getOrDefault("user_input", "");
                
                // Rule #1: Validate and sanitize input. Here we use HTML escaping for sanitization.
                String sanitizedInput = htmlEscape(userInput);

                String response = "<!DOCTYPE html>"
                        + "<html>"
                        + "<head><title>Submission Result</title></head>"
                        + "<body>"
                        + "<h2>You submitted:</h2>"
                        + "<p style=\"font-family: monospace; background-color: #f0f0f0; padding: 1em;\">" 
                        + sanitizedInput 
                        + "</p>"
                        + "<a href=\"/\">Go back</a>"
                        + "</body>"
                        + "</html>";
                sendResponse(exchange, 200, response);
            } else {
                sendResponse(exchange, 405, "Method Not Allowed");
            }
        }
    }

    private static void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
        exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(response.getBytes(StandardCharsets.UTF_8));
        }
    }

    private static Map<String, String> parseFormData(String formData) throws UnsupportedEncodingException {
        Map<String, String> parameters = new HashMap<>();
        if (formData == null || formData.trim().isEmpty()) {
            return parameters;
        }
        String[] pairs = formData.split("&");
        for (String pair : pairs) {
            int idx = pair.indexOf("=");
            if (idx > 0) {
                String key = URLDecoder.decode(pair.substring(0, idx), StandardCharsets.UTF_8.name());
                String value = URLDecoder.decode(pair.substring(idx + 1), StandardCharsets.UTF_8.name());
                parameters.put(key, value);
            }
        }
        return parameters;
    }

    /**
     * Rule #2: Use caution with potentially dangerous functions.
     * This function sanitizes output to prevent Cross-Site Scripting (XSS).
     */
    private static String htmlEscape(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\"", "&quot;")
                    .replace("'", "&#39;");
    }
}