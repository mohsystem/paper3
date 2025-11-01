
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;

public class Lse134 {
    
    // HTML escape function to prevent XSS
    public static String escape(String input) {
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
                case '\\'':
                    escaped.append("&#x27;");
                    break;
                case '/':
                    escaped.append("&#x2F;");
                    break;
                default:
                    escaped.append(c);
            }
        }
        return escaped.toString();
    }
    
    // Hello function that returns greeting with escaped username
    public static String hello(String username) {
        String sanitizedUsername = escape(username);
        return "Hello " + sanitizedUsername;
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test Cases:");
        System.out.println("1. " + hello("John"));
        System.out.println("2. " + hello("Alice<script>alert('XSS')</script>"));
        System.out.println("3. " + hello("Bob&Carol"));
        System.out.println("4. " + hello("User\\"123\\""));
        System.out.println("5. " + hello("Test'User"));
        
        // Create HTTP server on port 8000
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/hello/", new HelloHandler());
        server.setExecutor(null);
        System.out.println("\\nServer started on http://localhost:8000/hello/{username}");
        server.start();
    }
    
    static class HelloHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String path = exchange.getRequestURI().getPath();
            String username = path.substring("/hello/".length());
            String response = hello(username);
            
            exchange.sendResponseHeaders(200, response.getBytes().length);
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }
}
