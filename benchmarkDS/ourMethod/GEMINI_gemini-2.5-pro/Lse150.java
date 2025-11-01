import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

public class Lse150 {
    private static final int MAX_USERNAME_LENGTH = 32;
    private static final int MAX_MESSAGE_LENGTH = 256;
    // Simple alphanumeric pattern for username validation
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]+$");

    // Mock database insertion function
    public static boolean insert_user_message_in_db(String username, String message) {
        System.out.println("DATABASE: Inserting message for user '" + username + "': '" + message + "'");
        // In a real application, this would interact with a database.
        // Returning true to simulate success.
        return true;
    }

    // The core post logic, separated for testing
    public static String handle_post_logic(String username, String message) {
        if (username == null || username.trim().isEmpty()) {
            return "Username cannot be empty.";
        }
        if (message == null || message.trim().isEmpty()) {
            return "Message cannot be empty.";
        }
        if (username.length() > MAX_USERNAME_LENGTH) {
            return "Username is too long.";
        }
        if (message.length() > MAX_MESSAGE_LENGTH) {
            return "Message is too long.";
        }
        if (!USERNAME_PATTERN.matcher(username).matches()) {
            return "Username contains invalid characters.";
        }
        
        // Sanitize message for storage (though here we just print it).
        // In a real app, use libraries like OWASP Java Encoder.
        // For this example, we assume the DB layer handles SQL injection.
        
        if (insert_user_message_in_db(username, message)) {
            return "Success";
        } else {
            return "Failed to save message.";
        }
    }

    // Handler for the main page, displaying the form
    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                exchange.sendResponseHeaders(405, -1); // Method Not Allowed
                return;
            }
            String formHtml = "<!DOCTYPE html>"
                            + "<html><head><title>Message Board</title></head>"
                            + "<body><h1>Post a Message</h1>"
                            + "<form action=\"/post\" method=\"post\">"
                            + "<label for=\"username\">Username:</label><br>"
                            + "<input type=\"text\" id=\"username\" name=\"username\" required maxlength=\"" + MAX_USERNAME_LENGTH + "\"><br><br>"
                            + "<label for=\"message\">Message:</label><br>"
                            + "<textarea id=\"message\" name=\"message\" rows=\"4\" cols=\"50\" required maxlength=\"" + MAX_MESSAGE_LENGTH + "\"></textarea><br><br>"
                            + "<input type=\"submit\" value=\"Submit\">"
                            + "</form></body></html>";
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=utf-8");
            exchange.sendResponseHeaders(200, formHtml.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(formHtml.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    // Handler for the POST request
    static class PostHandler implements HttpHandler {
        private static Map<String, String> parseFormData(String body) throws java.io.UnsupportedEncodingException {
            Map<String, String> params = new HashMap<>();
            if (body == null || body.isEmpty()) {
                return params;
            }
            String[] pairs = body.split("&");
            for (String pair : pairs) {
                int idx = pair.indexOf("=");
                if (idx > 0) {
                     params.put(URLDecoder.decode(pair.substring(0, idx), "UTF-8"), URLDecoder.decode(pair.substring(idx + 1), "UTF-8"));
                }
            }
            return params;
        }

        // This function is the equivalent of the `post()` function from the prompt
        public void post(HttpExchange exchange) throws IOException {
             if (!"POST".equals(exchange.getRequestMethod())) {
                exchange.sendResponseHeaders(405, -1); // Method Not Allowed
                return;
            }
            
            // Read and parse the request body
            String body;
            try (InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
                 BufferedReader br = new BufferedReader(isr)) {
                body = br.lines().collect(Collectors.joining(System.lineSeparator()));
            }

            Map<String, String> params = parseFormData(body);
            String username = params.get("username");
            String message = params.get("message");
            
            String result = handle_post_logic(username, message);
            
            if ("Success".equals(result)) {
                // Redirect back to the main page
                exchange.getResponseHeaders().set("Location", "/");
                exchange.sendResponseHeaders(302, -1); // 302 Found for redirect
            } else {
                // Handle error case - for simplicity, we'll send a 400 Bad Request
                String errorResponse = "<html><body><h1>Error: " + result + "</h1><a href=\"/\">Go back</a></body></html>";
                exchange.getResponseHeaders().set("Content-Type", "text/html; charset=utf-8");
                exchange.sendResponseHeaders(400, errorResponse.getBytes(StandardCharsets.UTF_8).length);
                 try (OutputStream os = exchange.getResponseBody()) {
                    os.write(errorResponse.getBytes(StandardCharsets.UTF_8));
                }
            }
        }
        
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            post(exchange);
        }
    }
    
    public static void runTests() {
        System.out.println("--- Running Test Cases ---");
        // Test Case 1: Valid input
        System.out.println("Test 1: " + handle_post_logic("testuser", "Hello World!"));
        // Test Case 2: Empty username
        System.out.println("Test 2: " + handle_post_logic("", "A message from nowhere."));
        // Test Case 3: Empty message
        System.out.println("Test 3: " + handle_post_logic("anotheruser", ""));
        // Test Case 4: Username too long
        String longUsername = new String(new char[MAX_USERNAME_LENGTH + 1]).replace('\0', 'a');
        System.out.println("Test 4: " + handle_post_logic(longUsername, "Short message."));
        // Test Case 5: Message too long
        String longMessage = new String(new char[MAX_MESSAGE_LENGTH + 1]).replace('\0', 'b');
        System.out.println("Test 5: " + handle_post_logic("shortuser", longMessage));
        System.out.println("--- Test Cases Finished ---\n");
    }

    public static void main(String[] args) throws IOException {
        runTests();
        
        int port = 8000;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        System.out.println("Server started on port " + port);
        System.out.println("Open http://localhost:" + port + " in your browser.");
        server.createContext("/", new RootHandler());
        server.createContext("/post", new PostHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
    }
}