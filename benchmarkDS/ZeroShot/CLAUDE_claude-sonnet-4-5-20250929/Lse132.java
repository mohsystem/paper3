
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.net.URLDecoder;

public class Lse132 {
    
    // Simulated template rendering method
    private static String renderTemplate(String templateName, String username) {
        // In a real application, you would read from an actual template file
        // This simulates reading hello.html and replacing placeholders
        String template = """\n                <!DOCTYPE html>\n                <html>\n                <head>\n                    <title>Hello Page</title>\n                </head>\n                <body>\n                    <h1>Hello, %s!</h1>\n                    <p>Welcome to our secure application.</p>\n                </body>\n                </html>\n                """;
        
        // Escape HTML to prevent XSS attacks
        String safeUsername = escapeHtml(username);
        return String.format(template, safeUsername);
    }
    
    // HTML escape function for security
    private static String escapeHtml(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;");
    }
    
    // Handler for /hello/<username> route
    static class HelloHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String path = exchange.getRequestURI().getPath();
            
            if (path.startsWith("/hello/")) {
                String username = path.substring("/hello/".length());
                username = URLDecoder.decode(username, StandardCharsets.UTF_8);
                
                String response = renderTemplate("hello.html", username);
                
                exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
                exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
                
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes(StandardCharsets.UTF_8));
                os.close();
            } else {
                String response = "404 Not Found";
                exchange.sendResponseHeaders(404, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        }
    }
    
    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/hello/", new HelloHandler());
        server.setExecutor(null);
        server.start();
        
        System.out.println("Server started on port 8000");
        System.out.println("\\nTest cases (open in browser):");
        System.out.println("1. http://localhost:8000/hello/John");
        System.out.println("2. http://localhost:8000/hello/Alice");
        System.out.println("3. http://localhost:8000/hello/Bob123");
        System.out.println("4. http://localhost:8000/hello/User%20Name");
        System.out.println("5. http://localhost:8000/hello/TestUser");
    }
}
