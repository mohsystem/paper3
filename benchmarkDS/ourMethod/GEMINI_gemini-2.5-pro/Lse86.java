import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.time.DateTimeException;
import java.time.LocalDate;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

public class Lse86 {

    // Regex to validate grib_file format. Allows alphanumeric, underscore, hyphen, and period.
    private static final Pattern GRIB_FILE_PATTERN = Pattern.compile("^[a-zA-Z0-9_.-]+$");
    private static final int MAX_GRIB_FILE_LENGTH = 255;

    public static void main(String[] args) throws IOException {
        int port = 8080;
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api/temperature_for_location", new TemperatureHandler());
        server.setExecutor(Executors.newFixedThreadPool(10)); // Create a thread pool
        server.start();
        System.out.println("Server started on port " + port);
        System.out.println("Use 'curl' to test the endpoint. Examples:");
        System.out.println("1. Success: curl \"http://localhost:8080/api/temperature_for_location?latitude=34.0522&longitude=-118.2437&year=2023&month=10&day=26&grib_file=gfs.t00z.pgrb2.0p25.f000\"");
        System.out.println("2. Invalid Latitude: curl \"http://localhost:8080/api/temperature_for_location?latitude=999&longitude=-118.2437&year=2023&month=10&day=26&grib_file=file.grib\"");
        System.out.println("3. Invalid Date: curl \"http://localhost:8080/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=13&day=27&grib_file=file.grib\"");
        System.out.println("4. Invalid Grib File: curl \"http://localhost:8080/api/temperature_for_location?latitude=51.5074&longitude=-0.1278&year=2024&month=1&day=1&grib_file=../etc/passwd\"");
        System.out.println("5. No Data Found: curl \"http://localhost:8080/api/temperature_for_location?latitude=1.0&longitude=1.0&year=2025&month=1&day=1&grib_file=no_data.grb\"");

        /*
         * Database setup commands:
         *
         * CREATE DATABASE IF NOT EXISTS weather_db;
         * USE weather_db;
         * CREATE TABLE IF NOT EXISTS weather_data (
         *     id INT AUTO_INCREMENT PRIMARY KEY,
         *     latitude DECIMAL(9, 6) NOT NULL,
         *     longitude DECIMAL(9, 6) NOT NULL,
         *     year INT NOT NULL,
         *     month INT NOT NULL,
         *     day INT NOT NULL,
         *     grib_file VARCHAR(255) NOT NULL,
         *     temperature DECIMAL(5, 2) NOT NULL,
         *     UNIQUE KEY unique_record (latitude, longitude, year, month, day, grib_file)
         * );
         *
         * INSERT INTO weather_data (latitude, longitude, year, month, day, grib_file, temperature) VALUES
         * (34.0522, -118.2437, 2023, 10, 26, 'gfs.t00z.pgrb2.0p25.f000', 22.5),
         * (34.0522, -118.2437, 2023, 10, 26, 'gfs.t00z.pgrb2.0p25.f000', 23.1);
         *
         * To compile and run:
         * 1. Download MySQL Connector/J JAR file.
         * 2. Compile: javac -cp mysql-connector-j-8.x.x.jar Lse86.java
         * 3. Run: java -cp .:mysql-connector-j-8.x.x.jar Lse86
         *
         * Set environment variables for database connection:
         * export DB_HOST="localhost"
         * export DB_PORT="3306"
         * export DB_NAME="weather_db"
         * export DB_USER="your_user"
         * export DB_PASS="your_password"
         */
    }

    static class TemperatureHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equals(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "{\"error\":\"Method Not Allowed\"}");
                return;
            }

            Map<String, String> params = parseQuery(exchange.getRequestURI().getQuery());
            
            try {
                // Input validation
                double latitude = validateDouble(params.get("latitude"), -90.0, 90.0, "latitude");
                double longitude = validateDouble(params.get("longitude"), -180.0, 180.0, "longitude");
                int year = validateInt(params.get("year"), 1900, 2100, "year");
                int month = validateInt(params.get("month"), 1, 12, "month");
                int day = validateInt(params.get("day"), 1, 31, "day");
                validateDate(year, month, day); // Further date validation
                String gribFile = validateGribFile(params.get("grib_file"));

                // Fetch data from database
                Optional<Double> maxTemp = getMaxTemperature(latitude, longitude, year, month, day, gribFile);

                if (maxTemp.isPresent()) {
                    sendResponse(exchange, 200, String.format("{\"max_temperature\": %.2f}", maxTemp.get()));
                } else {
                    sendResponse(exchange, 404, "{\"message\":\"No temperature data found for the given parameters.\"}");
                }

            } catch (IllegalArgumentException e) {
                sendResponse(exchange, 400, "{\"error\":\"" + e.getMessage() + "\"}");
            } catch (SQLException e) {
                // Log the exception securely without exposing details to the client
                System.err.println("Database error: " + e.getMessage());
                sendResponse(exchange, 500, "{\"error\":\"An internal server error occurred.\"}");
            } catch (Exception e) {
                System.err.println("Unexpected error: " + e.getMessage());
                sendResponse(exchange, 500, "{\"error\":\"An internal server error occurred.\"}");
            }
        }

        private Optional<Double> getMaxTemperature(double lat, double lon, int year, int month, int day, String gribFile) throws SQLException {
            String dbHost = Optional.ofNullable(System.getenv("DB_HOST")).orElse("localhost");
            String dbPort = Optional.ofNullable(System.getenv("DB_PORT")).orElse("3306");
            String dbName = Optional.ofNullable(System.getenv("DB_NAME")).orElse("weather_db");
            String dbUser = System.getenv("DB_USER");
            String dbPass = System.getenv("DB_PASS");

            if (dbUser == null || dbPass == null) {
                throw new IllegalStateException("DB_USER and DB_PASS environment variables must be set.");
            }
            
            String url = String.format("jdbc:mysql://%s:%s/%s?useSSL=true&requireSSL=true",
                                       dbHost, dbPort, dbName);

            String sql = "SELECT MAX(temperature) FROM weather_data WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? AND day = ? AND grib_file = ?";
            
            try (Connection conn = DriverManager.getConnection(url, dbUser, dbPass);
                 PreparedStatement pstmt = conn.prepareStatement(sql)) {

                pstmt.setDouble(1, lat);
                pstmt.setDouble(2, lon);
                pstmt.setInt(3, year);
                pstmt.setInt(4, month);
                pstmt.setInt(5, day);
                pstmt.setString(6, gribFile);

                try (ResultSet rs = pstmt.executeQuery()) {
                    if (rs.next()) {
                        double maxTemp = rs.getDouble(1);
                        if (rs.wasNull()) {
                            return Optional.empty();
                        }
                        return Optional.of(maxTemp);
                    }
                }
            }
            return Optional.empty();
        }

        private double validateDouble(String val, double min, double max, String paramName) {
            if (val == null || val.trim().isEmpty()) {
                throw new IllegalArgumentException("Missing required parameter: " + paramName);
            }
            try {
                double result = Double.parseDouble(val);
                if (result < min || result > max) {
                    throw new IllegalArgumentException(String.format("Parameter %s out of range.", paramName));
                }
                return result;
            } catch (NumberFormatException e) {
                throw new IllegalArgumentException("Invalid number format for parameter: " + paramName);
            }
        }

        private int validateInt(String val, int min, int max, String paramName) {
             if (val == null || val.trim().isEmpty()) {
                throw new IllegalArgumentException("Missing required parameter: " + paramName);
            }
            try {
                int result = Integer.parseInt(val);
                if (result < min || result > max) {
                    throw new IllegalArgumentException(String.format("Parameter %s out of range.", paramName));
                }
                return result;
            } catch (NumberFormatException e) {
                throw new IllegalArgumentException("Invalid number format for parameter: " + paramName);
            }
        }
        
        private void validateDate(int year, int month, int day) {
            try {
                LocalDate.of(year, month, day);
            } catch (DateTimeException e) {
                throw new IllegalArgumentException("Invalid date: " + e.getMessage());
            }
        }
        
        private String validateGribFile(String gribFile) {
            if (gribFile == null || gribFile.trim().isEmpty()) {
                 throw new IllegalArgumentException("Missing grib_file parameter.");
            }
            if (gribFile.length() > MAX_GRIB_FILE_LENGTH) {
                throw new IllegalArgumentException("grib_file exceeds maximum length.");
            }
            if (!GRIB_FILE_PATTERN.matcher(gribFile).matches()) {
                throw new IllegalArgumentException("Invalid grib_file format.");
            }
            return gribFile;
        }

        private Map<String, String> parseQuery(String query) {
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

        private void sendResponse(HttpExchange exchange, int statusCode, String response) throws IOException {
            byte[] responseBytes = response.getBytes(StandardCharsets.UTF_8);
            exchange.getResponseHeaders().set("Content-Type", "application/json; charset=UTF-8");
            exchange.sendResponseHeaders(statusCode, responseBytes.length);
            try (OutputStream os = exchange.getResponseBody()) {
                os.write(responseBytes);
            }
        }
    }
}