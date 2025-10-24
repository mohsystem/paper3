import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

/*
 * NOTE: This is a self-contained example using only standard Java libraries.
 * For a real-world application, use a framework like Spring Boot.
 *
 * DEPENDENCIES:
 * This code requires an SQLite JDBC driver.
 * Download it from: https://github.com/xerial/sqlite-jdbc
 *
 * COMPILATION & RUN:
 * 1. Make sure 'sqlite-jdbc-VERSION.jar' is in your classpath.
 *    javac -cp ".;sqlite-jdbc-3.45.1.0.jar" Task49.java  (on Windows)
 *    javac -cp ".:sqlite-jdbc-3.45.1.0.jar" Task49.java  (on Linux/macOS)
 *
 * 2. Run the program:
 *    java -cp ".;sqlite-jdbc-3.45.1.0.jar" Task49        (on Windows)
 *    java -cp ".:sqlite-jdbc-3.45.1.0.jar" Task49        (on Linux/macOS)
 *
 * The server will start on http://localhost:8080
 */
public class Task49 {
    private static final String DB_URL = "jdbc:sqlite:user_database.db";

    public static void main(String[] args) throws IOException {
        setupDatabase();

        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/users", new UserHandler());
        server.setExecutor(null); // creates a default executor
        server.start();

        System.out.println("Server started on port 8080. Press Enter to stop.");
        System.out.println("API Endpoint: POST /users");
        System.out.println("--- 5 Test Cases (use a tool like curl) ---");
        System.out.println("1. Successful creation:");
        System.out.println("   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"alice\",\"email\":\"alice@example.com\"}' http://localhost:8080/users");
        System.out.println("\n2. Duplicate username (409 Conflict):");
        System.out.println("   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"alice\",\"email\":\"alice2@example.com\"}' http://localhost:8080/users");
        System.out.println("\n3. Duplicate email (409 Conflict):");
        System.out.println("   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"alice2\",\"email\":\"alice@example.com\"}' http://localhost:8080/users");
        System.out.println("\n4. Missing data (400 Bad Request):");
        System.out.println("   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"bob\"}' http://localhost:8080/users");
        System.out.println("\n5. Malformed JSON (400 Bad Request):");
        System.out.println("   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"carol\",\"email\":\"carol@example.com\"' http://localhost:8080/users");

        System.in.read();
        server.stop(0);
        System.out.println("Server stopped.");
    }

    private static void setupDatabase() {
        String sql = "CREATE TABLE IF NOT EXISTS users ("
                   + "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   + "username TEXT NOT NULL UNIQUE,"
                   + "email TEXT NOT NULL UNIQUE,"
                   + "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                   + ");";

        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            stmt.execute(sql);
        } catch (SQLException e) {
            System.err.println("Database setup failed: " + e.getMessage());
            System.exit(1);
        }
    }

    static class UserHandler implements HttpHandler {
        private static final Pattern EMAIL_PATTERN = Pattern.compile("^[a-zA-Z0-9_!#$%&'*+/=?`{|}~^.-]+@[a-zA-Z0-9.-]+$");
        private static final int MAX_USERNAME_LENGTH = 50;
        private static final int MAX_EMAIL_LENGTH = 254;


        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"POST".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "{\"error\":\"Method Not Allowed\"}");
                return;
            }

            try {
                String requestBody = new String(exchange.getRequestBody().readAllBytes(), StandardCharsets.UTF_8);
                Map<String, String> userData = parseSimpleJson(requestBody);

                String username = userData.get("username");
                String email = userData.get("email");

                // --- Input Validation ---
                if (username == null || username.trim().isEmpty() || email == null || email.trim().isEmpty()) {
                    sendResponse(exchange, 400, "{\"error\":\"Username and email are required\"}");
                    return;
                }
                if (username.length() > MAX_USERNAME_LENGTH || email.length() > MAX_EMAIL_LENGTH) {
                    sendResponse(exchange, 400, "{\"error\":\"Input length exceeds maximum limit\"}");
                    return;
                }
                if (!EMAIL_PATTERN.matcher(email).matches()) {
                    sendResponse(exchange, 400, "{\"error\":\"Invalid email format\"}");
                    return;
                }

                long newUserId = addUserToDb(username, email);
                String response = String.format("{\"message\":\"User created successfully\", \"user_id\":%d}", newUserId);
                sendResponse(exchange, 201, response);

            } catch (IllegalArgumentException e) {
                sendResponse(exchange, 400, "{\"error\":\"Bad Request: " + e.getMessage() + "\"}");
            } catch (SQLException e) {
                if (e.getErrorCode() == 19) { // SQLITE_CONSTRAINT
                     sendResponse(exchange, 409, "{\"error\":\"Conflict: Username or email already exists\"}");
                } else {
                    sendResponse(exchange, 500, "{\"error\":\"Internal Server Error\"}");
                }
            } catch (Exception e) {
                sendResponse(exchange, 500, "{\"error\":\"Internal Server Error\"}");
            }
        }
        
        private long addUserToDb(String username, String email) throws SQLException {
            String sql = "INSERT INTO users(username, email) VALUES(?,?)";
            long newId = -1;

            try (Connection conn = DriverManager.getConnection(DB_URL);
                 PreparedStatement pstmt = conn.prepareStatement(sql, Statement.RETURN_GENERATED_KEYS)) {
                
                // --- Parameterized Query to prevent SQL Injection ---
                pstmt.setString(1, username);
                pstmt.setString(2, email);
                
                int affectedRows = pstmt.executeUpdate();

                if (affectedRows > 0) {
                    try (ResultSet rs = pstmt.getGeneratedKeys()) {
                        if (rs.next()) {
                            newId = rs.getLong(1);
                        }
                    }
                }
            }
            return newId;
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.getResponseHeaders().set("Content-Type", "application/json");
            exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes(StandardCharsets.UTF_8));
            }
        }
        
        // Simple parser to avoid a library dependency for this example. Not robust for complex JSON.
        private Map<String, String> parseSimpleJson(String json) {
            Map<String, String> map = new HashMap<>();
            json = json.trim();
            if (!json.startsWith("{") || !json.endsWith("}")) {
                throw new IllegalArgumentException("Invalid JSON object");
            }
            json = json.substring(1, json.length() - 1).trim();
            String[] pairs = json.split(",");
            for (String pair : pairs) {
                String[] keyValue = pair.split(":", 2);
                if (keyValue.length != 2) {
                    throw new IllegalArgumentException("Invalid JSON pair");
                }
                String key = keyValue[0].trim().replace("\"", "");
                String value = keyValue[1].trim().replace("\"", "");
                map.put(key, value);
            }
            return map;
        }
    }
}