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
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

public class Task40 {

    private static final Map<String, String> sessions = new ConcurrentHashMap<>(); // SessionID -> CSRF Token
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Base64.Encoder base64Encoder = Base64.getUrlEncoder();

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/settings", new SettingsHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("--- How to Test ---");
        System.out.println("1. Open a web browser and navigate to http://localhost:8080/settings");
        System.out.println("\n--- Test Cases ---");
        System.out.println("1. [SUCCESS] Normal submission: Fill out the form and click 'Update Settings'. You should see a success message.");
        System.out.println("2. [FAIL] Tampered CSRF Token: Use browser dev tools to change the value of the hidden 'csrf_token' input, then submit. You should get a 403 Forbidden error.");
        System.out.println("3. [FAIL] Missing CSRF Token: Use browser dev tools to delete the hidden 'csrf_token' input, then submit. You should get a 403 Forbidden error.");
        System.out.println("4. [FAIL] Stale CSRF Token: Open the form in two tabs. Submit the first one successfully. Then try to submit the second one. It should fail because the session CSRF token was regenerated after the first successful submission.");
        System.out.println("5. [FAIL] Cross-Site Request Forgery Simulation (using curl):");
        System.out.println("   Run this command in your terminal: \n   curl -X POST -d \"username=attacker&email=attacker@site.com&csrf_token=fake_token\" http://localhost:8080/settings -v --cookie \"session_id=...your_session_id_from_browser...\"");
        System.out.println("   (Replace ...your_session_id_from_browser... with the actual value from your browser's cookies). This should fail with a 403 error.");

    }

    static class SettingsHandler implements HttpHandler {

        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if ("GET".equals(exchange.getRequestMethod())) {
                    handleGet(exchange);
                } else if ("POST".equals(exchange.getRequestMethod())) {
                    handlePost(exchange);
                } else {
                    sendResponse(exchange, 405, "Method Not Allowed");
                }
            } catch (Exception e) {
                // In a real app, log the exception
                sendResponse(exchange, 500, "Internal Server Error");
            }
        }
        
        private void handleGet(HttpExchange exchange) throws IOException {
            String sessionId = getOrCreateSession(exchange);
            String csrfToken = generateCsrfToken();
            sessions.put(sessionId, csrfToken);

            String htmlForm = "<html>" +
                "<head><title>User Settings</title></head>" +
                "<body>" +
                "<h2>Update Settings</h2>" +
                "<form action=\"/settings\" method=\"post\">" +
                "    <label for=\"username\">Username:</label><br>" +
                "    <input type=\"text\" id=\"username\" name=\"username\" value=\"testuser\"><br><br>" +
                "    <label for=\"email\">Email:</label><br>" +
                "    <input type=\"email\" id=\"email\" name=\"email\" value=\"user@example.com\"><br><br>" +
                "    <input type=\"hidden\" name=\"csrf_token\" value=\"" + csrfToken + "\">" +
                "    <input type=\"submit\" value=\"Update Settings\">" +
                "</form>" +
                "</body>" +
                "</html>";
            
            sendResponse(exchange, 200, htmlForm);
        }

        private void handlePost(HttpExchange exchange) throws IOException {
            String sessionId = getSessionId(exchange);
            if (sessionId == null || !sessions.containsKey(sessionId)) {
                sendResponse(exchange, 403, "Forbidden: Invalid session");
                return;
            }

            String expectedToken = sessions.get(sessionId);
            
            String requestBody = new BufferedReader(new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8))
                .lines().collect(Collectors.joining("\n"));
            
            Map<String, String> params = parseFormData(requestBody);
            String submittedToken = params.get("csrf_token");

            if (submittedToken == null || !constantTimeEquals(expectedToken.getBytes(StandardCharsets.UTF_8), submittedToken.getBytes(StandardCharsets.UTF_8))) {
                sendResponse(exchange, 403, "Forbidden: Invalid CSRF token");
                return;
            }

            // CSRF check passed. Regenerate token to prevent reuse.
            sessions.put(sessionId, generateCsrfToken());

            String username = htmlEscape(params.getOrDefault("username", ""));
            String email = htmlEscape(params.getOrDefault("email", ""));

            System.out.println("Settings updated for session " + sessionId + ": Username=" + username + ", Email=" + email);

            String successResponse = "<html>" +
                "<body>" +
                "<h2>Settings Updated Successfully</h2>" +
                "<p>Username: " + username + "</p>" +
                "<p>Email: " + email + "</p>" +
                "<a href=\"/settings\">Go back</a>" +
                "</body>" +
                "</html>";

            sendResponse(exchange, 200, successResponse);
        }

        private String getOrCreateSession(HttpExchange exchange) {
            String sessionId = getSessionId(exchange);
            if (sessionId == null || !sessions.containsKey(sessionId)) {
                sessionId = UUID.randomUUID().toString();
                exchange.getResponseHeaders().add("Set-Cookie", "session_id=" + sessionId + "; HttpOnly; Path=/");
            }
            return sessionId;
        }

        private String getSessionId(HttpExchange exchange) {
            return Optional.ofNullable(exchange.getRequestHeaders().getFirst("Cookie"))
                .map(cookieStr -> {
                    for (String cookie : cookieStr.split(";")) {
                        String[] parts = cookie.trim().split("=", 2);
                        if (parts.length == 2 && "session_id".equals(parts[0])) {
                            return parts[1];
                        }
                    }
                    return null;
                })
                .orElse(null);
        }

        private String generateCsrfToken() {
            byte[] randomBytes = new byte[32];
            secureRandom.nextBytes(randomBytes);
            return base64Encoder.encodeToString(randomBytes);
        }

        private Map<String, String> parseFormData(String formData) {
            Map<String, String> params = new HashMap<>();
            if (formData == null || formData.isEmpty()) {
                return params;
            }
            for (String param : formData.split("&")) {
                String[] parts = param.split("=", 2);
                if (parts.length == 2) {
                    try {
                        String key = URLDecoder.decode(parts[0], StandardCharsets.UTF_8.name());
                        String value = URLDecoder.decode(parts[1], StandardCharsets.UTF_8.name());
                        params.put(key, value);
                    } catch (java.io.UnsupportedEncodingException e) {
                        // Should not happen with UTF-8
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

        private boolean constantTimeEquals(byte[] a, byte[] b) {
            return MessageDigest.isEqual(a, b);
        }

        private String htmlEscape(String input) {
            if (input == null) return "";
            return input.replace("&", "&amp;")
                        .replace("<", "&lt;")
                        .replace(">", "&gt;")
                        .replace("\"", "&quot;")
                        .replace("'", "&#x27;");
        }
    }
}