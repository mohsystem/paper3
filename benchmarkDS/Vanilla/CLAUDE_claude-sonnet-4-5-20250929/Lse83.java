
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import org.json.JSONObject;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;

public class Lse83 {
    
    public static String processPayload(String payload) {
        try {
            JSONObject json = new JSONObject(payload);
            if (json.has("name")) {
                return json.getString("name");
            }
            return "Field 'name' not found";
        } catch (Exception e) {
            return "Invalid JSON payload";
        }
    }
    
    public static void main(String[] args) throws IOException {
        // Test cases
        System.out.println("Test Case 1: " + processPayload("{\\"name\\":\\"John Doe\\"}"));
        System.out.println("Test Case 2: " + processPayload("{\\"name\\":\\"Alice\\",\\"age\\":30}"));
        System.out.println("Test Case 3: " + processPayload("{\\"age\\":25}"));
        System.out.println("Test Case 4: " + processPayload("{\\"name\\":\\"\\"}"));
        System.out.println("Test Case 5: " + processPayload("invalid json"));
        
        // Create HTTP server
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/api", new MyHandler());
        server.setExecutor(null);
        System.out.println("\\nServer started on port 8000");
        server.start();
    }
    
    static class MyHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equals(exchange.getRequestMethod())) {
                InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
                BufferedReader br = new BufferedReader(isr);
                StringBuilder payload = new StringBuilder();
                String line;
                while ((line = br.readLine()) != null) {
                    payload.append(line);
                }
                
                String response = processPayload(payload.toString());
                exchange.sendResponseHeaders(200, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        }
    }
}
