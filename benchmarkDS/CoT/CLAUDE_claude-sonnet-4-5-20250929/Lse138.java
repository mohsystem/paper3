
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.sql.*;

public class Lse138 {
    private static final String DB_URL = System.getenv().getOrDefault("DB_URL", "jdbc:sqlite:test.db");
    private static final String DB_USER = System.getenv().getOrDefault("DB_USER", "user");
    private static final String DB_PASSWORD = System.getenv().getOrDefault("DB_PASSWORD", "password");

    public static boolean checkIfModerator(String username) {
        if (username == null || username.trim().isEmpty()) {
            return false;
        }
        
        // Use parameterized query to prevent SQL injection
        String query = "SELECT is_moderator FROM users WHERE username = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setString(1, username);
            ResultSet rs = pstmt.executeQuery();
            
            if (rs.next()) {
                return rs.getBoolean("is_moderator");
            }
        } catch (SQLException e) {
            System.err.println("Database error occurred");
            return false;
        }
        return false;
    }

    static class ModeratorHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String query = exchange.getRequestURI().getQuery();
            String username = "";
            
            if (query != null && query.contains("username=")) {
                username = query.split("username=")[1].split("&")[0];
                username = java.net.URLDecoder.decode(username, "UTF-8");
            }
            
            boolean isModerator = checkIfModerator(username);
            String response = "{\\"is_moderator\\": " + isModerator + "}";
            
            exchange.getResponseHeaders().set("Content-Type", "application/json");
            exchange.sendResponseHeaders(200, response.getBytes().length);
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }

    public static void main(String[] args) throws Exception {
        // Initialize test database
        initTestDatabase();
        
        // Test cases
        System.out.println("Test Case 1: " + checkIfModerator("admin")); // true
        System.out.println("Test Case 2: " + checkIfModerator("user1")); // false
        System.out.println("Test Case 3: " + checkIfModerator("moderator1")); // true
        System.out.println("Test Case 4: " + checkIfModerator("")); // false
        System.out.println("Test Case 5: " + checkIfModerator(null)); // false
        
        // Start web server
        HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
        server.createContext("/check-moderator", new ModeratorHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 8000");
    }

    private static void initTestDatabase() throws SQLException {
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
             Statement stmt = conn.createStatement()) {
            stmt.execute("CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, is_moderator BOOLEAN)");
            stmt.execute("DELETE FROM users");
            stmt.execute("INSERT INTO users VALUES ('admin', 1)");
            stmt.execute("INSERT INTO users VALUES ('user1', 0)");
            stmt.execute("INSERT INTO users VALUES ('moderator1', 1)");
            stmt.execute("INSERT INTO users VALUES ('user2', 0)");
        }
    }
}
