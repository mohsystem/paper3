
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;

public class Lse126 {
    
    public static void http_hello_page(HttpExchange exchange) throws IOException {
        // Read the request body (username)
        InputStream inputStream = exchange.getRequestBody();
        byte[] buffer = new byte[1024];
        int bytesRead = inputStream.read(buffer);
        String username = "";
        if (bytesRead > 0) {
            username = new String(buffer, 0, bytesRead, StandardCharsets.UTF_8);
        }
        
        // Construct the response body (HTML page)
        String outputbuf = "<!DOCTYPE html>\\n" +
                          "<html>\\n" +
                          "<head><title>Hello Page</title></head>\\n" +
                          "<body>\\n" +
                          "<h1>Hello, " + username + "!</h1>\\n" +
                          "<p>Welcome to our server.</p>\\n" +
                          "</body>\\n" +
                          "</html>";
        
        // Set response headers
        exchange.getResponseHeaders().set("Content-Type", "text/html");
        exchange.sendResponseHeaders(200, outputbuf.length());
        
        // Write the response body to the response stream
        OutputStream outputStream = exchange.getResponseBody();
        outputStream.write(outputbuf.getBytes(StandardCharsets.UTF_8));
        
        // Close the response stream
        outputStream.close();
    }
    
    public static void main(String[] args) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        
        server.createContext("/hello", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                http_hello_page(exchange);
            }
        });
        
        server.setExecutor(null);
        server.start();
        
        System.out.println("Server started on port 8080");
        System.out.println("Test the server with:");
        System.out.println("curl -X POST -d 'Alice' http://localhost:8080/hello");
        System.out.println("curl -X POST -d 'Bob' http://localhost:8080/hello");
        System.out.println("curl -X POST -d 'Charlie' http://localhost:8080/hello");
        System.out.println("curl -X POST -d 'David' http://localhost:8080/hello");
        System.out.println("curl -X POST -d 'Eve' http://localhost:8080/hello");
    }
}
