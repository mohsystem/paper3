
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;
import java.io.*;
import java.net.*;
import java.sql.*;
import java.util.*;

public class Lse86 {
    private static final String DB_URL = System.getenv("DB_URL");
    private static final String DB_USER = System.getenv("DB_USER");
    private static final String DB_PASSWORD = System.getenv("DB_PASSWORD");
    
    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/api/temperature_for_location", new TemperatureHandler());
        server.setExecutor(null);
        server.start();
        System.out.println("Server started on port 8080");
        
        // Test cases
        testTemperatureQuery();
    }
    
    static class TemperatureHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }
            
            try {
                Map<String, String> params = parseQueryParams(exchange.getRequestURI().getQuery());
                
                Double latitude = validateDouble(params.get("latitude"), -90, 90);
                Double longitude = validateDouble(params.get("longitude"), -180, 180);
                Integer year = validateInteger(params.get("year"), 1900, 2100);
                Integer month = validateInteger(params.get("month"), 1, 12);
                Integer day = validateInteger(params.get("day"), 1, 31);
                String gribFile = validateString(params.get("grib_file"), 100);
                
                if (latitude == null || longitude == null || year == null || 
                    month == null || day == null || gribFile == null) {
                    sendResponse(exchange, 400, "Invalid parameters");
                    return;
                }
                
                Double maxTemp = getMaxTemperature(latitude, longitude, year, month, day, gribFile);
                
                if (maxTemp != null) {
                    sendResponse(exchange, 200, "{\\"max_temperature\\": " + maxTemp + "}");
                } else {
                    sendResponse(exchange, 404, "No data found");
                }
                
            } catch (Exception e) {
                sendResponse(exchange, 500, "Internal Server Error");
            }
        }
    }
    
    public static Double getMaxTemperature(double latitude, double longitude, 
                                          int year, int month, int day, String gribFile) {
        String query = "SELECT MAX(temperature) as max_temp FROM temperature_data " +
                      "WHERE latitude = ? AND longitude = ? AND year = ? " +
                      "AND month = ? AND day = ? AND grib_file = ?";
        
        try (Connection conn = getConnection();
             PreparedStatement stmt = conn.prepareStatement(query)) {
            
            stmt.setDouble(1, latitude);
            stmt.setDouble(2, longitude);
            stmt.setInt(3, year);
            stmt.setInt(4, month);
            stmt.setInt(5, day);
            stmt.setString(6, gribFile);
            
            try (ResultSet rs = stmt.executeQuery()) {
                if (rs.next()) {
                    return rs.getDouble("max_temp");
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return null;
    }
    
    private static Connection getConnection() throws SQLException {
        return DriverManager.getConnection(
            DB_URL != null ? DB_URL : "jdbc:mysql://localhost:3306/weather_db",
            DB_USER != null ? DB_USER : "root",
            DB_PASSWORD != null ? DB_PASSWORD : "password"
        );
    }
    
    private static Map<String, String> parseQueryParams(String query) {
        Map<String, String> params = new HashMap<>();
        if (query != null) {
            for (String param : query.split("&")) {
                String[] pair = param.split("=");
                if (pair.length == 2) {
                    try {
                        params.put(URLDecoder.decode(pair[0], "UTF-8"), 
                                 URLDecoder.decode(pair[1], "UTF-8"));
                    } catch (UnsupportedEncodingException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
        return params;
    }
    
    private static Double validateDouble(String value, double min, double max) {
        if (value == null) return null;
        try {
            double d = Double.parseDouble(value);
            return (d >= min && d <= max) ? d : null;
        } catch (NumberFormatException e) {
            return null;
        }
    }
    
    private static Integer validateInteger(String value, int min, int max) {
        if (value == null) return null;
        try {
            int i = Integer.parseInt(value);
            return (i >= min && i <= max) ? i : null;
        } catch (NumberFormatException e) {
            return null;
        }
    }
    
    private static String validateString(String value, int maxLength) {
        if (value == null || value.length() > maxLength) return null;
        return value.replaceAll("[^a-zA-Z0-9._-]", "");
    }
    
    private static void sendResponse(HttpExchange exchange, int statusCode, String response) 
            throws IOException {
        exchange.sendResponseHeaders(statusCode, response.getBytes().length);
        OutputStream os = exchange.getResponseBody();
        os.write(response.getBytes());
        os.close();
    }
    
    private static void testTemperatureQuery() {
        System.out.println("\\n=== Test Cases ===");
        
        // Test case 1
        Double result1 = getMaxTemperature(40.7128, -74.0060, 2023, 6, 15, "test.grb");
        System.out.println("Test 1 - NYC coordinates: " + result1);
        
        // Test case 2
        Double result2 = getMaxTemperature(51.5074, -0.1278, 2023, 7, 20, "test.grb");
        System.out.println("Test 2 - London coordinates: " + result2);
        
        // Test case 3
        Double result3 = getMaxTemperature(35.6762, 139.6503, 2023, 8, 10, "test.grb");
        System.out.println("Test 3 - Tokyo coordinates: " + result3);
        
        // Test case 4
        Double result4 = getMaxTemperature(-33.8688, 151.2093, 2023, 12, 25, "test.grb");
        System.out.println("Test 4 - Sydney coordinates: " + result4);
        
        // Test case 5
        Double result5 = getMaxTemperature(48.8566, 2.3522, 2023, 5, 1, "test.grb");
        System.out.println("Test 5 - Paris coordinates: " + result5);
    }
}
