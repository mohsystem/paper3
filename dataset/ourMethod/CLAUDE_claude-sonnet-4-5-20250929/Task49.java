
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.util.regex.Pattern;
import java.security.SecureRandom;
import java.util.Base64;

public class Task49 {
    private static final Pattern NAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_\\\\s]{1,100}$");
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");
    private static final int MAX_INPUT_LENGTH = 1000;
    private static final String DB_URL = "jdbc:sqlite:users.db";

    public static void main(String[] args) throws Exception {
        initDatabase();
        
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/api/user", new UserHandler());
        server.setExecutor(null);
        server.start();
        
        System.out.println("Server started on port 8080");
        
        // Test cases
        testApiEndpoint("John Doe", "john@example.com");
        testApiEndpoint("Jane Smith", "jane@example.com");
        testApiEndpoint("Bob_123", "bob@test.org");
        testApiEndpoint("Alice", "alice@domain.co.uk");
        testApiEndpoint("Charlie99", "charlie@mail.com");
        
        Thread.sleep(2000);
        server.stop(0);
        displayUsers();
    }

    private static void initDatabase() throws SQLException {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            String sql = "CREATE TABLE IF NOT EXISTS users (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "name TEXT NOT NULL," +
                        "email TEXT NOT NULL UNIQUE," +
                        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
            stmt.execute(sql);
        }
    }

    private static String validateInput(String input, Pattern pattern, String fieldName) {
        if (input == null || input.isEmpty()) {
            throw new IllegalArgumentException(fieldName + " is required");
        }
        if (input.length() > MAX_INPUT_LENGTH) {
            throw new IllegalArgumentException(fieldName + " exceeds maximum length");
        }
        String trimmed = input.trim();
        if (!pattern.matcher(trimmed).matches()) {
            throw new IllegalArgumentException(fieldName + " contains invalid characters");
        }
        return trimmed;
    }

    private static String storeUser(String name, String email) throws SQLException {
        String validName = validateInput(name, NAME_PATTERN, "Name");
        String validEmail = validateInput(email, EMAIL_PATTERN, "Email");
        
        String sql = "INSERT INTO users (name, email) VALUES (?, ?)";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, validName);
            pstmt.setString(2, validEmail);
            pstmt.executeUpdate();
            return "{\\"status\\":\\"success\\",\\"message\\":\\"User created\\"}";
        } catch (SQLException e) {
            if (e.getMessage().contains("UNIQUE constraint failed")) {
                return "{\\"status\\":\\"error\\",\\"message\\":\\"Email already exists\\"}";
            }
            throw e;
        }
    }

    static class UserHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "{\\"error\\":\\"Method not allowed\\"}");
                return;
            }

            try (InputStream is = exchange.getRequestBody();
                 ByteArrayOutputStream baos = new ByteArrayOutputStream()) {
                
                byte[] buffer = new byte[1024];
                int bytesRead;
                int totalBytes = 0;
                while ((bytesRead = is.read(buffer)) != -1) {
                    totalBytes += bytesRead;
                    if (totalBytes > MAX_INPUT_LENGTH) {
                        sendResponse(exchange, 413, "{\\"error\\":\\"Request too large\\"}");
                        return;
                    }
                    baos.write(buffer, 0, bytesRead);
                }
                
                String body = baos.toString(StandardCharsets.UTF_8.name());
                String[] parts = body.split("&");
                String name = null;
                String email = null;
                
                for (String part : parts) {
                    String[] kv = part.split("=", 2);
                    if (kv.length == 2) {
                        if ("name".equals(kv[0])) {
                            name = java.net.URLDecoder.decode(kv[1], StandardCharsets.UTF_8.name());
                        } else if ("email".equals(kv[0])) {
                            email = java.net.URLDecoder.decode(kv[1], StandardCharsets.UTF_8.name());
                        }
                    }
                }
                
                String result = storeUser(name, email);
                sendResponse(exchange, 200, result);
                
            } catch (IllegalArgumentException e) {
                sendResponse(exchange, 400, "{\\"error\\":\\"" + e.getMessage() + "\\"}");
            } catch (Exception e) {
                sendResponse(exchange, 500, "{\\"error\\":\\"Internal server error\\"}");
            }
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            byte[] bytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "application/json");
            exchange.sendResponseHeaders(statusCode, bytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(bytes);
            }
        }
    }

    private static void testApiEndpoint(String name, String email) {
        try {
            String urlParameters = "name=" + java.net.URLEncoder.encode(name, StandardCharsets.UTF_8.name()) +
                                 "&email=" + java.net.URLEncoder.encode(email, StandardCharsets.UTF_8.name());
            byte[] postData = urlParameters.getBytes(StandardCharsets.UTF_8);
            
            java.net.URL url = new java.net.URL("http://localhost:8080/api/user");
            java.net.HttpURLConnection conn = (java.net.HttpURLConnection) url.openConnection();
            conn.setRequestMethod("POST");
            conn.setDoOutput(true);
            conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
            
            try (OutputStream os = conn.getOutputStream()) {
                os.write(postData);
            }
            
            int responseCode = conn.getResponseCode();
            System.out.println("Test: " + name + " - Response: " + responseCode);
            
        } catch (Exception e) {
            System.err.println("Test failed: " + e.getMessage());
        }
    }

    private static void displayUsers() throws SQLException {
        System.out.println("\\nStored users:");
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery("SELECT name, email FROM users")) {
            while (rs.next()) {
                System.out.println("Name: " + rs.getString("name") + ", Email: " + rs.getString("email"));
            }
        }
    }
}
