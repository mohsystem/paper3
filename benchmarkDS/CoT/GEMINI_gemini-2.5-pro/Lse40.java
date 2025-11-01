import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;

public class Lse40 {

    // --- In-memory Session Storage (for demonstration purposes) ---
    // In a real application, use a proper session management system.
    private static final Map<String, Map<String, String>> sessions = new ConcurrentHashMap<>();

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        // Route to simulate login and create a session
        server.createContext("/login", new LoginHandler());
        
        // The main /info route
        server.createContext("/info", new InfoHandler());

        // Route for testing
        server.createContext("/test", new TestHandler());

        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("--- Test Cases ---");
        System.out.println("Visit http://localhost:8080/test to run test cases.");
    }

    // Handler for the /info route
    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String sessionId = getSessionId(exchange);
            String response;
            int statusCode = 200;

            if (sessionId != null && sessions.containsKey(sessionId)) {
                response = info(sessions.get(sessionId));
            } else {
                statusCode = 401; // Unauthorized
                response = "<h1>Unauthorized</h1><p>Please <a href='/test'>login</a> first.</p>";
            }

            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }

        // The main logic function for the /info route
        private String info(Map<String, String> sessionData) {
            String username = sessionData.getOrDefault("username", "N/A");
            String email = sessionData.getOrDefault("email", "N/A");
            String ssn = sessionData.getOrDefault("ssn", "0000");

            // SECURITY: Never store full SSN in session in a real application.
            // This is done here only to adhere to the prompt's requirements.
            String last4SSN = ssn.length() > 4 ? ssn.substring(ssn.length() - 4) : ssn;

            // SECURITY: Escape user-provided data to prevent XSS attacks.
            String escapedUsername = escapeHtml(username);
            String escapedEmail = escapeHtml(email);

            return String.join("\n",
                "<!DOCTYPE html>",
                "<html>",
                "<head><title>User Information</title></head>",
                "<body>",
                "<h1>User Information</h1>",
                "<p><strong>Username:</strong> " + escapedUsername + "</p>",
                "<p><strong>Email:</strong> " + escapedEmail + "</p>",
                "<p><strong>Last 4 Digits of SSN:</strong> " + last4SSN + "</p>",
                "</body>",
                "</html>"
            );
        }
    }

    // Handler to simulate logging in and setting session data
    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            Map<String, String> params = parseQuery(exchange.getRequestURI().getQuery());
            
            String username = params.get("user");
            String email = params.get("email");
            String ssn = params.get("ssn");

            if (username == null || email == null || ssn == null) {
                 String response = "Bad Request: 'user', 'email', and 'ssn' query parameters are required.";
                 exchange.sendResponseHeaders(400, response.length());
                 try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes());
                 }
                 return;
            }

            String sessionId = UUID.randomUUID().toString();
            Map<String, String> sessionData = new HashMap<>();
            sessionData.put("username", username);
            sessionData.put("email", email);
            sessionData.put("ssn", ssn);
            
            sessions.put(sessionId, sessionData);

            exchange.getResponseHeaders().add("Set-Cookie", "sessionId=" + sessionId + "; Path=/");
            // Redirect to the info page
            exchange.getResponseHeaders().add("Location", "/info");
            exchange.sendResponseHeaders(302, -1); 
        }
    }
    
    // Handler to provide links for the 5 test cases
    static class TestHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String response = String.join("\n",
                "<!DOCTYPE html><html><head><title>Test Cases</title></head><body>",
                "<h1>Click a user to log in</h1>",
                "<ul>",
                "<li><a href='/login?user=alice&email=alice@example.com&ssn=111223344'>Login as Alice</a></li>",
                "<li><a href='/login?user=bob&email=bob@example.com&ssn=555667788'>Login as Bob</a></li>",
                "<li><a href='/login?user=charlie<script>alert(1)</script>&email=charlie@bad.com&ssn=999887766'>Login as Charlie (XSS Test)</a></li>",
                "<li><a href='/login?user=dave&email=dave@example.com&ssn=123456789'>Login as Dave</a></li>",
                "<li><a href='/login?user=eve&email=eve@example.com&ssn=000000001'>Login as Eve</a></li>",
                "</ul>",
                "</body></html>"
            );
            t.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            t.sendResponseHeaders(200, response.getBytes().length);
            try (OutputStream os = t.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }


    // --- Utility Functions ---

    private static String getSessionId(HttpExchange exchange) {
        String cookieHeader = exchange.getRequestHeaders().getFirst("Cookie");
        if (cookieHeader != null) {
            String[] cookies = cookieHeader.split(";");
            for (String cookie : cookies) {
                String[] parts = cookie.trim().split("=");
                if (parts.length == 2 && "sessionId".equals(parts[0])) {
                    return parts[1];
                }
            }
        }
        return null;
    }
    
    private static Map<String, String> parseQuery(String query) {
        if (query == null) {
            return new HashMap<>();
        }
        return Stream.of(query.split("&"))
                .map(s -> s.split("=", 2))
                .collect(Collectors.toMap(
                        a -> URLDecoder.decode(a[0], StandardCharsets.UTF_8),
                        a -> a.length > 1 ? URLDecoder.decode(a[1], StandardCharsets.UTF_8) : "",
                        (v1, v2) -> v1 // In case of duplicate keys, keep the first
                ));
    }
    
    private static String escapeHtml(String text) {
        if (text == null) {
            return "";
        }
        return text.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\"", "&quot;")
                   .replace("'", "&#x27;");
    }
}