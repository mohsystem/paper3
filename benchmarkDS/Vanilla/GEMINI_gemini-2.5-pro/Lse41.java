import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;

public class Lse41 {

    // Hardcoded user data
    private static final String USERNAME = "johndoe";
    private static final String EMAIL = "j.doe@example.com";
    private static final String SSN_LAST_4 = "5678";

    public static void main(String[] args) throws IOException {
        // Create an HTTP server on port 8080
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        // Create a context for the /info route
        server.createContext("/info", new InfoHandler());
        
        // Start the server
        server.setExecutor(null); // creates a default executor
        server.start();
        
        System.out.println("Java server started on port " + port);
        System.out.println("Visit http://localhost:" + port + "/info to see the user information.");
        
        // Main method with 5 test cases (instructions for manual testing)
        System.out.println("\n--- How to Test ---");
        System.out.println("Test Case 1: Open a web browser and navigate to http://localhost:8080/info");
        System.out.println("Test Case 2: Use curl in your terminal: curl http://localhost:8080/info");
        System.out.println("Test Case 3: Use a tool like Postman to send a GET request to http://localhost:8080/info");
        System.out.println("Test Case 4: Use PowerShell Invoke-WebRequest: Invoke-WebRequest -Uri http://localhost:8080/info");
        System.out.println("Test Case 5: Use Python requests library: python -c \"import requests; print(requests.get('http://localhost:8080/info').text)\"");
    }

    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = getInfoPage();
            
            // Set response headers
            exchange.sendResponseHeaders(200, response.getBytes().length);
            
            // Write response body
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }
    
    public static String getInfoPage() {
        return "<html>" +
               "<head><title>User Information</title></head>" +
               "<body>" +
               "<h1>User Information</h1>" +
               "<ul>" +
               "<li><strong>Username:</strong> " + USERNAME + "</li>" +
               "<li><strong>Email:</strong> " + EMAIL + "</li>" +
               "<li><strong>SSN (Last 4 digits):</strong> " + SSN_LAST_4 + "</li>" +
               "</ul>" +
               "</body>" +
               "</html>";
    }
}