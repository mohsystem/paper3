import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.net.URLDecoder;
import java.security.MessageDigest;

public class Lse54 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        server.createContext("/login", new LoginHandler());
        server.createContext("/do_login", new DoLoginHandler());
        server.createContext("/admin", new AdminHandler());
        
        server.setExecutor(null); // creates a default executor
        server.start();
        
        System.out.println("Server started on port " + port);
        System.out.println("--- How to run test cases ---");
        System.out.println("1. Open a web browser and go to: http://localhost:8080/login");
        System.out.println("2. Test Case 1 (Correct Credentials): Enter 'admin' and 'password123', then submit. You should be redirected to the admin page.");
        System.out.println("3. Test Case 2 (Incorrect Username): Go back to the login page. Enter 'user' and 'password123', then submit. You should see a 'Login Failed' message.");
        System.out.println("4. Test Case 3 (Incorrect Password): Go back to the login page. Enter 'admin' and 'wrongpass', then submit. You should see a 'Login Failed' message.");
        System.out.println("5. Test Case 4 (Empty Credentials): Go back to the login page. Leave fields empty and submit. You should see a 'Login Failed' message.");
        System.out.println("6. Test Case 5 (Direct Admin Access): Try to directly access http://localhost:8080/admin. You should see the welcome message (Note: In a real app, this page should be protected by a session).");
    }

    // Handler for GET /login
    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("GET".equals(exchange.getRequestMethod())) {
                String response = "<html>" +
                                "<body>" +
                                "<h2>Login</h2>" +
                                "<form action=\"/do_login\" method=\"post\">" +
                                "  Username: <input type=\"text\" name=\"username\"><br>" +
                                "  Password: <input type=\"password\" name=\"password\"><br>" +
                                "  <input type=\"submit\" value=\"Login\">" +
                                "</form>" +
                                "</body>" +
                                "</html>";
                sendResponse(exchange, 200, response);
            } else {
                sendResponse(exchange, 405, "Method Not Allowed");
            }
        }
    }

    // Handler for POST /do_login
    static class DoLoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equals(exchange.getRequestMethod())) {
                Map<String, String> params = parseFormData(exchange.getRequestBody());
                String username = params.getOrDefault("username", "");
                String password = params.getOrDefault("password", "");

                // NOTE: In a real application, NEVER use hardcoded plaintext passwords.
                // Always use a strong, salted hashing algorithm like Argon2 or bcrypt.
                // The comparison should be constant-time to prevent timing attacks.
                final String expectedUser = "admin";
                final String expectedPass = "password123";

                boolean userMatch = constantTimeEquals(username, expectedUser);
                boolean passMatch = constantTimeEquals(password, expectedPass);

                if (userMatch && passMatch) {
                    // Redirect to admin page
                    exchange.getResponseHeaders().set("Location", "/admin");
                    sendResponse(exchange, 302, ""); // 302 Found for redirection
                } else {
                    sendResponse(exchange, 401, "Login Failed");
                }
            } else {
                sendResponse(exchange, 405, "Method Not Allowed");
            }
        }
    }

    // Handler for GET /admin
    static class AdminHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "<h1>Welcome, admin!</h1>";
            sendResponse(exchange, 200, response);
        }
    }

    // Utility to send a response
    private static void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
        exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
        exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(response.getBytes(StandardCharsets.UTF_8));
        }
    }

    // Utility to parse form data from the request body
    private static Map<String, String> parseFormData(InputStream is) throws IOException {
        Map<String, String> map = new HashMap<>();
        try (BufferedReader br = new BufferedReader(new InputStreamReader(is, StandardCharsets.UTF_8))) {
            String formData = br.readLine();
            if (formData != null) {
                String[] pairs = formData.split("&");
                for (String pair : pairs) {
                    int idx = pair.indexOf("=");
                    if (idx > 0) {
                        String key = URLDecoder.decode(pair.substring(0, idx), "UTF-8");
                        String value = URLDecoder.decode(pair.substring(idx + 1), "UTF-8");
                        map.put(key, value);
                    }
                }
            }
        }
        return map;
    }

    // Constant-time string comparison to mitigate timing attacks
    private static boolean constantTimeEquals(String a, String b) {
        return MessageDigest.isEqual(a.getBytes(StandardCharsets.UTF_8), b.getBytes(StandardCharsets.UTF_8));
    }
}