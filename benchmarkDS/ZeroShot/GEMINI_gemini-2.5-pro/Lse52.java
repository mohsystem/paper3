import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.net.URLDecoder;
import java.util.stream.Collectors;

public class Lse52 {
    
    // In-memory storage for users and sessions.
    // In a real application, use a database.
    private static final Map<String, User> userDatabase = new ConcurrentHashMap<>();
    private static final Map<String, String> sessionStore = new ConcurrentHashMap<>();

    // User record class
    private static class User {
        final String username;
        final String salt;
        final String hashedPassword;

        User(String username, String password) {
            this.username = username;
            this.salt = generateSalt();
            this.hashedPassword = hashPassword(password, this.salt);
        }
    }

    // --- Security Helper Functions ---

    private static String generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return Base64.getEncoder().encodeToString(salt);
    }

    private static String hashPassword(String password, String salt) {
        try {
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(Base64.getDecoder().decode(salt));
            byte[] hashedPassword = md.digest(password.getBytes(StandardCharsets.UTF_8));
            return Base64.getEncoder().encodeToString(hashedPassword);
        } catch (NoSuchAlgorithmException e) {
            throw new RuntimeException("SHA-256 not available", e);
        }
    }

    // Securely checks credentials using a constant-time comparison
    public static boolean checkCredentials(String username, String password) {
        if (username == null || password == null || username.trim().isEmpty() || password.trim().isEmpty()) {
            return false;
        }
        User user = userDatabase.get(username);
        if (user == null) {
            // To prevent username enumeration, perform a dummy hash check
            hashPassword(password, generateSalt());
            return false;
        }
        String providedPasswordHash = hashPassword(password, user.salt);
        return MessageDigest.isEqual(
            Base64.getDecoder().decode(user.hashedPassword),
            Base64.getDecoder().decode(providedPasswordHash)
        );
    }
    
    // --- HTTP Handlers ---
    
    static class IndexHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String response = "<html><body><h1>Welcome</h1><p><a href='/login'>Login</a></p></body></html>";
            sendResponse(t, 200, response);
        }
    }

    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            if ("GET".equalsIgnoreCase(t.getRequestMethod())) {
                String response = "<html><body>" +
                                "<h2>Login</h2>" +
                                "<form method='post' action='/do_login'>" +
                                "  Username: <input type='text' name='username'><br>" +
                                "  Password: <input type='password' name='password'><br>" +
                                "  <input type='submit' value='Login'>" +
                                "</form>" +
                                "</body></html>";
                sendResponse(t, 200, response);
            } else {
                sendResponse(t, 405, "Method Not Allowed");
            }
        }
    }
    
    static class DoLoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            if (!"POST".equalsIgnoreCase(t.getRequestMethod())) {
                 sendResponse(t, 405, "Method Not Allowed");
                 return;
            }

            String body = new BufferedReader(new InputStreamReader(t.getRequestBody(), StandardCharsets.UTF_8))
                .lines().collect(Collectors.joining("\n"));

            Map<String, String> params = parseFormData(body);
            String username = params.get("username");
            String password = params.get("password");

            if (checkCredentials(username, password)) {
                String sessionId = UUID.randomUUID().toString();
                sessionStore.put(sessionId, username);
                // Set a secure, HttpOnly cookie. Add "; Secure" if using HTTPS.
                t.getResponseHeaders().add("Set-Cookie", "session_id=" + sessionId + "; HttpOnly; Path=/");
                // Redirect to secret page
                t.getResponseHeaders().add("Location", "/secret");
                sendResponse(t, 302, "");
            } else {
                String response = "<html><body>Invalid username or password. <a href='/login'>Try again</a>.</body></html>";
                sendResponse(t, 401, response);
            }
        }
    }

    static class SecretHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String sessionId = getSessionId(t);
            if (sessionId != null && sessionStore.containsKey(sessionId)) {
                String username = sessionStore.get(sessionId);
                String response = "<html><body><h1>Secret settings</h1><p>Welcome, " + htmlEscape(username) + "!</p></body></html>";
                sendResponse(t, 200, response);
            } else {
                t.getResponseHeaders().add("Location", "/login");
                sendResponse(t, 302, "");
            }
        }
    }

    // --- Utility Functions ---

    private static void sendResponse(HttpExchange t, int statusCode, String response) throws IOException {
        t.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
        try (OutputStream os = t.getResponseBody()) {
            os.write(response.getBytes(StandardCharsets.UTF_8));
        }
    }

    private static Map<String, String> parseFormData(String formData) {
        Map<String, String> map = new HashMap<>();
        if (formData == null || formData.isEmpty()) {
            return map;
        }
        for (String pair : formData.split("&")) {
            try {
                int idx = pair.indexOf("=");
                if (idx > 0) {
                    map.put(
                        URLDecoder.decode(pair.substring(0, idx), "UTF-8"),
                        URLDecoder.decode(pair.substring(idx + 1), "UTF-8")
                    );
                }
            } catch (java.io.UnsupportedEncodingException e) {
                 // Should not happen with UTF-8
            }
        }
        return map;
    }

    private static String getSessionId(HttpExchange t) {
        String cookieHeader = t.getRequestHeaders().getFirst("Cookie");
        if (cookieHeader != null) {
            for (String cookie : cookieHeader.split(";")) {
                String[] parts = cookie.trim().split("=", 2);
                if (parts.length == 2 && "session_id".equals(parts[0])) {
                    return parts[1];
                }
            }
        }
        return null;
    }

    private static String htmlEscape(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\"", "&quot;")
                    .replace("'", "&#39;");
    }

    public static void main(String[] args) throws IOException {
        // --- Setup Users ---
        // In a real app, load from a database.
        userDatabase.put("admin", new User("admin", "password123"));

        // --- 5 Test Cases for Login Logic ---
        System.out.println("--- Running 5 Login Logic Test Cases ---");
        System.out.println("1. Correct credentials ('admin', 'password123'): " + checkCredentials("admin", "password123"));
        System.out.println("2. Wrong password ('admin', 'wrongpass'): " + checkCredentials("admin", "wrongpass"));
        System.out.println("3. Wrong username ('user', 'password123'): " + checkCredentials("user", "password123"));
        System.out.println("4. Null username (null, 'password123'): " + checkCredentials(null, "password123"));
        System.out.println("5. Empty password ('admin', ''): " + checkCredentials("admin", ""));
        System.out.println("----------------------------------------\n");

        // --- Start Web Server ---
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/", new IndexHandler());
        server.createContext("/login", new LoginHandler());
        server.createContext("/do_login", new DoLoginHandler());
        server.createContext("/secret", new SecretHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Access at http://localhost:" + port);
    }
}