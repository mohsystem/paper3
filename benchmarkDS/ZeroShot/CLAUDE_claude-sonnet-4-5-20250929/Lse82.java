
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import org.json.JSONObject;

public class Lse82 {
    
    public static String processPayload(String payload) {
        try {
            // Decode the URL-encoded payload
            String decodedPayload = URLDecoder.decode(payload, StandardCharsets.UTF_8.toString());
            
            // Parse JSON payload
            JSONObject jsonObject = new JSONObject(decodedPayload);
            
            // Return the name value from the dictionary
            if (jsonObject.has("name")) {
                return jsonObject.getString("name");
            } else {
                return "Name not found";
            }
        } catch (Exception e) {
            return "Error processing payload: " + e.getMessage();
        }
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test Cases:");
        System.out.println("1. " + processPayload("%7B%22name%22%3A%22John%22%7D"));
        System.out.println("2. " + processPayload("%7B%22name%22%3A%22Alice%22%2C%22age%22%3A30%7D"));
        System.out.println("3. " + processPayload("%7B%22age%22%3A25%7D"));
        System.out.println("4. " + processPayload("%7B%22name%22%3A%22Bob%20Smith%22%7D"));
        System.out.println("5. " + processPayload("invalid"));
        
        // Create HTTP server
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/api", new ApiHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("\\nServer started on port 8000");
    }
    
    static class ApiHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            String response = "Invalid request";
            
            if (query != null) {
                Map<String, String> params = queryToMap(query);
                String payload = params.get("payload");
                
                if (payload != null) {
                    response = processPayload(payload);
                }
            }
            
            exchange.sendResponseHeaders(200, response.getBytes().length);
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
        
        private Map<String, String> queryToMap(String query) {
            Map<String, String> result = new HashMap<>();
            for (String param : query.split("&")) {
                String[] entry = param.split("=");
                if (entry.length > 1) {
                    result.put(entry[0], entry[1]);
                } else {
                    result.put(entry[0], "");
                }
            }
            return result;
        }
    }
}
