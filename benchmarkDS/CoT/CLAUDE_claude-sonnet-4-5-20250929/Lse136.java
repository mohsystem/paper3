
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.*;
import java.net.InetSocketAddress;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;
import java.util.Arrays;

public class Lse136 {
    private static final Map<String, byte[]> userDatabase = new HashMap<>();
    private static final Map<String, byte[]> saltDatabase = new HashMap<>();
    
    static {
        // Initialize with test users (hashed passwords)
        try {
            addUser("admin", "Admin@123");
            addUser("user1", "Pass@word1");
            addUser("testuser", "Test@123");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    private static void addUser(String username, String password) throws NoSuchAlgorithmException {
        byte[] salt = generateSalt();
        byte[] hashedPassword = hashPassword(password, salt);
        userDatabase.put(username, hashedPassword);
        saltDatabase.put(username, salt);
    }
    
    private static byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
    
    private static byte[] hashPassword(String password, byte[] salt) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-256");
        md.update(salt);
        return md.digest(password.getBytes());
    }
    
    private static boolean constantTimeEquals(byte[] a, byte[] b) {
        if (a.length != b.length) {
            return false;
        }
        int result = 0;
        for (int i = 0; i < a.length; i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    public static String check_mod(String username, String password) {
        if (username == null || password == null || username.isEmpty() || password.isEmpty()) {
            return "false";
        }
        
        // Sanitize input
        username = username.trim();
        
        if (!userDatabase.containsKey(username)) {
            return "false";
        }
        
        try {
            byte[] salt = saltDatabase.get(username);
            byte[] hashedPassword = hashPassword(password, salt);
            byte[] storedHash = userDatabase.get(username);
            
            if (constantTimeEquals(hashedPassword, storedHash)) {
                return "true";
            }
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        
        return "false";
    }
    
    static class CheckModHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if ("POST".equals(exchange.getRequestMethod())) {
                InputStreamReader isr = new InputStreamReader(exchange.getRequestBody());
                BufferedReader br = new BufferedReader(isr);
                String query = br.readLine();
                
                Map<String, String> params = parseQuery(query);
                String username = params.get("username");
                String password = params.get("password");
                
                String result = check_mod(username, password);
                
                exchange.sendResponseHeaders(200, result.length());
                OutputStream os = exchange.getResponseBody();
                os.write(result.getBytes());
                os.close();
            } else {
                String response = "Method not allowed";
                exchange.sendResponseHeaders(405, response.length());
                OutputStream os = exchange.getResponseBody();
                os.write(response.getBytes());
                os.close();
            }
        }
        
        private Map<String, String> parseQuery(String query) {
            Map<String, String> result = new HashMap<>();
            if (query != null) {
                for (String param : query.split("&")) {
                    String[] pair = param.split("=");
                    if (pair.length > 1) {
                        result.put(pair[0], pair[1]);
                    }
                }
            }
            return result;
        }
    }
    
    public static void main(String[] args) throws Exception {
        // Test cases
        System.out.println("Test Case 1 (Valid admin): " + check_mod("admin", "Admin@123"));
        System.out.println("Test Case 2 (Valid user1): " + check_mod("user1", "Pass@word1"));
        System.out.println("Test Case 3 (Invalid password): " + check_mod("admin", "wrongpass"));
        System.out.println("Test Case 4 (Non-existent user): " + check_mod("hacker", "password"));
        System.out.println("Test Case 5 (Empty credentials): " + check_mod("", ""));
        
        // Start HTTP server (commented out to avoid blocking in test environment)
        // HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        // server.createContext("/check_mod", new CheckModHandler());
        // server.setExecutor(null);
        // server.start();
        // System.out.println("Server started on port 8000");
    }
}
