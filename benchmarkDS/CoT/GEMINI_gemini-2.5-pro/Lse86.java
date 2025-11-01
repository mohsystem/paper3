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
import java.util.concurrent.Executors;

/*
 * Dependencies:
 * This code requires the MySQL JDBC driver. You can add it to your project's classpath.
 * For example, if using Maven, add the following to pom.xml:
 * <dependency>
 *     <groupId>mysql</groupId>
 *     <artifactId>mysql-connector-java</artifactId>
 *     <version>8.0.33</version>
 * </dependency>
 *
 * Database Setup:
 * CREATE DATABASE weather_db;
 * USE weather_db;
 * CREATE TABLE weather_data (
 *     id INT AUTO_INCREMENT PRIMARY KEY,
 *     latitude DECIMAL(9, 6) NOT NULL,
 *     longitude DECIMAL(9, 6) NOT NULL,
 *     year INT NOT NULL,
 *     month INT NOT NULL,
 *     day INT NOT NULL,
 *     grib_file VARCHAR(255) NOT NULL,
 *     temperature DECIMAL(5, 2) NOT NULL,
 *     INDEX idx_location_date (latitude, longitude, year, month, day, grib_file)
 * );
 */
public class Lse86 {

    // --- Database Configuration ---
    // Best practice: Use a configuration file or environment variables instead of hardcoding.
    private static final String DB_URL = "jdbc:mysql://localhost:3306/weather_db";
    private static final String DB_USER = "your_user";
    private static final String DB_PASSWORD = "your_password";

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api/temperature_for_location", new TemperatureHandler());
        server.setExecutor(Executors.newCachedThreadPool()); // For handling multiple requests
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("API endpoint available at: http://localhost:8080/api/temperature_for_location");
        System.out.println("Press Ctrl+C to stop the server.");

        // --- Test Cases (run from command line using curl) ---
        System.out.println("\n--- How to Test ---");
        System.out.println("Use a tool like curl to send requests to the running server.");
        System.out.println("1. Valid request with data:");
        System.out.println("   curl \"http://localhost:8080/api/temperature_for_location?latitude=34.0522&longitude=-118.2437&year=2023&month=10&day=26&grib_file=gfs.t00z.pgrb2.0p25.f000\"");
        System.out.println("\n2. Request for data that doesn't exist:");
        System.out.println("   curl \"http://localhost:8080/api/temperature_for_location?latitude=0&longitude=0&year=1900&month=1&day=1&grib_file=none\"");
        System.out.println("\n3. Request with missing parameters:");
        System.out.println("   curl \"http://localhost:8080/api/temperature_for_location?latitude=34.0522&longitude=-118.2437\"");
        System.out.println("\n4. Request with invalid parameter type:");
        System.out.println("   curl \"http://localhost:8080/api/temperature_for_location?latitude=invalid&longitude=-118.2437&year=2023&month=10&day=26&grib_file=file.grib\"");
        System.out.println("\n5. Another valid request:");
        System.out.println("   curl \"http://localhost:8080/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=10&day=27&grib_file=gfs.t00z.pgrb2.0p25.f001\"");
    }

    static class TemperatureHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "{\"error\":\"Method Not Allowed\"}");
                return;
            }

            Map<String, String> params = queryToMap(exchange.getRequestURI().getQuery());

            try {
                // Parameter extraction and validation
                double latitude = Double.parseDouble(params.get("latitude"));
                double longitude = Double.parseDouble(params.get("longitude"));
                int year = Integer.parseInt(params.get("year"));
                int month = Integer.parseInt(params.get("month"));
                int day = Integer.parseInt(params.get("day"));
                String gribFile = params.get("grib_file");

                if (gribFile == null || gribFile.trim().isEmpty()) {
                     sendResponse(exchange, 400, "{\"error\":\"Missing grib_file parameter\"}");
                     return;
                }

                String maxTemp = findMaxTemperature(latitude, longitude, year, month, day, gribFile);
                if (maxTemp != null) {
                    sendResponse(exchange, 200, String.format("{\"max_temperature\":%s}", maxTemp));
                } else {
                    sendResponse(exchange, 404, "{\"message\":\"No temperature data found for the specified criteria.\"}");
                }

            } catch (NumberFormatException | NullPointerException e) {
                sendResponse(exchange, 400, "{\"error\":\"Invalid or missing required numerical parameters\"}");
            } catch (SQLException e) {
                // Log the exception for debugging but don't expose details to the client
                e.printStackTrace();
                sendResponse(exchange, 500, "{\"error\":\"Internal Server Error: Could not query database.\"}");
            }
        }

        private String findMaxTemperature(double latitude, double longitude, int year, int month, int day, String gribFile) throws SQLException {
            // Using a PreparedStatement to prevent SQL injection
            String sql = "SELECT MAX(temperature) AS max_temp FROM weather_data " +
                         "WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? AND day = ? AND grib_file = ?";
            
            // Try-with-resources ensures that resources are closed automatically
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
                        return rs.getString("max_temp"); // Can be null if no rows matched
                    }
                }
            }
            return null;
        }

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
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
                    result.put(entry[0], java.net.URLDecoder.decode(entry[1], java.nio.charset.StandardCharsets.UTF_8));
                } else {
                    result.put(entry[0], "");
                }
            }
            return result;
        }
    }
}