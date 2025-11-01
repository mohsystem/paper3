
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;

public class Lse126 {
    
    public static void httpHelloPage(HttpExchange exchange) throws IOException {
        // Read request body (username)
        InputStream inputStream = exchange.getRequestBody();
        byte[] requestBodyBytes = inputStream.readAll();
        String username = new String(requestBodyBytes, StandardCharsets.UTF_8).trim();
        
        // Sanitize username to prevent XSS attacks
        username = sanitizeInput(username);
        
        // Construct response body (HTML page)
        String outputbuf = "<!DOCTYPE html>\\n" +
                "<html>\\n" +
                "<head>\\n" +
                "    <title>Hello Page</title>\\n" +
                "</head>\\n" +
                "<body>\\n" +
                "    <h1>Hello, " + username + "!</h1>\\n" +
                "</body>\\n" +
                "</html>";
        
        // Write response
        exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
        byte[] responseBytes = outputbuf.getBytes(StandardCharsets.UTF_8);
        exchange.sendResponseHeaders(200, responseBytes.length);
        
        OutputStream outputStream = exchange.getResponseBody();
        outputStream.write(responseBytes);
        outputStream.close();
    }
    
    private static String sanitizeInput(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }
    
    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        
        server.createContext("/hello", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                httpHelloPage(exchange);
            }
        });
        
        server.setExecutor(null);
        server.start();
        
        System.out.println("Server started on port 8000");
        System.out.println("Test with: curl -X POST -d 'TestUser' http://localhost:8000/hello");
    }
}
