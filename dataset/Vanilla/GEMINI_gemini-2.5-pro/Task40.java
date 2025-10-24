import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.util.UUID;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

public class Task40 {

    // In-memory storage for session CSRF tokens and user data.
    // In a real application, this would be a persistent store or a distributed cache.
    private static final Map<String, String> sessionCsrfTokens = new ConcurrentHashMap<>();
    private static final Map<String, String> userSettings = new ConcurrentHashMap<>();

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/settings", new SettingsHandler());
        server.createContext("/update-settings", new UpdateHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("--- CSRF Protection Test Cases ---");
        System.out.println("Open your web browser and follow these steps:");
        System.out.println("\n1. Test Case 1 (Normal Load):");
        System.out.println("   - Go to http://localhost:8080/settings");
        System.out.println("   - The user settings form should be displayed.");
        
        System.out.println("\n2. Test Case 2 (Valid Submission):");
        System.out.println("   - On the form, enter a new username (e.g., 'test_user').");
        System.out.println("   - Click 'Update Settings'.");
        System.out.println("   - You should see a 'Settings updated successfully!' message.");
        System.out.println("   - Go back to http://localhost:8080/settings, the new username should be pre-filled.");
        
        System.out.println("\n3. Test Case 3 (Invalid CSRF Token):");
        System.out.println("   - Load the form at http://localhost:8080/settings.");
        System.out.println("   - Use browser developer tools to change the value of the hidden 'csrf_token' input to 'invalid_token'.");
        System.out.println("   - Submit the form.");
        System.out.println("   - You should see a 'CSRF Token Invalid' error (403 Forbidden).");
        
        System.out.println("\n4. Test Case 4 (Missing CSRF Token):");
        System.out.println("   - Load the form at http://localhost:8080/settings.");
        System.out.println("   - Use browser developer tools to delete the hidden 'csrf_token' input field entirely.");
        System.out.println("   - Submit the form.");
        System.out.println("   - You should see a 'CSRF Token Invalid' error (403 Forbidden).");

        System.out.println("\n5. Test Case 5 (Using Stale/Old Token):");
        System.out.println("   - Open http://localhost:8080/settings in two separate browser tabs (Tab A and Tab B).");
        System.out.println("   - In Tab A, submit a valid change (e.g., username 'user_a').");
        System.out.println("   - Now, go to Tab B (do not refresh it). The form in Tab B still has the old CSRF token.");
        System.out.println("   - In Tab B, try to submit a change (e.g., username 'user_b').");
        System.out.println("   - The submission from Tab B should fail with a 'CSRF Token Invalid' error because the token was already used/invalidated by the submission from Tab A.");
    }

    static class SettingsHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String sessionId = getOrCreateSessionId(exchange);
            // Generate a new CSRF token for the form page
            String csrfToken = UUID.randomUUID().toString();
            sessionCsrfTokens.put(sessionId, csrfToken);

            String currentUsername = userSettings.getOrDefault(sessionId, "DefaultUser");

            String htmlResponse = "<html>" +
                "<head><title>User Settings</title></head>" +
                "<body>" +
                "<h2>Update User Settings</h2>" +
                "<form action='/update-settings' method='post'>" +
                "<input type='hidden' name='csrf_token' value='" + csrfToken + "' />" +
                "<label for='username'>Username:</label><br>" +
                "<input type='text' id='username' name='username' value='" + currentUsername + "'><br><br>" +
                "<input type='submit' value='Update Settings'>" +
                "</form>" +
                "</body>" +
                "</html>";

            sendResponse(exchange, 200, htmlResponse);
        }
    }

    static class UpdateHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            String sessionId = getSessionId(exchange);
            if (sessionId == null || !sessionCsrfTokens.containsKey(sessionId)) {
                sendResponse(exchange, 403, "CSRF Token Invalid (No active session)");
                return;
            }

            String requestBody = new BufferedReader(new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8))
                .lines().collect(Collectors.joining("\n"));
            Map<String, String> params = parseFormData(requestBody);

            String submittedToken = params.get("csrf_token");
            String expectedToken = sessionCsrfTokens.get(sessionId);

            if (expectedToken == null || !expectedToken.equals(submittedToken)) {
                sendResponse(exchange, 403, "CSRF Token Invalid");
                return;
            }
            
            // CSRF token is valid. Invalidate it to prevent reuse and process the request.
            sessionCsrfTokens.remove(sessionId); 
            
            String newUsername = params.get("username");
            if(newUsername != null && !newUsername.trim().isEmpty()){
                userSettings.put(sessionId, newUsername);
            }

            String successResponse = "<html><body>" +
                                     "<h2>Settings updated successfully!</h2>" +
                                     "<a href='/settings'>Go back to settings</a>" +
                                     "</body></html>";
            sendResponse(exchange, 200, successResponse);
        }
    }
    
    private static String getOrCreateSessionId(HttpExchange exchange) {
        String sessionId = getSessionId(exchange);
        if (sessionId == null) {
            sessionId = UUID.randomUUID().toString();
            // Set cookie for new session
            exchange.getResponseHeaders().add("Set-Cookie", "session_id=" + sessionId + "; HttpOnly; Path=/");
        }
        return sessionId;
    }

    private static String getSessionId(HttpExchange exchange) {
        List<String> cookies = exchange.getRequestHeaders().get("Cookie");
        if (cookies != null) {
            for (String cookie : cookies) {
                String[] pairs = cookie.split(";");
                for (String pair : pairs) {
                    String[] keyValue = pair.trim().split("=", 2);
                    if (keyValue.length == 2 && "session_id".equals(keyValue[0])) {
                        return keyValue[1];
                    }
                }
            }
        }
        return null;
    }

    private static Map<String, String> parseFormData(String formData) {
        Map<String, String> map = new HashMap<>();
        String[] pairs = formData.split("&");
        for (String pair : pairs) {
            String[] keyValue = pair.split("=", 2);
            if (keyValue.length == 2) {
                try {
                    String key = URLDecoder.decode(keyValue[0], "UTF-8");
                    String value = URLDecoder.decode(keyValue[1], "UTF-8");
                    map.put(key, value);
                } catch (Exception e) {
                    // Ignore malformed pairs
                }
            }
        }
        return map;
    }

    private static void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
        exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
        OutputStream os = exchange.getResponseBody();
        os.write(response.getBytes(StandardCharsets.UTF_8));
        os.close();
    }
}