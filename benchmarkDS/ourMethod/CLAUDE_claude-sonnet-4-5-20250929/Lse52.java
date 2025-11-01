
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

public class Lse52 {
    // Secure session storage with expiration
    private static final Map<String, SessionData> sessions = new ConcurrentHashMap<>();
    // Store hashed passwords with salt - in production use a database
    private static final Map<String, UserCredentials> users = new ConcurrentHashMap<>();
    private static final SecureRandom secureRandom = new SecureRandom();
    private static final int SESSION_TIMEOUT_MS = 30 * 60 * 1000; // 30 minutes
    private static final int MAX_LOGIN_ATTEMPTS = 5;
    private static final Map<String, LoginAttempt> loginAttempts = new ConcurrentHashMap<>();

    static class UserCredentials {
        byte[] passwordHash;
        byte[] salt;

        UserCredentials(byte[] passwordHash, byte[] salt) {
            this.passwordHash = passwordHash;
            this.salt = salt;
        }
    }

    static class SessionData {
        String username;
        long expiry;

        SessionData(String username, long expiry) {
            this.username = username;
            this.expiry = expiry;
        }

        boolean isExpired() {
            return System.currentTimeMillis() > expiry;
        }
    }

    static class LoginAttempt {
        int attempts;
        long lastAttempt;

        LoginAttempt() {
            this.attempts = 0;
            this.lastAttempt = 0;
        }
    }

    // Hash password with salt using SHA-256 and multiple iterations
    private static byte[] hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt);
        byte[] hash = md.digest(password.getBytes(StandardCharsets.UTF_8));
        // Multiple iterations for stronger security
        for (int i = 0; i < 10000; i++) {
            md.reset();
            hash = md.digest(hash);
        }
        return hash;
    }

    // Create a new user with hashed password
    private static void createUser(String username, String password) throws NoSuchAlgorithmException {
        // Generate unique random salt for this user
        byte[] salt = new byte[16];
        secureRandom.nextBytes(salt);
        byte[] hash = hashPassword(password, salt);
        users.put(username, new UserCredentials(hash, salt));
    }

    // Verify password against stored hash
    private static boolean verifyPassword(String username, String password) throws NoSuchAlgorithmException {
        UserCredentials creds = users.get(username);
        if (creds == null) {
            return false;
        }
        byte[] hash = hashPassword(password, creds.salt);
        return MessageDigest.isEqual(hash, creds.passwordHash); // Constant-time comparison
    }

    // Generate secure session token
    private static String generateSessionToken() {
        byte[] token = new byte[32];
        secureRandom.nextBytes(token);
        return Base64.getEncoder().encodeToString(token);
    }

    // Validate session token and return username if valid
    private static String validateSession(String token) {
        if (token == null || token.isEmpty()) {
            return null;
        }
        SessionData session = sessions.get(token);
        if (session == null || session.isExpired()) {
            if (session != null) {
                sessions.remove(token); // Clean up expired session
            }
            return null;
        }
        return session.username;
    }

    // Get session token from cookie
    private static String getSessionToken(HttpExchange exchange) {
        String cookie = exchange.getRequestHeaders().getFirst("Cookie");
        if (cookie != null) {
            for (String c : cookie.split(";")) {
                c = c.trim();
                if (c.startsWith("session=")) {
                    return c.substring(8);
                }
            }
        }
        return null;
    }

    // Set secure cookie
    private static void setSessionCookie(HttpExchange exchange, String token) {
        // Secure cookie with HttpOnly and SameSite flags
        exchange.getResponseHeaders().add("Set-Cookie", 
            "session=" + token + "; HttpOnly; Secure; SameSite=Strict; Max-Age=1800; Path=/");
    }

    // Check rate limiting for login attempts
    private static boolean isRateLimited(String username) {
        LoginAttempt attempt = loginAttempts.get(username);
        if (attempt == null) {
            return false;
        }
        long timeSinceLastAttempt = System.currentTimeMillis() - attempt.lastAttempt;
        if (timeSinceLastAttempt > 300000) { // 5 minutes cooldown
            loginAttempts.remove(username);
            return false;
        }
        return attempt.attempts >= MAX_LOGIN_ATTEMPTS;
    }

    // Record login attempt
    private static void recordLoginAttempt(String username, boolean success) {
        if (success) {
            loginAttempts.remove(username);
        } else {
            LoginAttempt attempt = loginAttempts.computeIfAbsent(username, k -> new LoginAttempt());
            attempt.attempts++;
            attempt.lastAttempt = System.currentTimeMillis();
        }
    }

    static class IndexHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response = "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\"><title>Index</title></head>" +
                "<body><h1>Welcome</h1><p><a href=\\"/login\\">Login</a></p></body></html>";
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
            exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    static class LoginHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("GET".equals(exchange.getRequestMethod())) {
                String response = "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\"><title>Login</title></head>" +
                    "<body><h1>Login</h1>" +
                    "<form method=\\"POST\\" action=\\"/login\\">" +
                    "<label>Username: <input type=\\"text\\" name=\\"username\\" required maxlength=\\"50\\"></label><br>" +
                    "<label>Password: <input type=\\"password\\" name=\\"password\\" required maxlength=\\"100\\"></label><br>" +
                    "<button type=\\"submit\\">Login</button>" +
                    "</form></body></html>";
                exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
                exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
                exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
                exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
            } else if ("POST".equals(exchange.getRequestMethod())) {
                doLogin(exchange);
            } else {
                exchange.sendResponseHeaders(405, -1);
            }
        }

        private void doLogin(HttpExchange exchange) throws IOException {
            // Read and validate input length
            InputStream is = exchange.getRequestBody();
            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            byte[] data = new byte[1024];
            int bytesRead;
            int totalBytes = 0;
            int maxSize = 4096; // Maximum request size
            
            while ((bytesRead = is.read(data)) != -1) {
                totalBytes += bytesRead;
                if (totalBytes > maxSize) {
                    exchange.sendResponseHeaders(413, -1); // Request too large
                    return;
                }
                buffer.write(data, 0, bytesRead);
            }

            String body = buffer.toString(StandardCharsets.UTF_8.name());
            Map<String, String> params = parseFormData(body);

            String username = params.get("username");
            String password = params.get("password");

            // Input validation
            if (username == null || password == null || 
                username.isEmpty() || password.isEmpty() ||
                username.length() > 50 || password.length() > 100) {
                sendErrorResponse(exchange, "Invalid input");
                return;
            }

            // Sanitize username to alphanumeric only
            if (!username.matches("^[a-zA-Z0-9_]+$")) {
                sendErrorResponse(exchange, "Invalid username format");
                return;
            }

            // Check rate limiting
            if (isRateLimited(username)) {
                sendErrorResponse(exchange, "Too many login attempts. Please try again later.");
                return;
            }

            try {
                // Verify credentials
                boolean valid = verifyPassword(username, password);
                recordLoginAttempt(username, valid);

                if (valid) {
                    // Create session
                    String token = generateSessionToken();
                    long expiry = System.currentTimeMillis() + SESSION_TIMEOUT_MS;
                    sessions.put(token, new SessionData(username, expiry));
                    
                    // Set secure cookie and redirect
                    setSessionCookie(exchange, token);
                    exchange.getResponseHeaders().set("Location", "/secret");
                    exchange.sendResponseHeaders(302, -1);
                } else {
                    sendErrorResponse(exchange, "Invalid credentials");
                }
            } catch (NoSuchAlgorithmException e) {
                sendErrorResponse(exchange, "Authentication error");
            }
        }

        private Map<String, String> parseFormData(String body) {
            Map<String, String> params = new HashMap<>();
            if (body == null || body.isEmpty()) {
                return params;
            }
            for (String pair : body.split("&")) {
                String[] kv = pair.split("=", 2);
                if (kv.length == 2) {
                    try {
                        String key = java.net.URLDecoder.decode(kv[0], StandardCharsets.UTF_8.name());
                        String value = java.net.URLDecoder.decode(kv[1], StandardCharsets.UTF_8.name());
                        params.put(key, value);
                    } catch (UnsupportedEncodingException e) {
                        // Should not happen with UTF-8
                    }
                }
            }
            return params;
        }

        private void sendErrorResponse(HttpExchange exchange, String message) throws IOException {
            String response = "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\"><title>Login Error</title></head>" +
                "<body><h1>Error</h1><p>" + escapeHtml(message) + "</p>" +
                "<p><a href=\\"/login\\">Try again</a></p></body></html>";
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
            exchange.sendResponseHeaders(401, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    static class SecretHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String token = getSessionToken(exchange);
            String username = validateSession(token);

            if (username == null) {
                // Redirect to login if not authenticated
                exchange.getResponseHeaders().set("Location", "/login");
                exchange.sendResponseHeaders(302, -1);
                return;
            }

            String response = "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\"><title>Secret</title></head>" +
                "<body><h1>Secret settings</h1>" +
                "<p>Welcome, " + escapeHtml(username) + "!</p>" +
                "<p><a href=\\"/\\">Home</a></p></body></html>";
            exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
            exchange.getResponseHeaders().set("X-Content-Type-Options", "nosniff");
            exchange.getResponseHeaders().set("X-Frame-Options", "DENY");
            exchange.getResponseHeaders().set("Cache-Control", "no-store, no-cache, must-revalidate");
            exchange.sendResponseHeaders(200, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
    }

    // HTML escape to prevent XSS
    private static String escapeHtml(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                    .replace("<", "&lt;")
                    .replace(">", "&gt;")
                    .replace("\\"", "&quot;")
                    .replace("'", "&#x27;");
    }

    public static void main(String[] args) {
        try {
            // Create test users with secure password hashing
            createUser("admin", "SecureP@ss123!");
            createUser("user1", "TestUser#456");
            createUser("user2", "Demo$Pass789");
            createUser("testuser", "MyTest&Pass000");
            createUser("guest", "Guest!Pass321");

            HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
            server.createContext("/", new IndexHandler());
            server.createContext("/login", new LoginHandler());
            server.createContext("/secret", new SecretHandler());
            server.setExecutor(null);
            server.start();

            System.out.println("Server started on port 8000");
            System.out.println("Test users created:");
            System.out.println("  admin / SecureP@ss123!");
            System.out.println("  user1 / TestUser#456");
            System.out.println("  user2 / Demo$Pass789");
            System.out.println("  testuser / MyTest&Pass000");
            System.out.println("  guest / Guest!Pass321");

        } catch (Exception e) {
            System.err.println("Server error: " + e.getMessage());
        }
    }
}
