import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.time.Instant;
import java.time.Duration;
import java.util.UUID;
import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.ConcurrentHashMap;
import java.util.stream.Collectors;

public class Task43 {

    private static final Map<String, Session> sessionStore = new ConcurrentHashMap<>();
    private static final Duration SESSION_TIMEOUT = Duration.ofMinutes(5);

    // In-memory record to store session data
    private record Session(String username, Instant expires) {
        public boolean isExpired() {
            return Instant.now().isAfter(expires);
        }
    }

    // A dummy user database
    private static final Map<String, String> users = Map.of("testuser", "password123");

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        server.createContext("/login", new LoginHandler());
        server.createContext("/profile", new ProfileHandler());
        server.createContext("/logout", new LogoutHandler());
        
        server.setExecutor(null); // creates a default executor
        server.start();
        
        System.out.println("Server started on port " + port);
        System.out.println("--- Test Cases (run in your terminal) ---");
        System.out.println("1. Login (Success):");
        System.out.println("   curl -X POST -d \"username=testuser&password=password123\" -c cookie.txt -v http://localhost:8080/login");
        System.out.println("\n2. Access Profile with Session Cookie (Success):");
        System.out.println("   curl -b cookie.txt http://localhost:8080/profile");
        System.out.println("\n3. Access Profile without Session Cookie (Failure):");
        System.out.println("   curl http://localhost:8080/profile");
        System.out.println("\n4. Logout:");
        System.out.println("   curl -b cookie.txt http://localhost:8080/logout");
        System.out.println("\n5. Access Profile after Logout (Failure):");
        System.out.println("   curl -b cookie.txt http://localhost:8080/profile");
    }

    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equalsIgnoreCase(exchange.getRequestMethod())) {
                // Parse form data
                String body = new BufferedReader(new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8))
                        .lines().collect(Collectors.joining("\n"));
                Map<String, String> params = parseFormData(body);
                String username = params.get("username");
                String password = params.get("password");

                // Authenticate user
                if (username != null && users.containsKey(username) && users.get(username).equals(password)) {
                    String sessionId = createSession(username);
                    exchange.getResponseHeaders().add("Set-Cookie", "session_id=" + sessionId + "; HttpOnly; Path=/");
                    sendResponse(exchange, 200, "Login successful.");
                } else {
                    sendResponse(exchange, 401, "Invalid credentials.");
                }
            } else {
                sendResponse(exchange, 405, "Method Not Allowed");
            }
        }
    }

    static class ProfileHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String sessionId = getSessionIdFromCookie(exchange);
            Session session = getSession(sessionId);

            if (session != null) {
                sendResponse(exchange, 200, "Hello, " + session.username() + ". Welcome to your profile.");
            } else {
                sendResponse(exchange, 401, "Unauthorized. Please log in.");
            }
        }
    }

    static class LogoutHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String sessionId = getSessionIdFromCookie(exchange);
            if (sessionId != null) {
                terminateSession(sessionId);
            }
            // Expire the cookie on the client side
            exchange.getResponseHeaders().add("Set-Cookie", "session_id=; HttpOnly; Path=/; Max-Age=0");
            sendResponse(exchange, 200, "Logged out successfully.");
        }
    }

    // --- Session Management Logic ---
    public static String createSession(String username) {
        String sessionId = UUID.randomUUID().toString();
        Instant expires = Instant.now().plus(SESSION_TIMEOUT);
        sessionStore.put(sessionId, new Session(username, expires));
        return sessionId;
    }

    public static Session getSession(String sessionId) {
        if (sessionId == null || sessionId.isEmpty()) {
            return null;
        }
        Session session = sessionStore.get(sessionId);
        if (session != null && !session.isExpired()) {
            return session;
        }
        // If session is expired, remove it
        if (session != null) {
            sessionStore.remove(sessionId);
        }
        return null;
    }

    public static void terminateSession(String sessionId) {
        if (sessionId != null) {
            sessionStore.remove(sessionId);
        }
    }

    // --- Utility Methods ---
    private static void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
        exchange.sendResponseHeaders(statusCode, response.getBytes().length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(response.getBytes());
        }
    }

    private static String getSessionIdFromCookie(HttpExchange exchange) {
        String cookieHeader = exchange.getRequestHeaders().getFirst("Cookie");
        if (cookieHeader != null) {
            String[] cookies = cookieHeader.split("; ");
            for (String cookie : cookies) {
                if (cookie.startsWith("session_id=")) {
                    return cookie.substring("session_id=".length());
                }
            }
        }
        return null;
    }

    private static Map<String, String> parseFormData(String formData) {
        Map<String, String> map = new HashMap<>();
        if (formData == null || formData.isEmpty()) return map;
        String[] pairs = formData.split("&");
        for (String pair : pairs) {
            int idx = pair.indexOf("=");
            if (idx > 0) {
                 map.put(pair.substring(0, idx), pair.substring(idx + 1));
            }
        }
        return map;
    }
}