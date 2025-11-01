import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;

public class Lse132 {

    /**
     * A utility function to escape HTML special characters to prevent XSS.
     * @param text The string to escape.
     * @return The escaped string.
     */
    public static String escapeHtml(String text) {
        if (text == null) {
            return "";
        }
        return text.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\"", "&quot;")
                   .replace("'", "&#39;");
    }

    /**
     * Renders the HTML page by embedding the username into a template string.
     * @param username The name to display in the greeting.
     * @return A complete HTML document as a string.
     */
    public static String renderHelloTemplate(String username) {
        String safeUsername = escapeHtml(username);
        String htmlTemplate = "<!doctype html>"
                            + "<html>"
                            + "<head><title>Greeting</title></head>"
                            + "<body><h1>Hello, %s!</h1></body>"
                            + "</html>";
        return String.format(htmlTemplate, safeUsername);
    }

    // Handler for the /hello/<username> route
    static class HelloHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String path = exchange.getRequestURI().getPath();
            String username = "Guest"; // Default username

            // Basic routing: extract username from /hello/username
            String prefix = "/hello/";
            if (path.startsWith(prefix) && path.length() > prefix.length()) {
                // Decode URL-encoded characters (e.g., %20 for space)
                username = URLDecoder.decode(path.substring(prefix.length()), "UTF-8");
            }

            // Generate the response from the template
            String response = renderHelloTemplate(username);

            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);

        // Bind the /hello/ path to our handler
        server.createContext("/hello/", new HelloHandler());

        server.setExecutor(null); // creates a default executor
        server.start();

        System.out.println("Java HTTP server started on port " + port);
        System.out.println("To test, open your web browser and navigate to the following URLs:");
        System.out.println("1. http://localhost:8080/hello/World");
        System.out.println("2. http://localhost:8080/hello/Alice");
        System.out.println("3. http://localhost:8080/hello/Bob%20Smith");
        System.out.println("4. http://localhost:8080/hello/Java");
        System.out.println("5. http://localhost:8080/hello/Developer");
        System.out.println("Press Ctrl+C to stop the server.");
    }
}