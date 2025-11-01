import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.nio.charset.StandardCharsets;

public class Lse132 {

    // The content of the "hello.html" template file is stored in a string.
    private static final String HELLO_TEMPLATE = 
        "<!DOCTYPE html>\n" +
        "<html>\n" +
        "<head>\n" +
        "    <title>Hello Page</title>\n" +
        "</head>\n" +
        "<body>\n" +
        "    <h1>Hello, {{username}}!</h1>\n" +
        "</body>\n" +
        "</html>";

    /**
     * Escapes special HTML characters in a string to prevent Cross-Site Scripting (XSS).
     * @param data The input string to escape.
     * @return The escaped string, safe for HTML rendering.
     */
    public static String htmlEscape(String data) {
        if (data == null) {
            return "";
        }
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < data.length(); i++) {
            char c = data.charAt(i);
            switch (c) {
                case '<': sb.append("&lt;"); break;
                case '>': sb.append("&gt;"); break;
                case '&': sb.append("&amp;"); break;
                case '"': sb.append("&quot;"); break;
                case '\'': sb.append("&#39;"); break;
                default: sb.append(c);
            }
        }
        return sb.toString();
    }

    /**
     * Renders the hello page by replacing the placeholder with the escaped username.
     * This function contains the core logic.
     * @param username The username to display.
     * @return The full HTML page as a string.
     */
    public static String renderHelloPage(String username) {
        String safeUsername = htmlEscape(username);
        return HELLO_TEMPLATE.replace("{{username}}", safeUsername);
    }
    
    // Handler class to process HTTP requests for the /hello/ URL pattern.
    static class HelloHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String path = exchange.getRequestURI().getPath();
            String responseBody = "404 Not Found";
            int statusCode = 404;

            String prefix = "/hello/";
            if (path != null && path.startsWith(prefix)) {
                // Extract username from the URL path
                String username = path.substring(prefix.length());
                if (!username.isEmpty()) {
                    responseBody = renderHelloPage(username);
                    statusCode = 200;
                } else {
                    responseBody = "Bad Request: Username cannot be empty.";
                    statusCode = 400;
                }
            }
            
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, responseBody.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBody.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Simple username
        String user1 = "World";
        System.out.println("Test Case 1 (Input: " + user1 + "):\n" + renderHelloPage(user1) + "\n");
        
        // Test Case 2: Username with spaces
        String user2 = "Alice Smith";
        System.out.println("Test Case 2 (Input: " + user2 + "):\n" + renderHelloPage(user2) + "\n");
        
        // Test Case 3: Numeric username
        String user3 = "12345";
        System.out.println("Test Case 3 (Input: " + user3 + "):\n" + renderHelloPage(user3) + "\n");
        
        // Test Case 4: Empty username
        String user4 = "";
        System.out.println("Test Case 4 (Input: " + user4 + "):\n" + renderHelloPage(user4) + "\n");
        
        // Test Case 5: Malicious XSS attempt (should be escaped)
        String user5 = "<script>alert('XSS')</script>";
        System.out.println("Test Case 5 (Input: " + user5 + "):\n" + renderHelloPage(user5) + "\n");
        
        System.out.println("--- Test Cases Finished ---");

        // The following code starts a simple web server to demonstrate the concept.
        // This part is for demonstration and might not run in all online environments.
        try {
            HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
            server.createContext("/hello/", new HelloHandler());
            server.setExecutor(null); // creates a default executor
            System.out.println("\nServer started on port 8000. Visit http://localhost:8000/hello/yourname");
            server.start();
        } catch (IOException e) {
            System.err.println("Could not start server: " + e.getMessage());
        }
    }
}