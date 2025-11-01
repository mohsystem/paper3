import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;

public class Lse126 {

    // Simulates the core logic of the http_hello_page function.
    public static String http_hello_page(String username) {
        // Security: Escape HTML special characters to prevent Cross-Site Scripting (XSS).
        String escapedUsername = escapeHTML(username);

        // Construct the response body (the HTML page) into the outputbuf variable.
        String outputbuf = "<html><head><title>Hello</title></head>"
                         + "<body><h1>Hello, " + escapedUsername + "!</h1></body></html>";
        return outputbuf;
    }
    
    /**
     * Escapes characters that have special meaning in HTML.
     * @param s The string to escape.
     * @return The escaped string.
     */
    private static String escapeHTML(String s) {
        if (s == null) {
            return "";
        }
        StringBuilder out = new StringBuilder(Math.max(16, s.length()));
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            if (c > 127 || c == '"' || c == '<' || c == '>' || c == '&' || c == '\'') {
                out.append("&#");
                out.append((int) c);
                out.append(';');
            } else {
                out.append(c);
            }
        }
        return out.toString();
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/hello", new HelloHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Run the following commands in another terminal to test:");
        
        // Test cases documentation
        System.out.println("\n--- 5 Test Cases ---");
        System.out.println("1. Normal name:");
        System.out.println("   curl -X POST -d \"World\" http://localhost:8080/hello");
        System.out.println("\n2. Name with spaces (should be URL encoded or quoted):");
        System.out.println("   curl -X POST -d \"Bob Smith\" http://localhost:8080/hello");
        System.out.println("\n3. Empty name:");
        System.out.println("   curl -X POST -d \"\" http://localhost:8080/hello");
        System.out.println("\n4. Malicious XSS attempt (should be safely escaped):");
        System.out.println("   curl -X POST -d \"<script>alert('xss')</script>\" http://localhost:8080/hello");
        System.out.println("\n5. Name with HTML special characters:");
        System.out.println("   curl -X POST -d \"O'Malley & Sons <Company>\" http://localhost:8080/hello");
    }

    static class HelloHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equals(exchange.getRequestMethod())) {
                exchange.sendResponseHeaders(405, -1); // 405 Method Not Allowed
                return;
            }

            // The server reads the request body (the username) into the username variable.
            // Security: Limit the size of the request body to prevent Denial of Service.
            int maxBodySize = 1024; 
            byte[] bodyBytes = new byte[maxBodySize];
            int bytesRead;
            String username;
            try (InputStream is = exchange.getRequestBody()) {
                bytesRead = is.read(bodyBytes, 0, maxBodySize);
                 // Check if body is larger than the buffer
                if (is.read() != -1) { 
                    String response = "Request body too large";
                    exchange.sendResponseHeaders(413, response.length()); // 413 Payload Too Large
                    try(OutputStream os = exchange.getResponseBody()) {
                        os.write(response.getBytes());
                    }
                    return;
                }
            }
            if (bytesRead <= 0) {
                username = "";
            } else {
                username = new String(bodyBytes, 0, bytesRead, StandardCharsets.UTF_8);
            }
            
            // Generate the HTML page using the core logic
            String response = http_hello_page(username);

            // The server writes the response body to the response stream.
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=utf-8");
            // Security: Add basic security headers
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            exchange.getResponseHeaders().set("Content-Security-Policy", "default-src 'self'");
            exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
            
            // The server closes the response stream. (Handled by try-with-resources and exchange.close())
            exchange.close();
        }
    }
}