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
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;
import java.util.List;
import java.util.HashMap;

public class Task40 {

    // In-memory storage for sessions and user data.
    // In a real application, use a database.
    private static final Map<String, Map<String, String>> sessions = new ConcurrentHashMap<>();
    private static final Map<String, String> userData = new ConcurrentHashMap<>();

    static {
        // Pre-populate with a user
        userData.put("user1", "user1@example.com");
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/settings", new SettingsHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("--- How to Test ---");
        System.out.println("Use a tool like curl. A 'cookie-jar.txt' file will be created to manage the session.");
        System.out.println("\n--- Test Case 1: Load the form (GET request) ---");
        System.out.println("curl -v -c cookie-jar.txt http://localhost:8080/settings");
        System.out.println("=> This will create a session and return a form with a CSRF token.");
        
        System.out.println("\n--- Test Case 2: Submit form with a valid token (SUCCESS) ---");
        System.out.println("1. First, run Test Case 1 to get a valid token from the HTML output.");
        System.out.println("2. Replace 'VALID_TOKEN_FROM_STEP_1' with the actual token.");
        System.out.println("curl -v -X POST -b cookie-jar.txt -c cookie-jar.txt \\");
        System.out.println("  --data-urlencode \"email=new.email@example.com\" \\");
        System.out.println("  --data-urlencode \"csrf_token=VALID_TOKEN_FROM_STEP_1\" \\");
        System.out.println("  http://localhost:8080/settings");
        
        System.out.println("\n--- Test Case 3: Submit form with an invalid token (FAIL) ---");
        System.out.println("curl -v -X POST -b cookie-jar.txt -c cookie-jar.txt \\");
        System.out.println("  --data-urlencode \"email=another.email@example.com\" \\");
        System.out.println("  --data-urlencode \"csrf_token=invalid_token_12345\" \\");
        System.out.println("  http://localhost:8080/settings");

        System.out.println("\n--- Test Case 4: Submit form with a missing token (FAIL) ---");
        System.out.println("curl -v -X POST -b cookie-jar.txt -c cookie-jar.txt \\");
        System.out.println("  --data-urlencode \"email=another.email@example.com\" \\");
        System.out.println("  http://localhost:8080/settings");

        System.out.println("\n--- Test Case 5: Submit form with a valid token but no session cookie (FAIL) ---");
        System.out.println("1. First, run Test Case 1 to get a valid token.");
        System.out.println("2. Replace 'VALID_TOKEN_FROM_STEP_1' with the actual token.");
        System.out.println("3. Note we are NOT sending the cookie jar (-b flag).");
        System.out.println("curl -v -X POST \\");
        System.out.println("  --data-urlencode \"email=another.email@example.com\" \\");
        System.out.println("  --data-urlencode \"csrf_token=VALID_TOKEN_FROM_STEP_1\" \\");
        System.out.println("  http://localhost:8080/settings");
    }

    static class SettingsHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // A simple way to manage sessions via cookies
            String sessionId = getSessionId(exchange);
            if (sessionId == null || !sessions.containsKey(sessionId)) {
                sessionId = UUID.randomUUID().toString();
                sessions.put(sessionId, new ConcurrentHashMap<>());
                // We hardcode the user for this example. In a real app, this would be after login.
                sessions.get(sessionId).put("username", "user1");
            }
            exchange.getResponseHeaders().add("Set-Cookie", "sessionId=" + sessionId + "; HttpOnly");

            Map<String, String> sessionData = sessions.get(sessionId);

            if ("GET".equals(exchange.getRequestMethod())) {
                handleGet(exchange, sessionData);
            } else if ("POST".equals(exchange.getRequestMethod())) {
                handlePost(exchange, sessionData);
            } else {
                sendResponse(exchange, 405, "Method Not Allowed");
            }
        }

        private void handleGet(HttpExchange exchange, Map<String, String> sessionData) throws IOException {
            String username = sessionData.get("username");
            String currentEmail = userData.getOrDefault(username, "");
            
            // CSRF Protection: Generate and store token in session
            String csrfToken = UUID.randomUUID().toString();
            sessionData.put("csrf_token", csrfToken);

            String response = "<html><body>" +
                    "<h2>Update Settings for " + username + "</h2>" +
                    "<form action='/settings' method='post'>" +
                    "  <label for='email'>Email:</label><br>" +
                    "  <input type='email' id='email' name='email' value='" + currentEmail + "'><br><br>" +
                    // CSRF Protection: Include token in a hidden field
                    "  <input type='hidden' name='csrf_token' value='" + csrfToken + "'>" +
                    "  <input type='submit' value='Update'>" +
                    "</form>" +
                    "</body></html>";
            sendResponse(exchange, 200, response);
        }

        private void handlePost(HttpExchange exchange, Map<String, String> sessionData) throws IOException {
            Map<String, String> params = parseFormData(exchange);
            String formToken = params.get("csrf_token");
            String sessionToken = sessionData.get("csrf_token");

            // CSRF Protection: Validate token
            if (sessionToken == null || formToken == null || !sessionToken.equals(formToken)) {
                sendResponse(exchange, 403, "Forbidden: Invalid CSRF Token");
                return;
            }

            // Invalidate the token after use to prevent reuse (optional but good practice)
            sessionData.remove("csrf_token");

            String newEmail = params.get("email");
            String username = sessionData.get("username");

            if (username != null && newEmail != null && !newEmail.trim().isEmpty()) {
                userData.put(username, newEmail);
                String response = "<html><body><h2>Success!</h2><p>Email for " + username + " updated to " + newEmail + ".</p><a href='/settings'>Go back</a></body></html>";
                sendResponse(exchange, 200, response);
            } else {
                sendResponse(exchange, 400, "Bad Request: Email is required.");
            }
        }

        private String getSessionId(HttpExchange exchange) {
            List<String> cookies = exchange.getRequestHeaders().get("Cookie");
            if (cookies != null) {
                for (String cookieStr : cookies) {
                    for (String cookiePair : cookieStr.split(";")) {
                        String[] pair = cookiePair.trim().split("=");
                        if (pair.length == 2 && "sessionId".equals(pair[0])) {
                            return pair[1];
                        }
                    }
                }
            }
            return null;
        }

        private Map<String, String> parseFormData(HttpExchange exchange) throws IOException {
            Map<String, String> params = new HashMap<>();
            try (BufferedReader br = new BufferedReader(new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8))) {
                String query = br.lines().collect(Collectors.joining("\n"));
                for (String param : query.split("&")) {
                    String[] pair = param.split("=");
                    if (pair.length > 1) {
                        params.put(URLDecoder.decode(pair[0], "UTF-8"), URLDecoder.decode(pair[1], "UTF-8"));
                    } else {
                        params.put(URLDecoder.decode(pair[0], "UTF-8"), "");
                    }
                }
            }
            return params;
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }
}