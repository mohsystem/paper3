import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;
import java.util.List;
import java.util.Optional;

public class Lse42 {

    // In-memory session store. In a real application, use a more robust solution.
    private static final Map<String, Map<String, String>> sessions = new ConcurrentHashMap<>();

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/info", new InfoHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("--- How to Test ---");
        System.out.println("Run the following 5 tests in your terminal:");
        System.out.println("Test Case 1: First request to establish a session.");
        System.out.println("  curl -v http://localhost:8080/info");
        System.out.println("\nTest Case 2: Use the session cookie from Test 1 to retrieve data.");
        System.out.println("  (Replace 'yoursessionid' with the value from the 'Set-Cookie' header)");
        System.out.println("  curl --cookie \"session-id=yoursessionid\" http://localhost:8080/info");
        System.out.println("\nTest Case 3: Access an unknown route to get a 404.");
        System.out.println("  curl http://localhost:8080/other");
        System.out.println("\nTest Case 4: Simulate a different user by not sending a cookie (like a new browser).");
        System.out.println("  curl http://localhost:8080/info");
        System.out.println("\nTest Case 5: Send a fake session id.");
        System.out.println("  curl --cookie \"session-id=fakesession\" http://localhost:8080/info");
    }

    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            String sessionId = getSessionId(exchange);
            boolean newSession = false;

            if (sessionId == null || !sessions.containsKey(sessionId)) {
                sessionId = createSession();
                newSession = true;
            }

            Map<String, String> userData = sessions.get(sessionId);
            
            // SECURITY WARNING: Storing and transmitting sensitive data like SSN is extremely
            // dangerous and violates security best practices (e.g., PCI DSS, HIPAA).
            // This is for demonstration purposes only, based on the prompt.
            // In a real-world application, never store raw SSNs or similar sensitive
            // information in a session.
            String response = String.format("Username: %s; Email: %s; SSN: %s",
                userData.get("username"), userData.get("email"), userData.get("ssn"));

            if (newSession) {
                exchange.getResponseHeaders().set("Set-Cookie", "session-id=" + sessionId);
            }
            
            sendResponse(exchange, 200, response);
        }

        private String getSessionId(HttpExchange exchange) {
            List<String> cookies = exchange.getRequestHeaders().get("Cookie");
            if (cookies != null) {
                for (String cookieStr : cookies) {
                    String[] cookiePairs = cookieStr.split(";");
                    for (String pair : cookiePairs) {
                        String[] keyValue = pair.trim().split("=");
                        if (keyValue.length == 2 && "session-id".equals(keyValue[0])) {
                            return keyValue[1];
                        }
                    }
                }
            }
            return null;
        }

        private String createSession() {
            String sessionId = UUID.randomUUID().toString();
            Map<String, String> userData = new ConcurrentHashMap<>();
            userData.put("username", "testuser" + (sessions.size() + 1));
            userData.put("email", "test" + (sessions.size() + 1) + "@example.com");
            // SECURITY WARNING: Hardcoding sensitive data is a major security risk.
            userData.put("ssn", "000-00-" + String.format("%04d", sessions.size() + 1)); 
            sessions.put(sessionId, userData);
            return sessionId;
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.getResponseHeaders().set("Content-Type", "text/plain");
            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }
}