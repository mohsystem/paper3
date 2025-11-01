
import com.sun.net.httpserver.*;
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Lse52 {
    private static final Map<String, String> users = new ConcurrentHashMap<>();
    private static final Map<String, String> sessions = new ConcurrentHashMap<>();
    private static final Map<String, Integer> loginAttempts = new ConcurrentHashMap<>();
    private static final int MAX_ATTEMPTS = 5;
    
    static {
        try {
            users.put("admin", hashPassword("admin123"));
            users.put("user1", hashPassword("password1"));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    private static String hashPassword(String password) throws Exception {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        byte[] hash = md.digest(password.getBytes(StandardCharsets.UTF_8));
        StringBuilder hexString = new StringBuilder();
        for (byte b : hash) {
            String hex = Integer.toHexString(0xff & b);
            if (hex.length() == 1) hexString.append('0');
            hexString.append(hex);
        }
        return hexString.toString();
    }
    
    private static String generateSessionToken() {
        SecureRandom random = new SecureRandom();
        byte[] bytes = new byte[32];
        random.nextBytes(bytes);
        return Base64.getEncoder().encodeToString(bytes);
    }
    
    private static String getSession(HttpExchange exchange) {
        String cookie = exchange.getRequestHeaders().getFirst("Cookie");
        if (cookie != null && cookie.startsWith("session=")) {
            return cookie.substring(8);
        }
        return null;
    }
    
    private static boolean isAuthenticated(HttpExchange exchange) {
        String sessionToken = getSession(exchange);
        return sessionToken != null && sessions.containsKey(sessionToken);
    }
    
    private static void sendResponse(HttpExchange exchange, int code, String response) throws IOException {
        exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
        exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
        exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
        exchange.sendResponseHeaders(code, response.getBytes(StandardCharsets.UTF_8).length);
        OutputStream os = exchange.getResponseBody();
        os.write(response.getBytes(StandardCharsets.UTF_8));
        os.close();
    }
    
    private static void handleIndex(HttpExchange exchange) throws IOException {
        String html = "<!DOCTYPE html><html><head><title>Index</title></head><body>" +
                     "<h1>Welcome</h1><a href='/login'>Login</a></body></html>";
        sendResponse(exchange, 200, html);
    }
    
    private static void handleLogin(HttpExchange exchange) throws IOException {
        if ("GET".equals(exchange.getRequestMethod())) {
            String html = "<!DOCTYPE html><html><head><title>Login</title></head><body>" +
                         "<h1>Login</h1><form method='POST' action='/login'>" +
                         "<input type='text' name='username' placeholder='Username' required pattern='[a-zA-Z0-9]+' maxlength='50'/><br/>" +
                         "<input type='password' name='password' placeholder='Password' required maxlength='100'/><br/>" +
                         "<button type='submit'>Login</button></form></body></html>";
            sendResponse(exchange, 200, html);
        } else if ("POST".equals(exchange.getRequestMethod())) {
            doLogin(exchange);
        }
    }
    
    private static void doLogin(HttpExchange exchange) throws IOException {
        InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
        BufferedReader br = new BufferedReader(isr);
        String formData = br.readLine();
        
        Map<String, String> params = new HashMap<>();
        if (formData != null) {
            for (String param : formData.split("&")) {
                String[] pair = param.split("=");
                if (pair.length == 2) {
                    params.put(URLDecoder.decode(pair[0], StandardCharsets.UTF_8), 
                              URLDecoder.decode(pair[1], StandardCharsets.UTF_8));
                }
            }
        }
        
        String username = params.get("username");
        String password = params.get("password");
        
        if (username == null || password == null || 
            !username.matches("[a-zA-Z0-9]+") || username.length() > 50) {
            sendResponse(exchange, 400, "Invalid input");
            return;
        }
        
        String clientIP = exchange.getRemoteAddress().getAddress().getHostAddress();
        int attempts = loginAttempts.getOrDefault(clientIP, 0);
        
        if (attempts >= MAX_ATTEMPTS) {
            sendResponse(exchange, 429, "Too many login attempts. Try again later.");
            return;
        }
        
        try {
            String hashedPassword = hashPassword(password);
            if (users.containsKey(username) && users.get(username).equals(hashedPassword)) {
                String sessionToken = generateSessionToken();
                sessions.put(sessionToken, username);
                loginAttempts.remove(clientIP);
                
                exchange.getResponseHeaders().add("Set-Cookie", 
                    "session=" + sessionToken + "; HttpOnly; SameSite=Strict; Path=/");
                exchange.getResponseHeaders().add("Location", "/secret");
                sendResponse(exchange, 302, "");
            } else {
                loginAttempts.put(clientIP, attempts + 1);
                sendResponse(exchange, 401, "Invalid credentials");
            }
        } catch (Exception e) {
            sendResponse(exchange, 500, "Server error");
        }
    }
    
    private static void handleSecret(HttpExchange exchange) throws IOException {
        if (!isAuthenticated(exchange)) {
            exchange.getResponseHeaders().add("Location", "/login");
            sendResponse(exchange, 302, "");
            return;
        }
        
        String html = "<!DOCTYPE html><html><head><title>Secret</title></head><body>" +
                     "<h1>Secret settings</h1></body></html>";
        sendResponse(exchange, 200, html);
    }
    
    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/", exchange -> handleIndex(exchange));
        server.createContext("/login", exchange -> handleLogin(exchange));
        server.createContext("/secret", exchange -> handleSecret(exchange));
        server.setExecutor(null);
        server.start();
        
        System.out.println("Server started on port 8000");
        System.out.println("Test cases:");
        System.out.println("1. Visit http://localhost:8000/ - should show index with login link");
        System.out.println("2. Click login link - should show login form");
        System.out.println("3. Login with admin/admin123 - should redirect to secret page");
        System.out.println("4. Try accessing /secret without login - should redirect to login");
        System.out.println("5. Login with wrong credentials - should show error");
    }
}
