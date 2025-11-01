
// Java doesn't have direct WSGI equivalent, but here's a similar web application using Spring Boot embedded server
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;

public class Lse133 {
    private HttpServer server;
    
    public Lse133() throws IOException {
        server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/", new RootHandler());
        server.setExecutor(null);
    }
    
    static class RootHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String message = getMessage();
            exchange.sendResponseHeaders(200, message.length());
            OutputStream os = exchange.getResponseBody();
            os.write(message.getBytes());
            os.close();
        }
    }
    
    public static String getMessage() {
        return "Hello, World!";
    }
    
    public void run() {
        server.start();
        System.out.println("Server started on port 8000");
    }
    
    public static void main(String[] args) {
        try {
            // Test cases
            System.out.println("Test Case 1: " + getMessage());
            System.out.println("Test Case 2: " + getMessage());
            System.out.println("Test Case 3: " + getMessage());
            System.out.println("Test Case 4: " + getMessage());
            System.out.println("Test Case 5: " + getMessage());
            
            // Uncomment to run the server
            // Lse133 app = new Lse133();
            // app.run();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
