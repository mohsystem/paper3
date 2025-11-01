
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.*;
import java.net.InetSocketAddress;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.util.*;

public class Lse149 {
    private static final String DB_URL = "jdbc:sqlite:messages.db";
    private static final int MAX_USERNAME_LENGTH = 50;
    private static final int MAX_MESSAGE_LENGTH = 500;

    // Initialize database with proper schema
    private static void initDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            // Create table if not exists with proper constraints
            stmt.execute("CREATE TABLE IF NOT EXISTS messages (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                        "username TEXT NOT NULL, " +
                        "message TEXT NOT NULL, " +
                        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)");
        } catch (SQLException e) {
            System.err.println("Database initialization failed");
        }
    }

    // Secure database insertion using parameterized queries to prevent SQL injection
    private static boolean insertUserMessageInDb(String username, String message) {
        // Validate inputs before database operation
        if (username == null || message == null) {
            return false;
        }
        
        username = username.trim();
        message = message.trim();
        
        if (username.isEmpty() || message.isEmpty()) {
            return false;
        }
        
        if (username.length() > MAX_USERNAME_LENGTH || message.length() > MAX_MESSAGE_LENGTH) {
            return false;
        }

        // Use parameterized query to prevent SQL injection
        String sql = "INSERT INTO messages (username, message) VALUES (?, ?)";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setString(1, username);
            pstmt.setString(2, message);
            pstmt.executeUpdate();
            return true;
        } catch (SQLException e) {
            System.err.println("Database insert failed");
            return false;
        }
    }

    // Retrieve messages from database with limit to prevent resource exhaustion
    private static List<Map<String, String>> getMessages() {
        List<Map<String, String>> messages = new ArrayList<>();
        String sql = "SELECT username, message, created_at FROM messages ORDER BY created_at DESC LIMIT 100";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement();
             ResultSet rs = stmt.executeQuery(sql)) {
            
            while (rs.next()) {
                Map<String, String> msg = new HashMap<>();
                msg.put("username", rs.getString("username"));
                msg.put("message", rs.getString("message"));
                msg.put("created_at", rs.getString("created_at"));
                messages.add(msg);
            }
        } catch (SQLException e) {
            System.err.println("Database query failed");
        }
        return messages;
    }

    // HTML encode to prevent XSS attacks
    private static String htmlEncode(String input) {
        if (input == null) {
            return "";
        }
        return input.replace("&", "&amp;")
                   .replace("<", "&lt;")
                   .replace(">", "&gt;")
                   .replace("\\"", "&quot;")
                   .replace("'", "&#x27;")
                   .replace("/", "&#x2F;");
    }

    // Parse form data securely
    private static Map<String, String> parseFormData(String formData) {
        Map<String, String> params = new HashMap<>();
        if (formData == null || formData.isEmpty()) {
            return params;
        }
        
        String[] pairs = formData.split("&");
        for (String pair : pairs) {
            String[] keyValue = pair.split("=", 2);
            if (keyValue.length == 2) {
                try {
                    String key = URLDecoder.decode(keyValue[0], StandardCharsets.UTF_8.name());
                    String value = URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8.name());
                    params.put(key, value);
                } catch (UnsupportedEncodingException e) {
                    System.err.println("Decoding failed");
                }
            }
        }
        return params;
    }

    public static void main(String[] args) throws IOException {
        initDatabase();
        
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        
        // Main route - display messages
        server.createContext("/", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if ("GET".equals(exchange.getRequestMethod())) {
                    List<Map<String, String>> messages = getMessages();
                    
                    StringBuilder html = new StringBuilder();
                    html.append("<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">");
                    html.append("<title>Messages</title></head><body>");
                    html.append("<h1>Post a Message</h1>");
                    html.append("<form method=\\"POST\\" action=\\"/post\\">");
                    html.append("Username: <input type=\\"text\\" name=\\"username\\" maxlength=\\"50\\" required><br>");
                    html.append("Message: <textarea name=\\"message\\" maxlength=\\"500\\" required></textarea><br>");
                    html.append("<input type=\\"submit\\" value=\\"Post\\"></form>");
                    html.append("<h2>Messages:</h2><ul>");
                    
                    for (Map<String, String> msg : messages) {
                        html.append("<li><strong>").append(htmlEncode(msg.get("username"))).append("</strong>: ");
                        html.append(htmlEncode(msg.get("message")));
                        html.append(" <em>(").append(htmlEncode(msg.get("created_at"))).append(")</em></li>");
                    }
                    html.append("</ul></body></html>");
                    
                    byte[] response = html.toString().getBytes(StandardCharsets.UTF_8);
                    exchange.getResponseHeaders().set("Content-Type", "text/html; charset=UTF-8");
                    exchange.sendResponseHeaders(200, response.length);
                    try (OutputStream os = exchange.getResponseBody()) {
                        os.write(response);
                    }
                } else {
                    exchange.sendResponseHeaders(405, -1);
                }
            }
        });
        
        // Post route - handle message submission
        server.createContext("/post", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if ("POST".equals(exchange.getRequestMethod())) {
                    InputStreamReader isr = new InputStreamReader(exchange.getRequestBody(), StandardCharsets.UTF_8);
                    BufferedReader br = new BufferedReader(isr);
                    String formData = br.readLine();
                    
                    Map<String, String> params = parseFormData(formData);
                    String username = params.get("username");
                    String message = params.get("message");
                    
                    if (insertUserMessageInDb(username, message)) {
                        exchange.getResponseHeaders().set("Location", "/");
                        exchange.sendResponseHeaders(303, -1);
                    } else {
                        String error = "Invalid input";
                        byte[] response = error.getBytes(StandardCharsets.UTF_8);
                        exchange.sendResponseHeaders(400, response.length);
                        try (OutputStream os = exchange.getResponseBody()) {
                            os.write(response);
                        }
                    }
                } else {
                    exchange.sendResponseHeaders(405, -1);
                }
            }
        });
        
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 8000");
        
        // Test cases
        testInsertUserMessageInDb();
    }
    
    private static void testInsertUserMessageInDb() {
        System.out.println("Test 1: Valid input: " + insertUserMessageInDb("Alice", "Hello World"));
        System.out.println("Test 2: Valid input: " + insertUserMessageInDb("Bob", "Test message"));
        System.out.println("Test 3: Null username: " + insertUserMessageInDb(null, "Message"));
        System.out.println("Test 4: Empty message: " + insertUserMessageInDb("User", ""));
        System.out.println("Test 5: Too long username: " + insertUserMessageInDb("a".repeat(100), "msg"));
    }
}
