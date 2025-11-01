import com.sun.net.httpserver.Headers;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.math.BigDecimal;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.time.LocalDate;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

public class Lse86 {

    private static final String TABLE_NAME = "temperature_data";
    private static final Pattern SAFE_FILE_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,128}$");

    // Function: Accepts input as parameters and returns the maximum temperature, or null if none
    public static Double queryMaxTemperature(double latitude, double longitude, int year, int month, int day, String gribFile) throws SQLException {
        validateInputs(latitude, longitude, year, month, day, gribFile);

        String url = System.getenv("DB_URL");
        String user = System.getenv("DB_USER");
        String pass = System.getenv("DB_PASS");

        if (url == null || user == null || pass == null) {
            throw new SQLException("Database configuration not set via environment variables.");
        }

        // Using BigDecimal to avoid floating precision pitfalls from DB, then return as Double
        String sql = "SELECT MAX(temperature) AS max_temp FROM " + TABLE_NAME +
                " WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? AND day = ? AND grib_file = ?";

        try (Connection conn = DriverManager.getConnection(url, user, pass);
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setBigDecimal(1, BigDecimal.valueOf(latitude));
            ps.setBigDecimal(2, BigDecimal.valueOf(longitude));
            ps.setInt(3, year);
            ps.setInt(4, month);
            ps.setInt(5, day);
            ps.setString(6, gribFile);

            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    BigDecimal val = rs.getBigDecimal("max_temp");
                    return (val != null) ? val.doubleValue() : null;
                }
                return null;
            }
        }
    }

    // Function: Accepts parameter map (as would be parsed from HTTP query) and returns JSON String
    public static String handleTemperatureQuery(Map<String, String> params) {
        try {
            double latitude = parseDoubleParam(params, "latitude", -90.0, 90.0);
            double longitude = parseDoubleParam(params, "longitude", -180.0, 180.0);
            int year = parseIntParam(params, "year", 1900, 2100);
            int month = parseIntParam(params, "month", 1, 12);
            int day = parseIntParam(params, "day", 1, 31);
            // Validate calendar day roughly
            if (!isPlausibleDate(year, month, day)) {
                return jsonError(400, "Invalid date components.");
            }
            String gribFile = parseStringParam(params, "grib_file");

            Double result = queryMaxTemperature(latitude, longitude, year, month, day, gribFile);
            if (result == null) {
                return "{"
                        + "\"status\":404,"
                        + "\"message\":\"No temperature found for given parameters.\","
                        + "\"latitude\":" + latitude + ","
                        + "\"longitude\":" + longitude + ","
                        + "\"year\":" + year + ","
                        + "\"month\":" + month + ","
                        + "\"day\":" + day + ","
                        + "\"grib_file\":\"" + jsonEscape(gribFile) + "\""
                        + "}";
            } else {
                return "{"
                        + "\"status\":200,"
                        + "\"latitude\":" + latitude + ","
                        + "\"longitude\":" + longitude + ","
                        + "\"year\":" + year + ","
                        + "\"month\":" + month + ","
                        + "\"day\":" + day + ","
                        + "\"grib_file\":\"" + jsonEscape(gribFile) + "\","
                        + "\"max_temperature\":" + result + ","
                        + "\"unit\":\"K\""
                        + "}";
            }
        } catch (IllegalArgumentException iae) {
            return jsonError(400, iae.getMessage());
        } catch (SQLException sqle) {
            return jsonError(500, "Database error.");
        } catch (Exception e) {
            return jsonError(500, "Server error.");
        }
    }

    // Start a minimal HTTP server for /api/temperature_for_location
    public static void startServer(int port) throws IOException {
        HttpServer server = HttpServer.create(new InetSocketAddress("0.0.0.0", port), 0);
        server.createContext("/api/temperature_for_location", new TemperatureHandler());
        server.setExecutor(Executors.newFixedThreadPool(Math.max(2, Runtime.getRuntime().availableProcessors())));
        server.start();
        System.out.println("Server started on port " + port);
    }

    private static class TemperatureHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, jsonError(405, "Method not allowed"));
                return;
            }
            URI uri = exchange.getRequestURI();
            String rawQuery = uri.getRawQuery();
            Map<String, String> params = parseQuery(rawQuery);

            String response = handleTemperatureQuery(params);
            int status = parseStatusFromJson(response);

            sendResponse(exchange, status, response);
        }
    }

    private static Map<String, String> parseQuery(String rawQuery) {
        Map<String, String> map = new HashMap<>();
        if (rawQuery == null || rawQuery.length() > 2048) {
            return map;
        }
        String[] parts = rawQuery.split("&");
        for (String part : parts) {
            if (part.isEmpty()) continue;
            String[] kv = part.split("=", 2);
            String key = urlDecode(kv[0]);
            String val = kv.length > 1 ? urlDecode(kv[1]) : "";
            if (key != null && key.length() <= 64) {
                map.put(key, val);
            }
        }
        return map;
    }

    private static String urlDecode(String s) {
        try {
            return URLDecoder.decode(s, StandardCharsets.UTF_8.name());
        } catch (Exception e) {
            return null;
        }
    }

    private static void sendResponse(HttpExchange exchange, int status, String body) throws IOException {
        Headers headers = exchange.getResponseHeaders();
        headers.set("Content-Type", "application/json; charset=utf-8");
        headers.set("X-Content-Type-Options", "nosniff");
        headers.set("Cache-Control", "no-store");
        byte[] data = body.getBytes(StandardCharsets.UTF_8);
        exchange.sendResponseHeaders(status, data.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(data);
        }
    }

    private static int parseStatusFromJson(String json) {
        try {
            int idx = json.indexOf("\"status\":");
            if (idx >= 0) {
                int start = idx + 9;
                int end = start;
                while (end < json.length() && Character.isDigit(json.charAt(end))) end++;
                return Integer.parseInt(json.substring(start, end));
            }
        } catch (Exception ignored) {
        }
        return 200;
    }

    private static String jsonError(int status, String message) {
        return "{"
                + "\"status\":" + status + ","
                + "\"message\":\"" + jsonEscape(message) + "\""
                + "}";
    }

    private static String jsonEscape(String s) {
        if (s == null) return "";
        StringBuilder sb = new StringBuilder(s.length() + 16);
        for (char c : s.toCharArray()) {
            switch (c) {
                case '\\': sb.append("\\\\"); break;
                case '"': sb.append("\\\""); break;
                case '\b': sb.append("\\b"); break;
                case '\f': sb.append("\\f"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) {
                        sb.append(String.format("\\u%04x", (int) c));
                    } else {
                        sb.append(c);
                    }
            }
        }
        return sb.toString();
    }

    private static double parseDoubleParam(Map<String, String> params, String name, double min, double max) {
        String v = params.get(name);
        if (v == null) {
            throw new IllegalArgumentException("Missing parameter: " + name);
        }
        try {
            double d = Double.parseDouble(v);
            if (Double.isNaN(d) || Double.isInfinite(d) || d < min || d > max) {
                throw new IllegalArgumentException("Invalid value for " + name);
            }
            return d;
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid numeric value for " + name);
        }
    }

    private static int parseIntParam(Map<String, String> params, String name, int min, int max) {
        String v = params.get(name);
        if (v == null) {
            throw new IllegalArgumentException("Missing parameter: " + name);
        }
        try {
            int i = Integer.parseInt(v);
            if (i < min || i > max) {
                throw new IllegalArgumentException("Invalid value for " + name);
            }
            return i;
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid numeric value for " + name);
        }
    }

    private static String parseStringParam(Map<String, String> params, String name) {
        String v = params.get(name);
        if (v == null) {
            throw new IllegalArgumentException("Missing parameter: " + name);
        }
        if (!SAFE_FILE_PATTERN.matcher(v).matches()) {
            throw new IllegalArgumentException("Invalid grib_file format.");
        }
        return v;
    }

    private static boolean isPlausibleDate(int year, int month, int day) {
        try {
            LocalDate.of(year, month, day);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    private static void validateInputs(double latitude, double longitude, int year, int month, int day, String gribFile) {
        if (Double.isNaN(latitude) || latitude < -90 || latitude > 90) throw new IllegalArgumentException("Invalid latitude");
        if (Double.isNaN(longitude) || longitude < -180 || longitude > 180) throw new IllegalArgumentException("Invalid longitude");
        if (year < 1900 || year > 2100) throw new IllegalArgumentException("Invalid year");
        if (month < 1 || month > 12) throw new IllegalArgumentException("Invalid month");
        if (day < 1 || day > 31) throw new IllegalArgumentException("Invalid day");
        if (!isPlausibleDate(year, month, day)) throw new IllegalArgumentException("Invalid date");
        if (gribFile == null || !SAFE_FILE_PATTERN.matcher(gribFile).matches()) throw new IllegalArgumentException("Invalid grib_file");
    }

    public static void main(String[] args) throws Exception {
        // Start server on port 8080
        // To run tests without starting the server, set RUN_SERVER env var to "0"
        String runServer = System.getenv("RUN_SERVER");
        if (runServer == null || !"0".equals(runServer)) {
            new Thread(() -> {
                try {
                    startServer(8080);
                } catch (IOException e) {
                    System.err.println("Failed to start server.");
                }
            }).start();
        }

        // 5 test cases calling the function directly (will require DB to be configured)
        List<Object[]> tests = Arrays.asList(
                new Object[]{37.7749, -122.4194, 2023, 7, 15, "gfs_20230715.grb2"},
                new Object[]{40.7128, -74.0060, 2022, 12, 25, "era5_20221225.grib"},
                new Object[]{51.5074, -0.1278, 2021, 1, 1, "icon_20210101.grb"},
                new Object[]{-33.8688, 151.2093, 2020, 2, 29, "gfs_20200229.grb2"},
                new Object[]{35.6895, 139.6917, 2019, 9, 10, "jra55_20190910.grb"}
        );

        for (Object[] t : tests) {
            double lat = (double) t[0];
            double lon = (double) t[1];
            int year = (int) t[2];
            int month = (int) t[3];
            int day = (int) t[4];
            String grib = (String) t[5];
            try {
                Double res = queryMaxTemperature(lat, lon, year, month, day, grib);
                System.out.println("Test result: " + (res == null ? "null" : res));
            } catch (Exception e) {
                System.out.println("Test error: " + e.getMessage());
            }
        }
    }
}