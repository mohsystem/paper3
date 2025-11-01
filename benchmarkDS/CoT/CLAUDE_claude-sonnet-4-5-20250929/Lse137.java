
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.util.*;

public class Lse137 {
    private static final Map<String, String> moderators = new HashMap<>();
    private static final Map<String, String> validSessions = new HashMap<>();
    
    static {
        // Example data - in production, use secure database
        try {
            moderators.put("admin", hashPassword("admin123"));
            moderators.put("mod1", hashPassword("modpass456"));
            validSessions.put("session123", "admin");
            validSessions.put("session456", "mod1");
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
    
    private static String sanitizeInput(String input) {
        if (input == null) return "";
        return input.replaceAll("[^a-zA-Z0-9]", "");
    }
    
    private static boolean auth_mod(String username, String password, String session) {
        try {
            // Sanitize inputs
            username = sanitizeInput(username);
            session = sanitizeInput(session);
            
            if (username.isEmpty() || password.isEmpty() || session.isEmpty()) {
                return false;
            }
            
            // Verify session
            String sessionUser = validSessions.get(session);
            if (sessionUser == null || !sessionUser.equals(username)) {
                return false;
            }
            
            // Verify credentials
            String storedHash = moderators.get(username);
            if (storedHash == null) {
                return false;
            }
            
            String inputHash = hashPassword(password);
            return storedHash.equals(inputHash);
            
        } catch (Exception e) {
            return false;
        }
    }
    
    private static Map<String, String> parseQuery(String query) throws UnsupportedEncodingException {
        Map<String, String> result = new HashMap<>();
        if (query == null || query.isEmpty()) {
            return result;
        }
        for (String param : query.split("&")) {
            String[] pair = param.split("=");
            if (pair.length == 2) {
                result.put(
                    URLDecoder.decode(pair[0], StandardCharsets.UTF_8.name()),
                    URLDecoder.decode(pair[1], StandardCharsets.UTF_8.name())
                );
            }
        }
        return result;
    }
    
    private static String check_mod(String username, String password, String session) {
        boolean isModerator = auth_mod(username, password, session);
        return isModerator ? "Moderator access granted" : "Access denied";
    }
    
    public static void main(String[] args) throws Exception {
        // Test cases
        System.out.println("Test Case 1: Valid moderator admin");
        System.out.println(check_mod("admin", "admin123", "session123"));
        
        System.out.println("\\nTest Case 2: Valid moderator mod1");
        System.out.println(check_mod("mod1", "modpass456", "session456"));
        
        System.out.println("\\nTest Case 3: Invalid password");
        System.out.println(check_mod("admin", "wrongpass", "session123"));
        
        System.out.println("\\nTest Case 4: Invalid session");
        System.out.println(check_mod("admin", "admin123", "invalidsession"));
        
        System.out.println("\\nTest Case 5: Non-existent user");
        System.out.println(check_mod("hacker", "password", "session999"));
        
        // Start HTTP server (commented out for test purposes)
        /*
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/check_mod", new HttpHandler() {
            public void handle(HttpExchange exchange) throws IOException {
                Map<String, String> params = parseQuery(exchange.getRequestURI().getQuery());
                String username = params.getOrDefault("username", "");
                String password = params.getOrDefault("password", "");
                String session = params.getOrDefault("session", "");
                
                String response = check_mod(username, password, session);
                exchange.sendResponseHeaders(200, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        });
        server.start();
        */
    }
}
