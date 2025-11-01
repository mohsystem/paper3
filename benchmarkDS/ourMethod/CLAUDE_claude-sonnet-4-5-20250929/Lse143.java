
import java.sql.*;
import java.util.regex.*;
import java.io.*;
import java.net.*;
import java.nio.charset.StandardCharsets;
import com.sun.net.httpserver.*;

public class Lse143 {
    // Database configuration from environment variables
    private static final String DB_URL = System.getenv("DB_URL") != null ? 
        System.getenv("DB_URL") : "jdbc:sqlite:subscribers.db";
    private static final String DB_USER = System.getenv("DB_USER");
    private static final String DB_PASSWORD = System.getenv("DB_PASSWORD");
    
    // Email validation pattern - RFC 5322 simplified
    private static final Pattern EMAIL_PATTERN = Pattern.compile(
        "^[a-zA-Z0-9_+&*-]+(?:\\\\.[a-zA-Z0-9_+&*-]+)*@(?:[a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,7}$"
    );
    
    // Maximum email length to prevent DOS
    private static final int MAX_EMAIL_LENGTH = 254;
    
    /**
     * Validates email format and length
     * @param email Email to validate
     * @return true if valid, false otherwise
     */
    private static boolean isValidEmail(String email) {
        if (email == null || email.isEmpty() || email.length() > MAX_EMAIL_LENGTH) {
            return false;
        }
        return EMAIL_PATTERN.matcher(email).matches();
    }
    
    /**
     * Initializes database with subscribers table
     */
    private static void initializeDatabase() {
        // Use try-with-resources to ensure connection is closed
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
             Statement stmt = conn.createStatement()) {
            
            String createTableSQL = "CREATE TABLE IF NOT EXISTS subscribers (" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT, " +
                "email VARCHAR(254) UNIQUE NOT NULL)";
            stmt.execute(createTableSQL);
            
        } catch (SQLException e) {
            // Log error internally but don't expose details
            System.err.println("Database initialization failed");
        }
    }
    
    /**
     * Handles unsubscribe request
     * @param email Email address to unsubscribe
     * @return Response message
     */
    public static String handleUnsubscribe(String email) {
        // Input validation - check for null/empty
        if (email == null || email.trim().isEmpty()) {
            return "Invalid request: email parameter is required";
        }
        
        // Trim and normalize input
        email = email.trim().toLowerCase();
        
        // Validate email format and length
        if (!isValidEmail(email)) {
            return "Invalid email format";
        }
        
        // Use try-with-resources for automatic resource cleanup
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD)) {
            
            // First check if email exists - using parameterized query to prevent SQL injection
            String checkSQL = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
            try (PreparedStatement checkStmt = conn.prepareStatement(checkSQL)) {
                checkStmt.setString(1, email);
                
                try (ResultSet rs = checkStmt.executeQuery()) {
                    if (rs.next() && rs.getInt(1) > 0) {
                        // Email exists, proceed with deletion using parameterized query
                        String deleteSQL = "DELETE FROM subscribers WHERE email = ?";
                        try (PreparedStatement deleteStmt = conn.prepareStatement(deleteSQL)) {
                            deleteStmt.setString(1, email);
                            int rowsAffected = deleteStmt.executeUpdate();
                            
                            if (rowsAffected > 0) {
                                return "Successfully unsubscribed";
                            } else {
                                // Unexpected case - log internally
                                return "Unsubscribe failed";
                            }
                        }
                    } else {
                        // Email not found in database
                        return "Email address is not subscribed";
                    }
                }
            }
            
        } catch (SQLException e) {
            // Log error details internally, return generic message
            System.err.println("Database error occurred");
            return "An error occurred processing your request";
        }
    }
    
    /**
     * HTTP handler for unsubscribe endpoint
     */
    static class UnsubscribeHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // Only accept GET requests
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                String response = "Method not allowed";
                exchange.sendResponseHeaders(405, response.length());
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
                return;
            }
            
            try {
                // Parse query parameters safely
                String query = exchange.getRequestURI().getQuery();
                String email = null;
                
                if (query != null) {
                    String[] params = query.split("&");
                    for (String param : params) {
                        String[] keyValue = param.split("=", 2);
                        if (keyValue.length == 2 && "email".equals(keyValue[0])) {
                            // URL decode the email parameter
                            email = URLDecoder.decode(keyValue[1], StandardCharsets.UTF_8.name());
                            break;
                        }
                    }
                }
                
                // Handle unsubscribe
                String response = handleUnsubscribe(email);
                
                // Send response
                exchange.getResponseHeaders().set("Content-Type", "text/plain; charset=UTF-8");
                exchange.sendResponseHeaders(200, response.length());
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
                
            } catch (Exception e) {
                // Catch any unexpected exceptions
                String response = "An error occurred";
                exchange.sendResponseHeaders(500, response.length());
                try (OutputStream os = exchange.getResponseBody()) {
                    os.write(response.getBytes(StandardCharsets.UTF_8));
                }
            }
        }
    }
    
    public static void main(String[] args) {
        // Initialize database
        initializeDatabase();
        
        // Test cases
        System.out.println("Test Case 1 - Valid email unsubscribe:");
        System.out.println(handleUnsubscribe("test@example.com"));
        
        System.out.println("\\nTest Case 2 - Email not in database:");
        System.out.println(handleUnsubscribe("notfound@example.com"));
        
        System.out.println("\\nTest Case 3 - Invalid email format:");
        System.out.println(handleUnsubscribe("invalid-email"));
        
        System.out.println("\\nTest Case 4 - Null email:");
        System.out.println(handleUnsubscribe(null));
        
        System.out.println("\\nTest Case 5 - Empty email:");
        System.out.println(handleUnsubscribe(""));
    }
}
