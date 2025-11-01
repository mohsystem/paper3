import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.Headers;

import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;
import java.util.HashMap;
import java.util.stream.Collectors;

public class Lse52 {

    // NOTE: In a real application, do NOT hardcode credentials.
    // Use a database and hashed passwords.
    private static final String CORRECT_USERNAME = "admin";
    private static final byte[] CORRECT_PASSWORD_BYTES = "password123".getBytes(StandardCharsets.UTF_8);

    // A simple in-memory session store. In a real application, use a more robust solution.
    private static final Map<String, String> sessions = new ConcurrentHashMap<>();

    public static void main(String[] args) throws IOException {
        int port = 8000;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        server.createContext("/", new IndexHandler());
        server.createContext("/login", new LoginHandler());
        server.createContext("/do_login", new DoLoginHandler());
        server.createContext("/secret", new SecretHandler());
        
        server.setExecutor(null); // creates a default executor
        server.start();
        
        System.out.println("Server started on port " + port);
        System.out.println("--- How to Run the 5 Test Cases ---");
        System.out.println("1. Open a web browser and go to http://localhost:8000/");
        System.out.println("   Expected: See a link to the login page.");
        System.out.println("2. Now, navigate directly to http://localhost:8000/secret");
        System.out.println("   Expected: You should be redirected to the login page.");
        System.out.println("3. Go to the login page, enter wrong credentials (e.g., user: 'test', pass: 'wrong') and submit.");
        System.out.println("   Expected: You should be redirected back to the login page with an error message.");
        System.out.println("4. On the login page, enter correct credentials (user: 'admin', pass: 'password123') and submit.");
        System.out.println("   Expected: You should be redirected to the secret page and see the secret content.");
        System.out.println("5. While on the secret page, refresh the page or navigate to http://localhost:8000/secret again.");
        System.out.println("   Expected: You should still see the secret content because your session is valid.");
    }

    static class IndexHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String response = "<h1>Welcome</h1><a href=\"/login\">Go to Login Page</a>";
            sendResponse(t, 200, response);
        }
    }

    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String query = t.getRequestURI().getQuery();
            String errorMessage = "";
            if (query != null && query.contains("error=1")) {
                errorMessage = "<p style=\"color:red;\">Invalid username or password.</p>";
            }

            String response = "<html><body>" +
                    "<h2>Login</h2>" +
                    errorMessage +
                    "<form action=\"/do_login\" method=\"post\">" +
                    "  <label for=\"username\">Username:</label><br>" +
                    "  <input type=\"text\" id=\"username\" name=\"username\"><br>" +
                    "  <label for=\"password\">Password:</label><br>" +
                    "  <input type=\"password\" id=\"password\" name=\"password\"><br><br>" +
                    "  <input type=\"submit\" value=\"Submit\">" +
                    "</form>" +
                    "</body></html>";
            sendResponse(t, 200, response);
        }
    }

    static class DoLoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            if (!"post".equalsIgnoreCase(t.getRequestMethod())) {
                sendResponse(t, 405, "Method Not Allowed");
                return;
            }

            Map<String, String> params = parseFormData(t.getRequestBody());
            String username = params.getOrDefault("username", "");
            String password = params.getOrDefault("password", "");

            // Secure constant-time comparison for password
            boolean isPasswordCorrect = MessageDigest.isEqual(
                password.getBytes(StandardCharsets.UTF_8), 
                CORRECT_PASSWORD_BYTES
            );

            if (CORRECT_USERNAME.equals(username) && isPasswordCorrect) {
                String sessionId = UUID.randomUUID().toString();
                sessions.put(sessionId, username);
                
                Headers headers = t.getResponseHeaders();
                headers.add("Set-Cookie", "session_id=" + sessionId + "; Path=/");
                headers.add("Location", "/secret");
                sendResponse(t, 302, "");
            } else {
                Headers headers = t.getResponseHeaders();
                headers.add("Location", "/login?error=1");
                sendResponse(t, 302, "");
            }
        }
    }
    
    static class SecretHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String sessionId = getSessionId(t);
            if (sessionId != null && sessions.containsKey(sessionId)) {
                String username = sessions.get(sessionId);
                String response = "<h1>Secret settings</h1><p>Welcome, " + htmlEscape(username) + "!</p>";
                sendResponse(t, 200, response);
            } else {
                Headers headers = t.getResponseHeaders();
                headers.add("Location", "/login");
                sendResponse(t, 302, "");
            }
        }
    }

    // Utility methods
    private static void sendResponse(HttpExchange t, int statusCode, String response) throws IOException {
        t.sendResponseHeaders(statusCode, response.getBytes().length);
        try (OutputStream os = t.getResponseBody()) {
            os.write(response.getBytes());
        }
    }

    private static Map<String, String> parseFormData(InputStream is) throws IOException {
        String body = new BufferedReader(new InputStreamReader(is, StandardCharsets.UTF_8))
                .lines().collect(Collectors.joining("\n"));
        Map<String, String> params = new HashMap<>();
        if (body == null || body.isEmpty()) {
            return params;
        }
        for (String param : body.split("&")) {
            String[] pair = param.split("=", 2);
            if (pair.length == 2) {
                params.put(URLDecoder.decode(pair[0], "UTF-8"), URLDecoder.decode(pair[1], "UTF-8"));
            }
        }
        return params;
    }

    private static String getSessionId(HttpExchange t) {
        Headers headers = t.getRequestHeaders();
        if (headers.containsKey("Cookie")) {
            for (String cookieStr : headers.get("Cookie")) {
                for (String cookiePair : cookieStr.split(";")) {
                    String[] pair = cookiePair.trim().split("=", 2);
                    if (pair.length == 2 && "session_id".equals(pair[0])) {
                        return pair[1];
                    }
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
                    .replace("'", "&#x27;");
    }
}