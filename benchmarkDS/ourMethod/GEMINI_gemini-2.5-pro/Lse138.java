import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.HashMap;
import java.util.Map;
import java.nio.charset.StandardCharsets;

/**
 * Note: This example requires the sqlite-jdbc library.
 * You can download it and run the compiled code with:
 * java -cp ".:sqlite-jdbc-3.36.0.3.jar" Lse138
 * The name of the jar file might be different depending on the version.
 */
public class Lse138 {

    private static final String DB_URL = "jdbc:sqlite:user_database.db";

    public static void main(String[] args) throws IOException {
        setupDatabase();

        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/is_moderator", new IsModeratorHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Press Enter to stop the server.");

        System.out.println("\n--- Test Cases ---");
        System.out.println("1. Moderator user: curl \"http://localhost:8080/is_moderator?user_id=1\"");
        System.out.println("2. Non-moderator user: curl \"http://localhost:8080/is_moderator?user_id=2\"");
        System.out.println("3. Non-existent user: curl \"http://localhost:8080/is_moderator?user_id=99\"");
        System.out.println("4. Invalid input: curl \"http://localhost:8080/is_moderator?user_id=abc\"");
        System.out.println("5. Missing input: curl \"http://localhost:8080/is_moderator\"");

        System.in.read();
        server.stop(0);
        System.out.println("Server stopped.");
    }

    private static void setupDatabase() {
        // In a real application, the database connection string should not be hardcoded.
        // It should be loaded from a secure configuration source.
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            String dropTableSql = "DROP TABLE IF EXISTS users";
            stmt.execute(dropTableSql);

            String createTableSql = "CREATE TABLE IF NOT EXISTS users ("
                                  + " id INTEGER PRIMARY KEY,"
                                  + " username TEXT NOT NULL,"
                                  + " is_moderator INTEGER NOT NULL"
                                  + ");";
            stmt.execute(createTableSql);

            String insertSql = "INSERT INTO users(id, username, is_moderator) VALUES(?, ?, ?)";
            try (PreparedStatement pstmt = conn.prepareStatement(insertSql)) {
                pstmt.setInt(1, 1);
                pstmt.setString(2, "Alice");
                pstmt.setInt(3, 1); // Alice is a moderator
                pstmt.executeUpdate();

                pstmt.setInt(1, 2);
                pstmt.setString(2, "Bob");
                pstmt.setInt(3, 0); // Bob is not a moderator
                pstmt.executeUpdate();
            }

            System.out.println("Database initialized successfully.");

        } catch (SQLException e) {
            System.err.println("Database setup failed: " + e.getMessage());
            // In a real application, handle this more gracefully.
            System.exit(1);
        }
    }

    static class IsModeratorHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "{\"error\": \"Method Not Allowed\"}");
                return;
            }

            String query = exchange.getRequestURI().getQuery();
            Map<String, String> params = parseQuery(query);
            String userIdStr = params.get("user_id");

            if (userIdStr == null || userIdStr.trim().isEmpty()) {
                sendResponse(exchange, 400, "{\"error\": \"user_id parameter is required\"}");
                return;
            }

            int userId;
            try {
                // Rule #1: Ensure all input is validated
                userId = Integer.parseInt(userIdStr);
            } catch (NumberFormatException e) {
                sendResponse(exchange, 400, "{\"error\": \"Invalid user_id format, must be an integer\"}");
                return;
            }
            
            try {
                boolean isModerator = checkModeratorStatus(userId);
                String jsonResponse = String.format("{\"user_id\": %d, \"is_moderator\": %b}", userId, isModerator);
                sendResponse(exchange, 200, jsonResponse);
            } catch (UserNotFoundException e) {
                String jsonResponse = String.format("{\"error\": \"User with id %d not found\"}", userId);
                sendResponse(exchange, 404, jsonResponse);
            } catch (SQLException e) {
                // Do not leak internal error details to the client
                System.err.println("Database query failed: " + e.getMessage());
                sendResponse(exchange, 500, "{\"error\": \"Internal server error\"}");
            }
        }

        private boolean checkModeratorStatus(int userId) throws SQLException, UserNotFoundException {
            // Rule #7: Avoid hardcoding credentials. DB_URL is a constant for simplicity,
            // but should be externalized in a production environment.
            // Rule #10: Use try-with-resources for automatic resource management.
            try (Connection conn = DriverManager.getConnection(DB_URL)) {
                // Rule #2: Use parameterized queries to prevent SQL injection (CWE-89)
                String sql = "SELECT is_moderator FROM users WHERE id = ?";
                try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                    pstmt.setInt(1, userId);
                    try (ResultSet rs = pstmt.executeQuery()) {
                        if (rs.next()) {
                            return rs.getInt("is_moderator") == 1;
                        } else {
                            throw new UserNotFoundException();
                        }
                    }
                }
            }
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            exchange.getResponseHeaders().set("Content-Type", "application/json; charset=UTF-8");
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }

        private Map<String, String> parseQuery(String query) {
            Map<String, String> result = new HashMap<>();
            if (query != null) {
                for (String param : query.split("&")) {
                    String[] entry = param.split("=");
                    if (entry.length > 1) {
                        result.put(entry[0], entry[1]);
                    } else {
                        result.put(entry[0], "");
                    }
                }
            }
            return result;
        }
    }
    
    static class UserNotFoundException extends Exception {
        // Custom exception for clarity
    }
}