
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.sql.*;
import java.nio.charset.StandardCharsets;
import java.util.regex.Pattern;

public class Lse138 {
    // Database connection parameters from environment variables
    private static final String DB_URL = System.getenv().getOrDefault("DB_URL", "jdbc:postgresql://localhost:5432/testdb");
    private static final String DB_USER = System.getenv().getOrDefault("DB_USER", "testuser");
    private static final String DB_PASSWORD = System.getenv().getOrDefault("DB_PASSWORD", "testpass");
    
    // Input validation pattern: alphanumeric and underscore only, max 50 chars
    private static final Pattern USERNAME_PATTERN = Pattern.compile("^[a-zA-Z0-9_]{1,50}$");

    public static void main(String[] args) {
        try {
            // Test cases
            testCheckModerator("admin");
            testCheckModerator("user123");
            testCheckModerator("moderator1");
            testCheckModerator("guest");
            testCheckModerator("invalid-user!");
            
            // Start web server
            HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
            server.createContext("/check-moderator", new ModeratorHandler());
            server.setExecutor(null);
            System.out.println("Server started on port 8080");
            server.start();
        } catch (IOException e) {
            System.err.println("Server error: Failed to start");
        }
    }

    static class ModeratorHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode;
            
            try {
                // Only accept GET requests
                if (!"GET".equals(exchange.getRequestMethod())) {
                    response = "Method not allowed";
                    statusCode = 405;
                } else {
                    // Extract username from query parameter
                    String query = exchange.getRequestURI().getQuery();
                    String username = extractUsername(query);
                    
                    if (username == null) {
                        response = "Invalid request";
                        statusCode = 400;
                    } else {
                        // Check moderator status
                        Boolean isModerator = checkModerator(username);
                        if (isModerator == null) {
                            response = "Service unavailable";
                            statusCode = 503;
                        } else {
                            response = isModerator ? "User is moderator" : "User is not moderator";
                            statusCode = 200;
                        }
                    }
                }
            } catch (Exception e) {
                // Generic error response without internal details
                response = "Request processing failed";
                statusCode = 500;
            }
            
            // Send response
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
        
        private String extractUsername(String query) {
            // Validate query parameter format
            if (query == null || !query.startsWith("username=")) {
                return null;
            }
            
            String username = query.substring(9);
            
            // Validate username format: alphanumeric and underscore only
            if (!USERNAME_PATTERN.matcher(username).matches()) {
                return null;
            }
            
            return username;
        }
    }

    /**
     * Checks if a user is a moderator by querying the database.
     * Uses parameterized query to prevent SQL injection.
     * 
     * @param username The username to check (already validated)
     * @return Boolean: true if moderator, false if not, null on error
     */
    public static Boolean checkModerator(String username) {
        // Validate input before database query
        if (username == null || !USERNAME_PATTERN.matcher(username).matches()) {
            return false;
        }
        
        // Use try-with-resources for automatic resource cleanup
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD)) {
            // Parameterized query prevents SQL injection
            String sql = "SELECT is_moderator FROM users WHERE username = ?";
            
            try (PreparedStatement pstmt = conn.prepareStatement(sql)) {
                // Set parameter using type-safe method
                pstmt.setString(1, username);
                
                try (ResultSet rs = pstmt.executeQuery()) {
                    if (rs.next()) {
                        return rs.getBoolean("is_moderator");
                    }
                    return false; // User not found
                }
            }
        } catch (SQLException e) {
            // Log error without exposing internal details
            System.err.println("Database query failed");
            return null; // Indicate error condition
        }
    }

    private static void testCheckModerator(String username) {
        System.out.println("Testing username: " + username);
        Boolean result = checkModerator(username);
        if (result == null) {
            System.out.println("Result: Database error");
        } else {
            System.out.println("Result: " + (result ? "Moderator" : "Not moderator"));
        }
    }
}
