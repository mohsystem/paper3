
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URL;
import java.net.MalformedURLException;

public class Lse33 {
    
    public static String redirectInvalidUrl(String urlString) {
        if (urlString == null || urlString.trim().isEmpty()) {
            return "Error: URL parameter is required";
        }
        
        try {
            URL url = new URL(urlString);
            String host = url.getHost().toLowerCase();
            
            // Strict domain validation - only allow example.com
            if ("example.com".equals(host) || host.endsWith(".example.com")) {
                return "REDIRECT:" + urlString;
            } else {
                return "Error: Invalid domain. Only example.com is allowed";
            }
        } catch (MalformedURLException e) {
            return "Error: Invalid URL format";
        }
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test 1: " + redirectInvalidUrl("https://example.com/page"));
        System.out.println("Test 2: " + redirectInvalidUrl("https://www.example.com/page"));
        System.out.println("Test 3: " + redirectInvalidUrl("https://evil.com"));
        System.out.println("Test 4: " + redirectInvalidUrl("https://example.com.evil.com"));
        System.out.println("Test 5: " + redirectInvalidUrl("invalid-url"));
        
        // Simple HTTP server
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/redirect", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                String query = exchange.getRequestURI().getQuery();
                String url = null;
                if (query != null && query.startsWith("url=")) {
                    url = query.substring(4);
                }
                
                String result = redirectInvalidUrl(url);
                String response;
                int statusCode;
                
                if (result.startsWith("REDIRECT:")) {
                    statusCode = 302;
                    exchange.getResponseHeaders().set("Location", result.substring(9));
                    response = "Redirecting...";
                } else {
                    statusCode = 400;
                    response = result;
                }
                
                exchange.sendResponseHeaders(statusCode, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        });
        server.start();
        System.out.println("Server started on port 8000");
    }
}
