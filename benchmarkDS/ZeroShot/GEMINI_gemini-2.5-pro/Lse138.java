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

// Note: This code requires the SQLite JDBC driver.
// You can get it from Maven: org.xerial:sqlite-jdbc
public class Lse138 {

    private static final String DB_URL = "jdbc:sqlite:user_moderators.db";

    // Sets up the database and populates it with initial data
    private static void setupDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            
            // Drop table if it exists to start fresh
            stmt.execute("DROP TABLE IF EXISTS users");
            
            // Create a users table
            String sql = "CREATE TABLE users (" +
                         " id INTEGER PRIMARY KEY, " +
                         " username TEXT NOT NULL, " +
                         " is_moderator BOOLEAN NOT NULL CHECK (is_moderator IN (0, 1)))";
            stmt.execute(sql);
            
            // Insert sample data
            stmt.execute("INSERT INTO users (id, username, is_moderator) VALUES (1, 'alice', 1)"); // Moderator
            stmt.execute("INSERT INTO users (id, username, is_moderator) VALUES (2, 'bob', 0)");   // Not a moderator
            stmt.execute("INSERT INTO users (id, username, is_moderator) VALUES (3, 'charlie', 0)");// Not a moderator

        } catch (SQLException e) {
            System.err.println("Database setup failed: " + e.getMessage());
        }
    }

    /**
     * Securely checks if a user is a moderator using a PreparedStatement.
     * @param userId The ID of the user to check.
     * @return true if the user is a moderator, false otherwise.
     */
    public static boolean isUserModerator(String userId) {
        // Use a safe default of false
        boolean isModerator = false;
        
        // Input validation: ensure userId is a valid integer
        int id;
        try {
            id = Integer.parseInt(userId);
        } catch (NumberFormatException e) {
            System.err.println("Invalid userId format: " + userId);
            return false;
        }

        // The query uses a placeholder (?) to prevent SQL injection
        String sql = "SELECT is_moderator FROM users WHERE id = ?";

        // try-with-resources ensures that resources are closed automatically
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            // Bind the integer id to the placeholder
            pstmt.setInt(1, id);

            try (ResultSet rs = pstmt.executeQuery()) {
                // Check if a result was returned and get the boolean value
                if (rs.next()) {
                    isModerator = rs.getBoolean("is_moderator");
                }
            }
        } catch (SQLException e) {
            System.err.println("Query failed: " + e.getMessage());
        }
        return isModerator;
    }

    static class ModeratorCheckHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String userId = null;
            String query = exchange.getRequestURI().getQuery();
            if (query != null) {
                // Simple query parser
                Map<String, String> params = new HashMap<>();
                for (String param : query.split("&")) {
                    String[] pair = param.split("=");
                    if (pair.length > 1) {
                        params.put(pair[0], pair[1]);
                    }
                }
                userId = params.get("userId");
            }

            String response;
            int statusCode = 200;

            if (userId == null || userId.trim().isEmpty()) {
                response = "{\"error\": \"userId parameter is required\"}";
                statusCode = 400;
            } else {
                boolean isMod = isUserModerator(userId);
                response = "{\"userId\": \"" + userId + "\", \"isModerator\": " + isMod + "}";
            }

            exchange.getResponseHeaders().set("Content-Type", "application/json");
            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }
    }

    public static void runTests() {
        System.out.println("--- Running 5 Test Cases ---");
        // Test Case 1: User is a moderator
        System.out.println("Test 1 (User 1 is Moderator): " + (isUserModerator("1") ? "Passed" : "Failed"));
        // Test Case 2: User is not a moderator
        System.out.println("Test 2 (User 2 is Not Moderator): " + (!isUserModerator("2") ? "Passed" : "Failed"));
        // Test Case 3: User does not exist
        System.out.println("Test 3 (User 99 does not exist): " + (!isUserModerator("99") ? "Passed" : "Failed"));
        // Test Case 4: Invalid input (non-numeric)
        System.out.println("Test 4 (Invalid input 'abc'): " + (!isUserModerator("abc") ? "Passed" : "Failed"));
        // Test Case 5: Invalid input (SQL injection attempt)
        System.out.println("Test 5 (SQLi attempt '1 OR 1=1'): " + (!isUserModerator("1 OR 1=1") ? "Passed" : "Failed"));
        System.out.println("--- Test Cases Finished ---\n");
    }

    public static void main(String[] args) throws IOException {
        // 1. Set up the database
        setupDatabase();

        // 2. Run test cases on the core logic
        runTests();

        // 3. Set up and start the web server
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/is_moderator", new ModeratorCheckHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
        System.out.println("Server started on port 8080.");
        System.out.println("Try visiting http://localhost:8080/is_moderator?userId=1");
    }
}