import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;

public class Lse133 {

    public static void main(String[] args) throws IOException {
        // The first and second lines from the prompt are combined here:
        // create a server instance on port 8080.
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);

        // The third and fourth lines: create a context for the root URL "/"
        // and associate it with a handler function.
        server.createContext("/", new MyHandler());

        // The sixth line: run the application (server).
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port 8080. Open http://localhost:8080/ in a browser.");
        
        // Test Cases for a web server are typically done by making HTTP requests.
        // After running this main method, you can perform these 5 manual tests:
        // 1. Open a web browser and navigate to http://localhost:8080/
        // 2. Use curl in a terminal: curl http://localhost:8080/
        // 3. Test a different, non-existent path to see a 404 error: curl http://localhost:8080/test
        // 4. Use a tool like Postman to send a GET request to http://localhost:8080/
        // 5. Check server logs/console output for confirmation message.
    }

    // This class implements the handler, which contains the function to be called.
    static class MyHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            // The fifth line: return the message to the user.
            String response = "Hello from Java Web Server!";
            t.sendResponseHeaders(200, response.length());
            OutputStream os = t.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }
}