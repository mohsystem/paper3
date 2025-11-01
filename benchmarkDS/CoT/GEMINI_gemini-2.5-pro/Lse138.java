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
import java.util.concurrent.Executors;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

public class Lse138 {

    // In a real application, use a proper database and external configuration.
    // H2 in-memory database is used for this self-contained example.
    private static final String DB_URL = "jdbc:h2:mem:testdb;DB_CLOSE_DELAY=-1";
    private static final String DB_USER = "sa";
    private static final String DB_PASSWORD = "";

    /**
     * The core logic to check if a user is a moderator.
     * Uses a PreparedStatement to prevent SQL injection.
     *
     * @param userId The ID of the user to check.
     * @return true if the user is a moderator, false otherwise.
     * @throws SQLException if a database access error occurs.
     */
    public static boolean isUserModerator(int userId) throws SQLException {
        // Use try-with-resources to ensure the connection and statement are closed automatically.
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD)) {
            // SQL injection is prevented by using a PreparedStatement with a '?' placeholder.
            String sql = "SELECT is_moderator FROM users WHERE id = ?";
            try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                // Bind the integer userId to the placeholder.
                pstmt.setInt(1, userId);
                try (ResultSet rs = pstmt.executeQuery()) {
                    if (rs.next()) {
                        return rs.getBoolean("is_moderator");
                    }
                }
            }
        }
        return false; // User not found
    }

    // Handler for the /isModerator route
    static class IsModeratorHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode = 200;

            try {
                // Parse query parameters
                Map<String, String> params = queryToMap(exchange.getRequestURI().getQuery());
                String userIdStr = params.get("userId");

                if (userIdStr == null) {
                    statusCode = 400; // Bad Request
                    response = "{\"error\": \"userId parameter is missing\"}";
                } else {
                    int userId = Integer.parseInt(userIdStr);
                    boolean isModerator = isUserModerator(userId);
                    response = String.format("{\"userId\": %d, \"isModerator\": %b}", userId, isModerator);
                }
            } catch (NumberFormatException e) {
                statusCode = 400; // Bad Request
                response = "{\"error\": \"Invalid userId format. Must be an integer.\"}";
            } catch (SQLException e) {
                statusCode = 500; // Internal Server Error
                response = "{\"error\": \"Database error occurred.\"}";
                // In a real app, log the exception e.g., e.printStackTrace();
            }

            exchange.getResponseHeaders().set("Content-Type", "application/json");
            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(response.getBytes());
            }
        }

        private Map<String, String> queryToMap(String query) {
            Map<String, String> result = new HashMap<>();
            if (query == null) {
                return result;
            }
            for (String param : query.split("&")) {
                String[] entry = param.split("=");
                if (entry.length > 1) {
                    result.put(entry[0], entry[1]);
                } else {
                    result.put(entry[0], "");
                }
            }
            return result;
        }
    }
    
    /**
     * Sets up the in-memory H2 database, creates the users table,
     * and populates it with sample data.
     */
    private static void setupDatabase() {
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
             Statement stmt = conn.createStatement()) {
            
            System.out.println("Setting up the database...");
            // Create table
            String createTableSql = "CREATE TABLE users (" +
                                    "id INT PRIMARY KEY, " +
                                    "username VARCHAR(255), " +
                                    "is_moderator BOOLEAN)";
            stmt.execute(createTableSql);

            // Insert sample data
            stmt.execute("INSERT INTO users VALUES (1, 'Alice', true)");
            stmt.execute("INSERT INTO users VALUES (2, 'Bob', false)");
            stmt.execute("INSERT INTO users VALUES (3, 'Charlie', true)");
            stmt.execute("INSERT INTO users VALUES (4, 'David', false)");
            
            System.out.println("Database setup complete.");
        } catch (SQLException e) {
            // If the table already exists, this might fail, which is okay for this example.
            System.err.println("Database setup failed (might be already set up): " + e.getMessage());
        }
    }


    public static void main(String[] args) throws IOException {
        // 1. Setup the database with sample data
        setupDatabase();
        
        // 2. Run test cases for the core logic before starting the server
        System.out.println("\n--- Running Test Cases ---");
        // Test Case 1: Moderator user
        runTestCase(1, true);
        // Test Case 2: Non-moderator user
        runTestCase(2, false);
        // Test Case 3: Another moderator user
        runTestCase(3, true);
        // Test Case 4: Another non-moderator user
        runTestCase(4, false);
        // Test Case 5: Non-existent user
        runTestCase(99, false);
        System.out.println("--- Test Cases Finished ---\n");


        // 3. Create and start the web server
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/isModerator", new IsModeratorHandler());
        server.setExecutor(Executors.newCachedThreadPool()); 
        server.start();

        System.out.println("Server started on port 8080.");
        System.out.println("Try accessing http://localhost:8080/isModerator?userId=1");
        System.out.println("Try accessing http://localhost:8080/isModerator?userId=2");
    }

    private static void runTestCase(int userId, boolean expected) {
        try {
            boolean actual = isUserModerator(userId);
            System.out.printf("Test for userId=%d: Expected=%b, Actual=%b -> %s%n",
                    userId, expected, actual, actual == expected ? "PASS" : "FAIL");
        } catch (SQLException e) {
            System.err.printf("Test for userId=%d failed with exception: %s%n", userId, e.getMessage());
        }
    }
}