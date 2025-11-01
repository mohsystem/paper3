import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

/*
NOTE:
1. This code must be compiled and run with the MySQL JDBC driver in the classpath.
   Download MySQL Connector/J from the official MySQL website.
   Example compilation and execution:
   javac Lse86.java
   java -cp .:mysql-connector-java-8.0.28.jar Lse86

2. Before running, set up a MySQL database with the following table and credentials.
   - Database: weatherdb
   - User:     weather_user
   - Password: password
   - Table:
     CREATE TABLE weather_data (
         id INT AUTO_INCREMENT PRIMARY KEY,
         latitude DECIMAL(9, 6) NOT NULL,
         longitude DECIMAL(9, 6) NOT NULL,
         year INT NOT NULL,
         month INT NOT NULL,
         day INT NOT NULL,
         grib_file VARCHAR(255) NOT NULL,
         temperature DECIMAL(5, 2) NOT NULL
     );
     -- Add some sample data to the table for testing.
     INSERT INTO weather_data (latitude, longitude, year, month, day, grib_file, temperature) VALUES
     (34.05, -118.24, 2023, 7, 15, 'gfs.t00z.pgrb2.0p25.f000', 29.5),
     (34.05, -118.24, 2023, 7, 15, 'gfs.t00z.pgrb2.0p25.f000', 31.2),
     (40.71, -74.00, 2023, 8, 20, 'gfs.t06z.pgrb2.0p25.f001', 25.0),
     (48.85, 2.35, 2024, 1, 10, 'era5.12z.an.sfc', 5.5),
     (48.85, 2.35, 2024, 1, 10, 'era5.12z.an.sfc', 6.1);
*/
public class Lse86 {

    // --- DATABASE CONFIGURATION ---
    private static final String DB_URL = "jdbc:mysql://localhost:3306/weatherdb";
    private static final String DB_USER = "weather_user";
    private static final String DB_PASSWORD = "password";

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api/temperature_for_location", new TemperatureHandler());
        server.setExecutor(Executors.newFixedThreadPool(10)); // creates a default executor
        server.start();

        System.out.println("Server started on port " + port);
        System.out.println("--- 5 Test Cases (run in a separate terminal) ---");
        System.out.println("1. curl \"http://localhost:8080/api/temperature_for_location?latitude=34.05&longitude=-118.24&year=2023&month=7&day=15&grib_file=gfs.t00z.pgrb2.0p25.f000\"");
        System.out.println("2. curl \"http://localhost:8080/api/temperature_for_location?latitude=40.71&longitude=-74.00&year=2023&month=8&day=20&grib_file=gfs.t06z.pgrb2.0p25.f001\"");
        System.out.println("3. curl \"http://localhost:8080/api/temperature_for_location?latitude=48.85&longitude=2.35&year=2024&month=1&day=10&grib_file=era5.12z.an.sfc\"");
        System.out.println("4. curl \"http://localhost:8080/api/temperature_for_location?latitude=99.99&longitude=99.99&year=2025&month=1&day=1&grib_file=none\" # Should return not found");
        System.out.println("5. curl \"http://localhost:8080/api/temperature_for_location?latitude=40.71\" # Should return bad request");
    }

    static class TemperatureHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            String response;
            int statusCode = 200;

            try {
                if ("GET".equals(exchange.getRequestMethod())) {
                    URI requestedUri = exchange.getRequestURI();
                    Map<String, String> params = parseQuery(requestedUri.getQuery());

                    // Extract and validate parameters
                    if (params.containsKey("latitude") && params.containsKey("longitude") &&
                        params.containsKey("year") && params.containsKey("month") &&
                        params.containsKey("day") && params.containsKey("grib_file")) {
                        
                        double latitude = Double.parseDouble(params.get("latitude"));
                        double longitude = Double.parseDouble(params.get("longitude"));
                        int year = Integer.parseInt(params.get("year"));
                        int month = Integer.parseInt(params.get("month"));
                        int day = Integer.parseInt(params.get("day"));
                        String gribFile = params.get("grib_file");
                        
                        response = getMaxTemperature(latitude, longitude, year, month, day, gribFile);
                        if (response.contains("not found")) {
                             statusCode = 404;
                        }
                    } else {
                        statusCode = 400;
                        response = "{\"error\": \"Bad Request: Missing required parameters.\"}";
                    }
                } else {
                    statusCode = 405;
                    response = "{\"error\": \"Method Not Allowed\"}";
                }
            } catch (NumberFormatException e) {
                statusCode = 400;
                response = "{\"error\": \"Bad Request: Invalid number format for a parameter.\"}";
            } catch (Exception e) {
                statusCode = 500;
                response = "{\"error\": \"Internal Server Error: " + e.getMessage() + "\"}";
                e.printStackTrace();
            }
            
            exchange.getResponseHeaders().set("Content-Type", "application/json");
            exchange.sendResponseHeaders(statusCode, response.getBytes().length);
            OutputStream os = exchange.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }

    private static String getMaxTemperature(double lat, double lon, int year, int month, int day, String gribFile) {
        String sql = "SELECT MAX(temperature) AS max_temp FROM weather_data " +
                     "WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? AND day = ? AND grib_file = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            
            pstmt.setDouble(1, lat);
            pstmt.setDouble(2, lon);
            pstmt.setInt(3, year);
            pstmt.setInt(4, month);
            pstmt.setInt(5, day);
            pstmt.setString(6, gribFile);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    double maxTemp = rs.getDouble("max_temp");
                    if (rs.wasNull()) {
                        return "{\"message\": \"Temperature data not found for the given criteria.\"}";
                    }
                    return String.format("{\"max_temperature\": %.2f}", maxTemp);
                } else {
                    return "{\"message\": \"Temperature data not found for the given criteria.\"}";
                }
            }
        } catch (SQLException e) {
            e.printStackTrace();
            return "{\"error\": \"Database query failed: " + e.getMessage() + "\"}";
        }
    }

    private static Map<String, String> parseQuery(String query) {
        Map<String, String> params = new HashMap<>();
        if (query != null) {
            for (String param : query.split("&")) {
                String[] pair = param.split("=");
                if (pair.length > 1) {
                    params.put(pair[0], pair[1]);
                } else {
                    params.put(pair[0], "");
                }
            }
        }
        return params;
    }
}