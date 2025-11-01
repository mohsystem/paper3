import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse40 {

    // In-memory store for session data. Key: sessionID, Value: user data map
    private static final Map<String, Map<String, String>> sessions = new ConcurrentHashMap<>();

    public static void main(String[] args) throws IOException, InterruptedException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);

        // Route to set session data for a test user
        server.createContext("/login", new LoginHandler());
        // The main route to get user info
        server.createContext("/info", new InfoHandler());

        server.setExecutor(Executors.newCachedThreadPool());
        
        // Start server in a separate thread to allow main thread to run tests
        Thread serverThread = new Thread(server::start);
        serverThread.start();

        System.out.println("Server started on port " + port);
        System.out.println("Running 5 test cases...");
        runTests(port);

        // Stop the server
        System.out.println("Tests finished. Stopping server...");
        server.stop(1);
        serverThread.join();
        System.out.println("Server stopped.");
    }

    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String cookieHeader = exchange.getRequestHeaders().getFirst("Cookie");
            String sessionId = parseSessionId(cookieHeader);
            String response;
            int statusCode = 200;

            if (sessionId != null && sessions.containsKey(sessionId)) {
                Map<String, String> userData = sessions.get(sessionId);
                String username = userData.get("username");
                String email = userData.get("email");
                String ssn = userData.get("ssn");
                String last4Ssn = ssn.substring(ssn.length() - 4);

                response = String.format(
                    "<html><body>" +
                    "<h1>User Information</h1>" +
                    "<p>Username: %s</p>" +
                    "<p>Email: %s</p>" +
                    "<p>Last 4 digits of SSN: %s</p>" +
                    "</body></html>",
                    username, email, last4Ssn
                );
            } else {
                statusCode = 401; // Unauthorized
                response = "<html><body><h1>Access Denied</h1><p>Please <a href='/login'>login</a> first.</p></body></html>";
            }

            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }

    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String sessionId = UUID.randomUUID().toString();
            Map<String, String> userData = new HashMap<>();
            userData.put("username", "johndoe");
            userData.put("email", "john.doe@example.com");
            userData.put("ssn", "123-456-7890");
            sessions.put(sessionId, userData);

            exchange.getResponseHeaders().add("Set-Cookie", "session_id=" + sessionId + "; Path=/");
            String response = "<html><body><h1>Login Successful</h1><p>Session has been set. Go to <a href='/info'>/info</a>.</p></body></html>";
            exchange.sendResponseHeaders(200, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }
    
    private static String parseSessionId(String cookieHeader) {
        if (cookieHeader == null || cookieHeader.isEmpty()) {
            return null;
        }
        String[] cookies = cookieHeader.split(";");
        for (String cookie : cookies) {
            String[] parts = cookie.trim().split("=");
            if (parts.length == 2 && "session_id".equals(parts[0])) {
                return parts[1];
            }
        }
        return null;
    }

    private static void runTests(int port) {
        HttpClient client = HttpClient.newHttpClient();
        String baseUrl = "http://localhost:" + port;
        String sessionCookie = null;

        try {
            // Test Case 1: Access /info without a session
            System.out.println("\n--- Test Case 1: Access /info without session ---");
            HttpRequest request1 = HttpRequest.newBuilder().uri(URI.create(baseUrl + "/info")).build();
            HttpResponse<String> response1 = client.send(request1, HttpResponse.BodyHandlers.ofString());
            System.out.println("Status Code: " + response1.statusCode());
            System.out.println("Body contains 'Access Denied': " + response1.body().contains("Access Denied"));
            assert response1.statusCode() == 401;

            // Test Case 2: Access /login to get a session cookie
            System.out.println("\n--- Test Case 2: Access /login to create session ---");
            HttpRequest request2 = HttpRequest.newBuilder().uri(URI.create(baseUrl + "/login")).build();
            HttpResponse<String> response2 = client.send(request2, HttpResponse.BodyHandlers.ofString());
            sessionCookie = response2.headers().firstValue("Set-Cookie").orElse(null);
            System.out.println("Status Code: " + response2.statusCode());
            System.out.println("Received Cookie: " + (sessionCookie != null));
            assert response2.statusCode() == 200 && sessionCookie != null;

            // Test Case 3: Access /info with the session cookie
            System.out.println("\n--- Test Case 3: Access /info with session ---");
            HttpRequest request3 = HttpRequest.newBuilder().uri(URI.create(baseUrl + "/info")).header("Cookie", sessionCookie).build();
            HttpResponse<String> response3 = client.send(request3, HttpResponse.BodyHandlers.ofString());
            System.out.println("Status Code: " + response3.statusCode());
            System.out.println("Body contains 'johndoe': " + response3.body().contains("johndoe"));
            System.out.println("Body contains '7890': " + response3.body().contains("7890"));
            assert response3.statusCode() == 200 && response3.body().contains("johndoe");

            // Test Case 4: Simulate a different user logging in (re-using /login for simplicity)
            System.out.println("\n--- Test Case 4: Simulate new login, get new session ---");
            // In a real app, /login would take parameters. Here we just get a new session.
             sessions.clear(); // Clear old sessions for a clean test
            Map<String, String> newUser = new HashMap<>();
            newUser.put("username", "janedoe");
            newUser.put("email", "jane.doe@example.com");
            newUser.put("ssn", "987-654-3210");
            String newSessionId = UUID.randomUUID().toString();
            sessions.put(newSessionId, newUser);
            String newSessionCookie = "session_id=" + newSessionId;
            System.out.println("Simulated login for 'janedoe'.");
            assert true;


            // Test Case 5: Access /info with the new user's session
            System.out.println("\n--- Test Case 5: Access /info with new session ---");
            HttpRequest request5 = HttpRequest.newBuilder().uri(URI.create(baseUrl + "/info")).header("Cookie", newSessionCookie).build();
            HttpResponse<String> response5 = client.send(request5, HttpResponse.BodyHandlers.ofString());
            System.out.println("Status Code: " + response5.statusCode());
            System.out.println("Body contains 'janedoe': " + response5.body().contains("janedoe"));
            System.out.println("Body contains '3210': " + response5.body().contains("3210"));
            assert response5.statusCode() == 200 && response5.body().contains("janedoe");


        } catch (Exception e) {
            System.err.println("An error occurred during tests: " + e.getMessage());
            e.printStackTrace();
        }
    }
}