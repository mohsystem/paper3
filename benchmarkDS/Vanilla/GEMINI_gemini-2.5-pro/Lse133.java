import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;

// 1. Imports are similar to importing a web application class.
// Here we import Java's built-in lightweight HTTP server.

public class Lse133 {

    // 4. This is the handler function that is called when a user visits a specific URL.
    static class MyHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            // 5. This line prepares and returns the message to the user.
            String response = "Hello from Java's built-in HttpServer!";
            t.sendResponseHeaders(200, response.length());
            OutputStream os = t.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }
    
    // A simple function that accepts a parameter and returns output.
    public static String createGreeting(String name) {
        return "Hello, " + name + "!";
    }

    public static void main(String[] args) throws Exception {
        System.out.println("--- Running 5 test cases for the helper function ---");
        String[] testCases = {"Alice", "Bob", "Charlie", "David", "Eve"};
        for (int i = 0; i < testCases.length; i++) {
            System.out.printf("Test Case %d: createGreeting(\"%s\") -> \"%s\"\n", 
                              i + 1, testCases[i], createGreeting(testCases[i]));
        }
        System.out.println("--------------------------------------------------\n");
        
        // 2. The second line creates the application (server) object.
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        
        // 3. This line tells the application which URL should call the associated function (handler).
        server.createContext("/", new MyHandler());
        server.setExecutor(null); // creates a default executor
        
        // 6. This line runs the application.
        server.start();
        
        System.out.println("Java HTTP server started on port 8080.");
        System.out.println("Open your web browser and navigate to http://localhost:8080");
    }
}