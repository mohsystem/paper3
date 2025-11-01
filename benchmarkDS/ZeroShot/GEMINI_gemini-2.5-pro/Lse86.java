import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

public class Lse86 {

    // --- Database Configuration ---
    // Note: In a production environment, use a secure way to manage credentials
    // (e.g., environment variables, configuration files, or a secrets manager).
    private static final String DB_URL = "jdbc:mysql://localhost:3306/weatherdb";
    private static final String DB_USER = "your_username";
    private static final String DB_PASSWORD = "your_password";

    // --- Security Configuration ---
    private static final Pattern GRIB_FILE_PATTERN = Pattern.compile("^[\\w.-]+$");

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api/temperature_for_location", new TemperatureHandler());
        server.setExecutor(Executors.newCachedThreadPool());
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Running test cases...");

        // Run test cases after server starts
        runTestCases(port);

        System.out.println("Test cases finished. Press Enter to stop the server.");
        System.in.read();
        server.stop(0);
        System.out.println("Server stopped.");
    }

    static class TemperatureHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "Method Not Allowed");
                return;
            }

            Map<String, String> params;
            try {
                params = parseQuery(exchange.getRequestURI().getRawQuery());
            } catch (IllegalArgumentException e) {
                sendResponse(exchange, 400, "Bad Request: " + e.getMessage());
                return;
            }

            try {
                // Parameter Extraction and Validation
                double latitude = Double.parseDouble(params.getOrDefault("latitude", ""));
                if (latitude < -90.0 || latitude > 90.0) {
                    throw new IllegalArgumentException("Invalid latitude value.");
                }

                double longitude = Double.parseDouble(params.getOrDefault("longitude", ""));
                if (longitude < -180.0 || longitude > 180.0) {
                    throw new IllegalArgumentException("Invalid longitude value.");
                }

                int year = Integer.parseInt(params.getOrDefault("year", ""));
                int month = Integer.parseInt(params.getOrDefault("month", ""));
                if (month < 1 || month > 12) {
                    throw new IllegalArgumentException("Invalid month value.");
                }
                
                int day = Integer.parseInt(params.getOrDefault("day", ""));
                if (day < 1 || day > 31) { // Basic validation
                     throw new IllegalArgumentException("Invalid day value.");
                }

                String gribFile = params.getOrDefault("grib_file", "");
                if (gribFile.isEmpty() || !GRIB_FILE_PATTERN.matcher(gribFile).matches()) {
                    throw new IllegalArgumentException("Invalid or missing grib_file.");
                }
                
                // Fetch data from database
                String result = getMaxTemperature(latitude, longitude, year, month, day, gribFile);
                sendResponse(exchange, 200, result);

            } catch (NumberFormatException e) {
                sendResponse(exchange, 400, "Bad Request: Invalid number format for a parameter.");
            } catch (IllegalArgumentException e) {
                sendResponse(exchange, 400, "Bad Request: " + e.getMessage());
            } catch (SQLException e) {
                // Log the exception internally for debugging
                System.err.println("Database error: " + e.getMessage());
                sendResponse(exchange, 500, "Internal Server Error");
            } catch (Exception e) {
                System.err.println("Unexpected error: " + e.getMessage());
                sendResponse(exchange, 500, "Internal Server Error");
            }
        }
    }

    public static String getMaxTemperature(double latitude, double longitude, int year, int month, int day, String gribFile) throws SQLException {
        String sql = "SELECT MAX(temperature) AS max_temp FROM temperature_data " +
                     "WHERE latitude = ? AND longitude = ? AND obs_year = ? AND obs_month = ? AND obs_day = ? AND grib_file = ?";
        
        // Use try-with-resources to ensure connection is closed
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            pstmt.setDouble(1, latitude);
            pstmt.setDouble(2, longitude);
            pstmt.setInt(3, year);
            pstmt.setInt(4, month);
            pstmt.setInt(5, day);
            pstmt.setString(6, gribFile);

            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    double maxTemp = rs.getDouble("max_temp");
                    if (rs.wasNull()) {
                        return "{\"status\": \"not_found\", \"message\": \"No temperature data found for the specified criteria.\"}";
                    }
                    return String.format("{\"status\": \"success\", \"max_temperature\": %.2f}", maxTemp);
                } else {
                    return "{\"status\": \"not_found\", \"message\": \"No temperature data found for the specified criteria.\"}";
                }
            }
        }
    }
    
    private static void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
        exchange.getResponseHeaders().set("Content-Type", "application/json");
        exchange.sendResponseHeaders(statusCode, response.getBytes(StandardCharsets.UTF_8).length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(response.getBytes(StandardCharsets.UTF_8));
        }
    }

    private static Map<String, String> parseQuery(String query) throws UnsupportedEncodingException {
        Map<String, String> params = new HashMap<>();
        if (query == null || query.isEmpty()) {
            throw new IllegalArgumentException("Query parameters are missing.");
        }
        for (String param : query.split("&")) {
            String[] pair = param.split("=", 2);
            if (pair.length > 1) {
                params.put(URLDecoder.decode(pair[0], StandardCharsets.UTF_8.name()),
                           URLDecoder.decode(pair[1], StandardCharsets.UTF_8.name()));
            } else {
                params.put(URLDecoder.decode(pair[0], StandardCharsets.UTF_8.name()), "");
            }
        }
        // Check for required params
        String[] required = {"latitude", "longitude", "year", "month", "day", "grib_file"};
        for(String req : required) {
            if(!params.containsKey(req)) {
                throw new IllegalArgumentException("Missing required parameter: " + req);
            }
        }
        return params;
    }
    
    private static void runTestCases(int port) {
        String[] testUrls = {
            // 1. Valid request (assuming this data exists)
            "http://localhost:" + port + "/api/temperature_for_location?latitude=34.0522&longitude=-118.2437&year=2023&month=7&day=15&grib_file=forecast.grib2",
            // 2. Valid request (assuming this data does NOT exist)
            "http://localhost:" + port + "/api/temperature_for_location?latitude=0&longitude=0&year=1900&month=1&day=1&grib_file=no_data.grib",
            // 3. Invalid latitude
            "http://localhost:" + port + "/api/temperature_for_location?latitude=100.0&longitude=-118.2437&year=2023&month=7&day=15&grib_file=forecast.grib2",
            // 4. Missing parameter (day)
            "http://localhost:" + port + "/api/temperature_for_location?latitude=34.0522&longitude=-118.2437&year=2023&month=7&grib_file=forecast.grib2",
            // 5. Invalid grib_file (path traversal attempt)
            "http://localhost:" + port + "/api/temperature_for_location?latitude=34.0522&longitude=-118.2437&year=2023&month=7&day=15&grib_file=../../etc/passwd"
        };

        HttpClient client = HttpClient.newHttpClient();
        for (int i = 0; i < testUrls.length; i++) {
            try {
                HttpRequest request = HttpRequest.newBuilder()
                        .uri(URI.create(testUrls[i]))
                        .build();
                HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());
                System.out.printf("Test Case %d URL: %s\n", i + 1, testUrls[i]);
                System.out.printf("--> Status: %d, Response: %s\n\n", response.statusCode(), response.body());
            } catch (IOException | InterruptedException e) {
                System.err.printf("Test Case %d failed: %s\n\n", i + 1, e.getMessage());
            }
        }
    }
}