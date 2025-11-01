
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
import java.util.HashMap;
import java.util.Map;
import java.nio.charset.StandardCharsets;

public class Lse86 {
    // Database connection parameters - should be loaded from secure config
    private static final String DB_URL = "jdbc:mysql://localhost:3306/weather?useSSL=true&requireSSL=true";
    private static final String DB_USER = "weatherapp";
    private static final String DB_PASSWORD = System.getenv("DB_PASSWORD"); // Load from environment
    
    public static void main(String[] args) {
        try {
            // Test cases
            System.out.println("Starting web server on port 8000...");
            HttpServer server = HttpServer.create(new InetSocketAddress(8000), 0);
            server.createContext("/api/temperature_for_location", new TemperatureHandler());
            server.setExecutor(null);
            server.start();
            System.out.println("Server started. Test with:");
            System.out.println("curl 'http://localhost:8000/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=6&day=15&grib_file=gfs_20230615.grb'");
        } catch (Exception e) {
            System.err.println("Server error: " + e.getMessage());
        }
    }
    
    static class TemperatureHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            // Only accept GET requests
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method not allowed");
                return;
            }
            
            try {
                // Parse query parameters
                Map<String, String> params = parseQueryParams(exchange.getRequestURI().getQuery());
                
                // Validate and extract parameters - treating all input as untrusted
                Double latitude = validateDouble(params.get("latitude"), -90.0, 90.0, "latitude");
                Double longitude = validateDouble(params.get("longitude"), -180.0, 180.0, "longitude");
                Integer year = validateInteger(params.get("year"), 1900, 2100, "year");
                Integer month = validateInteger(params.get("month"), 1, 12, "month");
                Integer day = validateInteger(params.get("day"), 1, 31, "day");
                String gribFile = validateGribFile(params.get("grib_file"));
                
                // Query database with parameterized query to prevent SQL injection
                Double maxTemp = queryMaxTemperature(latitude, longitude, year, month, day, gribFile);
                
                if (maxTemp != null) {
                    String response = String.format("{\\"max_temperature\\": %.2f}", maxTemp);
                    sendResponse(exchange, 200, response);
                } else {
                    sendResponse(exchange, 404, "{\\"error\\": \\"No data found\\"}");
                }
                
            } catch (IllegalArgumentException e) {
                // Generic error message - no internal details leaked
                sendResponse(exchange, 400, "{\\"error\\": \\"Invalid request parameters\\"}");
            } catch (Exception e) {
                // Log detailed error internally, return generic message
                System.err.println("Internal error: " + e.getMessage());
                sendResponse(exchange, 500, "{\\"error\\": \\"Internal server error\\"}");
            }
        }
        
        private Map<String, String> parseQueryParams(String query) {
            Map<String, String> params = new HashMap<>();
            if (query == null || query.isEmpty()) {
                return params;
            }
            
            // Limit query string length to prevent DoS
            if (query.length() > 1000) {
                throw new IllegalArgumentException("Query string too long");
            }
            
            String[] pairs = query.split("&");
            // Limit number of parameters
            if (pairs.length > 10) {
                throw new IllegalArgumentException("Too many parameters");
            }
            
            for (String pair : pairs) {
                String[] keyValue = pair.split("=", 2);
                if (keyValue.length == 2) {
                    params.put(keyValue[0], keyValue[1]);
                }
            }
            return params;
        }
        
        private Double validateDouble(String value, double min, double max, String fieldName) {
            if (value == null || value.isEmpty()) {
                throw new IllegalArgumentException(fieldName + " is required");
            }
            
            try {
                double parsed = Double.parseDouble(value);
                // Range validation
                if (parsed < min || parsed > max) {
                    throw new IllegalArgumentException(fieldName + " out of range");
                }
                return parsed;
            } catch (NumberFormatException e) {
                throw new IllegalArgumentException(fieldName + " is invalid");
            }
        }
        
        private Integer validateInteger(String value, int min, int max, String fieldName) {
            if (value == null || value.isEmpty()) {
                throw new IllegalArgumentException(fieldName + " is required");
            }
            
            try {
                int parsed = Integer.parseInt(value);
                // Range validation
                if (parsed < min || parsed > max) {
                    throw new IllegalArgumentException(fieldName + " out of range");
                }
                return parsed;
            } catch (NumberFormatException e) {
                throw new IllegalArgumentException(fieldName + " is invalid");
            }
        }
        
        private String validateGribFile(String gribFile) {
            if (gribFile == null || gribFile.isEmpty()) {
                throw new IllegalArgumentException("grib_file is required");
            }
            
            // Length check
            if (gribFile.length() > 100) {
                throw new IllegalArgumentException("grib_file too long");
            }
            
            // Sanitize filename - only allow alphanumeric, underscore, dash, and dot
            // Prevent path traversal attacks
            if (!gribFile.matches("^[a-zA-Z0-9_\\\\-\\\\.]+$")) {
                throw new IllegalArgumentException("grib_file contains invalid characters");
            }
            
            // Block path traversal attempts
            if (gribFile.contains("..") || gribFile.contains("/") || gribFile.contains("\\\\")) {\n                throw new IllegalArgumentException("grib_file invalid");\n            }\n            \n            return gribFile;\n        }\n        \n        private Double queryMaxTemperature(double latitude, double longitude, int year, \n                                          int month, int day, String gribFile) throws SQLException {\n            // Use try-with-resources for automatic connection cleanup\n            try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD)) {\n                // Parameterized query prevents SQL injection\n                String sql = "SELECT MAX(temperature) as max_temp FROM temperature_data " +\n                           "WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? " +\n                           "AND day = ? AND grib_file = ? LIMIT 1";\n                \n                try (PreparedStatement stmt = conn.prepareStatement(sql)) {\n                    // Set parameters - type-safe binding prevents injection\n                    stmt.setDouble(1, latitude);\n                    stmt.setDouble(2, longitude);\n                    stmt.setInt(3, year);\n                    stmt.setInt(4, month);\n                    stmt.setInt(5, day);\n                    stmt.setString(6, gribFile);\n                    \n                    try (ResultSet rs = stmt.executeQuery()) {\n                        if (rs.next()) {\n                            double temp = rs.getDouble("max_temp");\n                            return rs.wasNull() ? null : temp;\n                        }\n                    }\n                }\n            }\n            return null;\n        }\n        \n        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {\n            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);\n            exchange.getResponseHeaders().set("Content-Type", "application/json; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }
}
