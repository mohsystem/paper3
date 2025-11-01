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
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;

// In a real application, never hardcode credentials.
// CWE-798: Use of Hard-coded Credentials
// Use a secure identity provider and retrieve secrets from a vault or environment variables.
// This example uses plaintext comparison for simplicity, but production systems
// MUST use a strong, salted hashing algorithm like Argon2, scrypt, or PBKDF2.
// CWE-759: Use of a One-Way Hash without a Salt
final class Credentials {
    // This is for demonstration purposes ONLY.
    public static final String ADMIN_USERNAME = "admin";
    public static final String ADMIN_PASSWORD = "password";
}

public class Lse54 {

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        
        server.createContext("/login", new LoginHandler());
        server.createContext("/admin", new AdminHandler());
        server.setExecutor(Executors.newCachedThreadPool()); 
        server.start();

        System.out.println("Server started on port " + port);
        System.out.println("--- Test Cases ---");
        System.out.println("1. View login form: curl http://localhost:8080/login");
        System.out.println("2. Successful login (will get a 302 redirect): curl -i -L -d \"username=admin&password=password\" http://localhost:8080/login");
        System.out.println("3. Failed login (wrong password): curl -i -d \"username=admin&password=wrong\" http://localhost:8080/login");
        System.out.println("4. Failed login (wrong username): curl -i -d \"username=user&password=password\" http://localhost:8080/login");
        System.out.println("5. Access admin page after successful login: Follow the redirect from test case #2.");
    }

    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("GET".equals(exchange.getRequestMethod())) {
                handleGetRequest(exchange, null);
            } else if ("POST".equals(exchange.getRequestMethod())) {
                handlePostRequest(exchange);
            } else {
                exchange.sendResponseHeaders(405, -1); // Method Not Allowed
            }
        }

        private void handleGetRequest(HttpExchange exchange, String errorMessage) throws IOException {
            String errorHtml = errorMessage != null ? "<p style='color:red;'>" + escapeHtml(errorMessage) + "</p>" : "";
            String response = "<html><body>"
                            + "<h1>Login</h1>"
                            + errorHtml
                            + "<form method='POST' action='/login'>"
                            + "Username: <input type='text' name='username'><br>"
                            + "Password: <input type='password' name='password'><br>"
                            + "<input type='submit' value='Login'>"
                            + "</form>"
                            + "</body></html>";
            
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }

        private void handlePostRequest(HttpExchange exchange) throws IOException {
            // Rule #1: Validate input
            String contentType = exchange.getRequestHeaders().getFirst("Content-Type");
            if (contentType == null || !contentType.equalsIgnoreCase("application/x-www-form-urlencoded")) {
                 handleGetRequest(exchange, "Invalid request format.");
                 return;
            }

            InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
            BufferedReader br = new BufferedReader(isr);
            String formData = br.readLine();
            Map<String, String> params = parseFormData(formData);

            String username = params.getOrDefault("username", "");
            String password = params.getOrDefault("password", "");

            // In a real app, use a constant-time comparison for passwords to prevent timing attacks.
            if (Credentials.ADMIN_USERNAME.equals(username) && Credentials.ADMIN_PASSWORD.equals(password)) {
                exchange.getResponseHeaders().set("Location", "/admin");
                exchange.sendResponseHeaders(302, -1); // Found (Redirect)
            } else {
                handleGetRequest(exchange, "Invalid username or password.");
            }
        }

        private Map<String, String> parseFormData(String formData) {
            Map<String, String> map = new HashMap<>();
            if (formData == null || formData.isEmpty()) {
                return map;
            }
            String[] pairs = formData.split("&");
            for (String pair : pairs) {
                int idx = pair.indexOf("=");
                try {
                    if (idx > 0) {
                        String key = URLDecoder.decode(pair.substring(0, idx), StandardCharsets.UTF_8.name());
                        String value = URLDecoder.decode(pair.substring(idx + 1), StandardCharsets.UTF_8.name());
                        map.put(key, value);
                    }
                } catch (java.io.UnsupportedEncodingException e) {
                    // This should not happen with UTF-8
                } catch (IllegalArgumentException e) {
                    // Ignore malformed URL encoding
                }
            }
            return map;
        }
        
        // Basic HTML escaping to mitigate CWE-79 (XSS)
        private String escapeHtml(String input) {
            if (input == null) return "";
            return input.replace("&", "&amp;")
                        .replace("<", "&lt;")
                        .replace(">", "&gt;")
                        .replace("\"", "&quot;")
                        .replace("'", "&#x27;");
        }
    }

    static class AdminHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "<html><body><h1>Welcome, Admin!</h1></body></html>";
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }
}