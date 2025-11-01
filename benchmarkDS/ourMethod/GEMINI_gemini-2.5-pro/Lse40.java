import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.CookieManager;
import java.net.CookiePolicy;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.Base64;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;

public class Lse40 {

    private static final Map<String, Map<String, String>> sessionStore = new ConcurrentHashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final Base64.Encoder base64Encoder = Base64.getUrlEncoder();

    private static String generateNewSessionId() {
        byte[] randomBytes = new byte[24];
        secureRandom.nextBytes(randomBytes);
        return base64Encoder.encodeToString(randomBytes);
    }

    private static String htmlEscape(String input) {
        if (input == null) return "";
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\"", "&quot;")
                    .replace("'", "&#x27;");
    }

    static class InfoHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String sessionId = null;
            String cookieHeader = t.getRequestHeaders().getFirst("Cookie");
            if (cookieHeader != null) {
                String[] cookies = cookieHeader.split(";");
                for (String cookie : cookies) {
                    String[] parts = cookie.trim().split("=");
                    if (parts.length == 2 && "session_id".equals(parts[0])) {
                        sessionId = parts[1];
                        break;
                    }
                }
            }

            String response;
            int statusCode = 200;

            if (sessionId != null && sessionStore.containsKey(sessionId)) {
                Map<String, String> userData = sessionStore.get(sessionId);
                String username = userData.getOrDefault("username", "N/A");
                String email = userData.getOrDefault("email", "N/A");
                String ssn = userData.getOrDefault("ssn", "0000");
                String last4ssn = ssn.length() > 4 ? ssn.substring(ssn.length() - 4) : ssn;

                response = "<html><body>" +
                           "<h1>User Information</h1>" +
                           "<p>Username: " + htmlEscape(username) + "</p>" +
                           "<p>Email: " + htmlEscape(email) + "</p>" +
                           "<p>SSN (last 4 digits): ****-**-" + htmlEscape(last4ssn) + "</p>" +
                           "</body></html>";
            } else {
                response = "<html><body>" +
                           "<h1>Not Logged In</h1>" +
                           "<p>Please <a href=\"/login\">login</a> first to see your info.</p>" +
                           "</body></html>";
                statusCode = 401; 
            }

            t.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            t.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = t.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {
            String sessionId = generateNewSessionId();
            Map<String, String> userData = new HashMap<>();
            userData.put("username", "testuser");
            userData.put("email", "test@example.com");
            userData.put("ssn", "123-456-7890");
            sessionStore.put(sessionId, userData);
            
            // Note: Secure, HttpOnly, SameSite=Strict are best practice but harder to set with this basic server.
            t.getResponseHeaders().add("Set-Cookie", "session_id=" + sessionId + "; Path=/; HttpOnly");
            
            // Redirect to /info
            t.getResponseHeaders().add("Location", "/info");
            t.sendResponseHeaders(302, -1);
        }
    }

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/info", new InfoHandler());
        server.createContext("/login", new LoginHandler());
        server.setExecutor(Executors.newCachedThreadPool()); 
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Running tests...");

        runTests(port);

        System.out.println("Tests finished. Press Enter to stop the server.");
        System.in.read();
        server.stop(0);
        System.out.println("Server stopped.");
    }
    
    private static void runTests(int port) {
        // Use a client with a cookie manager to handle sessions automatically
        CookieManager cookieManager = new CookieManager();
        cookieManager.setCookiePolicy(CookiePolicy.ACCEPT_ALL);
        HttpClient client = HttpClient.newBuilder()
            .cookieHandler(cookieManager)
            .build();
        String baseUrl = "http://localhost:" + port;

        try {
            // Test Case 1: Access /info without a session
            System.out.println("\n--- Test Case 1: Access /info without session ---");
            HttpRequest request1 = HttpRequest.newBuilder().uri(URI.create(baseUrl + "/info")).build();
            HttpResponse<String> response1 = client.send(request1, HttpResponse.BodyHandlers.ofString());
            System.out.println("Status Code: " + response1.statusCode());
            System.out.println("Response Body contains 'Not Logged In': " + response1.body().contains("Not Logged In"));
            assert response1.statusCode() == 401;

            // Test Case 2: Access /login to set up the session
            System.out.println("\n--- Test Case 2: Access /login to establish session ---");
            // The client will follow the redirect and cookies will be stored by the manager.
             HttpRequest request2 = HttpRequest.newBuilder().uri(URI.create(baseUrl + "/login")).build();
            HttpResponse<String> response2 = client.send(request2, HttpResponse.BodyHandlers.ofString());
            System.out.println("Status Code after login and redirect: " + response2.statusCode());
            System.out.println("Response Body contains 'User Information': " + response2.body().contains("User Information"));
            assert response2.statusCode() == 200;

            // Test Case 3: Access /info with a session
            System.out.println("\n--- Test Case 3: Access /info with session ---");
            HttpRequest request3 = HttpRequest.newBuilder().uri(URI.create(baseUrl + "/info")).build();
            HttpResponse<String> response3 = client.send(request3, HttpResponse.BodyHandlers.ofString());
            System.out.println("Status Code: " + response3.statusCode());
            System.out.println("Response Body contains 'testuser': " + response3.body().contains("testuser"));
            assert response3.statusCode() == 200 && response3.body().contains("testuser");

            // Test Case 4: Check if SSN is properly masked
            System.out.println("\n--- Test Case 4: Verify SSN is masked ---");
            boolean ssnMasked = response3.body().contains("****-**-7890") && !response3.body().contains("123-456-7890");
            System.out.println("SSN is masked: " + ssnMasked);
            assert ssnMasked;

            // Test Case 5: Clear cookies and access /info again
            System.out.println("\n--- Test Case 5: Access /info after clearing session ---");
            cookieManager.getCookieStore().removeAll();
            HttpRequest request5 = HttpRequest.newBuilder().uri(URI.create(baseUrl + "/info")).build();
            HttpResponse<String> response5 = client.send(request5, HttpResponse.BodyHandlers.ofString());
            System.out.println("Status Code: " + response5.statusCode());
            System.out.println("Response Body contains 'Not Logged In': " + response5.body().contains("Not Logged In"));
            assert response5.statusCode() == 401;

        } catch (Exception e) {
            System.err.println("An error occurred during tests: " + e.getMessage());
            e.printStackTrace();
        }
    }
}