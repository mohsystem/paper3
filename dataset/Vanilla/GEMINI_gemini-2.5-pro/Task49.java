import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.InetSocketAddress;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;

public class Task49 {

    // In-memory database
    private static final List<User> userDatabase = new ArrayList<>();

    // Simple User record
    private static class User {
        private final String name;
        private final String email;

        public User(String name, String email) {
            this.name = name;
            this.email = email;
        }

        @Override
        public String toString() {
            return "User{name='" + name + "', email='" + email + "'}";
        }
    }

    // HTTP Handler for /api/users endpoint
    static class UserHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equals(exchange.getRequestMethod())) {
                // Read request body
                InputStream is = exchange.getRequestBody();
                String body = new String(is.readAllBytes(), StandardCharsets.UTF_8);

                // Simple JSON parsing (not robust)
                try {
                    String name = body.split(",")[0].split(":")[1].replace("\"", "").trim();
                    String email = body.split(",")[1].split(":")[1].replace("\"", "").replace("}", "").trim();
                    
                    User newUser = new User(name, email);
                    userDatabase.add(newUser);

                    String response = "User created successfully.";
                    exchange.sendResponseHeaders(201, response.length());
                    OutputStream os = exchange.getResponseBody();
                    os.write(response.getBytes());
                    os.close();
                } catch (Exception e) {
                    String response = "Invalid JSON format.";
                    exchange.sendResponseHeaders(400, response.length());
                    OutputStream os = exchange.getResponseBody();
                    os.write(response.getBytes());
                    os.close();
                }
            } else {
                String response = "Method not allowed. Use POST.";
                exchange.sendResponseHeaders(405, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api/users", new UserHandler());
        server.setExecutor(Executors.newSingleThreadExecutor()); // Use a single thread
        server.start();

        System.out.println("Server started on port " + port);
        System.out.println("Running 5 test cases...");

        // Run test cases in a separate thread to not block the main thread
        new Thread(() -> {
            try {
                // Wait for server to start up
                Thread.sleep(1000);
                
                // Test Cases
                sendPostRequest("{\"name\": \"Alice\", \"email\": \"alice@example.com\"}");
                sendPostRequest("{\"name\": \"Bob\", \"email\": \"bob@example.com\"}");
                sendPostRequest("{\"name\": \"Charlie\", \"email\": \"charlie@example.com\"}");
                sendPostRequest("{\"name\": \"David\", \"email\": \"david@example.com\"}");
                sendPostRequest("{\"name\": \"Eve\", \"email\": \"eve@example.com\"}");
                
                System.out.println("\n--- Current Database State ---");
                for(User u : userDatabase) {
                    System.out.println(u);
                }
                System.out.println("----------------------------");

            } catch (Exception e) {
                e.printStackTrace();
            } finally {
                 // In a real app, you might stop the server after tests.
                 // For this example, we leave it running to allow manual testing.
                 System.out.println("Test cases finished. Server is still running. Press Ctrl+C to stop.");
                 // server.stop(0); 
            }
        }).start();
    }
    
    private static void sendPostRequest(String jsonPayload) throws IOException {
        URL url = new URL("http://localhost:8080/api/users");
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("POST");
        conn.setRequestProperty("Content-Type", "application/json; utf-8");
        conn.setDoOutput(true);
        
        try (OutputStream os = conn.getOutputStream()) {
            byte[] input = jsonPayload.getBytes(StandardCharsets.UTF_8);
            os.write(input, 0, input.length);
        }
        
        int responseCode = conn.getResponseCode();
        System.out.println("POST request to /api/users with payload " + jsonPayload + " ... Response Code: " + responseCode);
        
        // Clean up
        conn.disconnect();
    }
}