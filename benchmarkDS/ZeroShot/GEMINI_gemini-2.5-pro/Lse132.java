import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.net.URLDecoder;

public class Lse132 {

    /**
     * Performs HTML entity encoding to prevent Cross-Site Scripting (XSS).
     * @param input The raw string to escape.
     * @return The escaped string, safe for inclusion in HTML.
     */
    public static String escapeHtml(String input) {
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
                    escaped.append("&#x27;");
                    break;
                default:
                    escaped.append(c);
                    break;
            }
        }
        return escaped.toString();
    }

    /**
     * The handler for the /hello/<username> route. It extracts the username,
     * securely escapes it, and renders a simple HTML template.
     */
    static class HelloHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String path = exchange.getRequestURI().getPath();
            String username = "Guest"; // Default value
            String[] pathParts = path.split("/");
            if (pathParts.length > 2 && !pathParts[2].isEmpty()) {
                try {
                    // URL-decode the segment to handle spaces (%20), etc.
                    username = URLDecoder.decode(pathParts[2], StandardCharsets.UTF_8.name());
                } catch (UnsupportedEncodingException e) {
                    // This should not happen with UTF-8
                }
            }

            // Securely escape the username to prevent XSS attacks
            String safeUsername = escapeHtml(username);

            // This acts as our "template file" (hello.html) content.
            String response = String.format(
                "<!DOCTYPE html><html><head><title>Hello Page</title></head>" +
                "<body><h1>Hello, %s!</h1></body></html>",
                safeUsername
            );
            
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(200, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }

    /**
     * Main method to set up and start the server, and list test cases.
     * This code is self-contained and does not require any external libraries.
     */
    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        // This 'createContext' method binds the URL path to our handler.
        // It's the equivalent of a decorator for this built-in server.
        server.createContext("/hello/", new HelloHandler());
        
        server.setExecutor(null); // Creates a default executor
        server.start();
        
        String baseUrl = "http://127.0.0.1:" + port;
        System.out.println("Starting Java HTTP server on " + baseUrl);
        System.out.println("--- Test Cases ---");
        System.out.println("Please open the following URLs in your web browser:");
        System.out.println("1. " + baseUrl + "/hello/World");
        System.out.println("2. " + baseUrl + "/hello/Test%20User");
        System.out.println("3. " + baseUrl + "/hello/O'Malley&Sons");
        System.out.println("4. " + baseUrl + "/hello/12345");
        System.out.println("5. " + baseUrl + "/hello/<script>alert('XSS')</script>");
        System.out.println("\nPress Ctrl+C to stop the server.");
    }
}