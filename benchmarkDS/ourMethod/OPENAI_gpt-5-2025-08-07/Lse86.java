import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLDecoder;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.time.Duration;
import java.util.*;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

public class Lse86 {

    private static final int PORT = 8081;
    private static final String ENDPOINT = "/api/temperature_for_location";
    private static final Pattern GRIB_FILE_ALLOWED = Pattern.compile("^[A-Za-z0-9_.-]{1,64}$");

    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress("127.0.0.1", PORT), 0);
        server.createContext(ENDPOINT, new TemperatureHandler());
        server.setExecutor(Executors.newFixedThreadPool(4));
        server.start();

        // Small delay to ensure server is ready before tests
        Thread.sleep(400);

        // Five test cases using Java 11+ HttpClient
        List<String> testQueries = List.of(
                // 1) Valid parameters (DB likely not configured - expect error)
                "latitude=40.7128&longitude=-74.0060&year=2023&month=5&day=23&grib_file=nyc_20230523.grb2",
                // 2) Invalid latitude
                "latitude=123.45&longitude=-74.0060&year=2023&month=5&day=23&grib_file=nyc_20230523.grb2",
                // 3) Invalid grib_file characters
                "latitude=40.7128&longitude=-74.0060&year=2023&month=5&day=23&grib_file=../../etc/passwd",
                // 4) Missing parameter
                "latitude=40.7128&longitude=-74.0060&year=2023&month=5&grib_file=nyc_20230523.grb2",
                // 5) Boundary values
                "latitude=-90&longitude=180&year=1900&month=12&day=31&grib_file=edge_case.grb"
        );

        HttpClient client = HttpClient.newBuilder()
                .connectTimeout(Duration.ofSeconds(2))
                .build();

        for (String qs : testQueries) {
            HttpRequest req = HttpRequest.newBuilder()
                    .uri(new URI("http://127.0.0.1:" + PORT + ENDPOINT + "?" + qs))
                    .timeout(Duration.ofSeconds(5))
                    .GET()
                    .build();
            HttpResponse<String> resp = client.send(req, HttpResponse.BodyHandlers.ofString(StandardCharsets.UTF_8));
            System.out.println("Request: " + qs);
            System.out.println("Response: " + resp.body());
            System.out.println("---");
        }

        server.stop(0);
    }

    static final class TemperatureHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            try {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    sendJson(exchange, 405, jsonError("method_not_allowed"));
                    return;
                }

                URI uri = exchange.getRequestURI();
                Map<String, String> params = parseQuery(uri.getRawQuery());

                // Validate required params exist
                String[] required = {"latitude", "longitude", "year", "month", "day", "grib_file"};
                for (String r : required) {
                    if (!params.containsKey(r)) {
                        sendJson(exchange, 400, jsonError("missing_param"));
                        return;
                    }
                }

                // Validate and parse inputs
                Double lat = parseDoubleInRange(params.get("latitude"), -90.0, 90.0);
                Double lon = parseDoubleInRange(params.get("longitude"), -180.0, 180.0);
                Integer year = parseIntInRange(params.get("year"), 1900, 2100);
                Integer month = parseIntInRange(params.get("month"), 1, 12);
                Integer day = parseIntInRange(params.get("day"), 1, 31);
                String gribFile = params.get("grib_file");

                if (lat == null || lon == null || year == null || month == null || day == null) {
                    sendJson(exchange, 400, jsonError("invalid_param"));
                    return;
                }
                if (gribFile == null || gribFile.length() > 64 || !GRIB_FILE_ALLOWED.matcher(gribFile).matches()) {
                    sendJson(exchange, 400, jsonError("invalid_param"));
                    return;
                }

                DbQueryResult r = queryMaxTemperature(lat, lon, year, month, day, gribFile);
                if (!r.success) {
                    if ("database_not_configured".equals(r.errorCode)) {
                        sendJson(exchange, 500, jsonError("database_not_configured"));
                    } else if ("not_found".equals(r.errorCode)) {
                        sendJson(exchange, 404, "{\"max_temperature\": null}");
                    } else {
                        sendJson(exchange, 500, jsonError("internal_error"));
                    }
                    return;
                }

                String json = "{\"max_temperature\": " + r.maxTemperature + "}";
                sendJson(exchange, 200, json);

            } catch (Exception e) {
                sendJson(exchange, 500, jsonError("internal_error"));
            }
        }
    }

    static class DbQueryResult {
        final boolean success;
        final Double maxTemperature;
        final String errorCode;
        DbQueryResult(boolean success, Double maxTemperature, String errorCode) {
            this.success = success;
            this.maxTemperature = maxTemperature;
            this.errorCode = errorCode;
        }
    }

    // Core function: query DB for max temperature
    public static DbQueryResult queryMaxTemperature(double latitude, double longitude, int year, int month, int day, String gribFile) {
        String host = System.getenv("DB_HOST");
        String port = System.getenv("DB_PORT");
        String db = System.getenv("DB_NAME");
        String user = System.getenv("DB_USER");
        String pass = System.getenv("DB_PASSWORD");

        if (host == null || port == null || db == null || user == null || pass == null) {
            return new DbQueryResult(false, null, "database_not_configured");
        }

        String url = "jdbc:mysql://" + host + ":" + port + "/" + db + "?useSSL=true&requireSSL=false&allowPublicKeyRetrieval=true&serverTimezone=UTC";
        String sql = "SELECT MAX(temp_c) AS max_temp FROM temperatures " +
                     "WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? AND day = ? AND grib_file = ?";

        try {
            // Load driver if available (won't fail if using JDBC 4, but keep explicit for clarity)
            try {
                Class.forName("com.mysql.cj.jdbc.Driver");
            } catch (ClassNotFoundException ignore) {
                // If driver not present, fail gracefully
            }

            DriverManager.setLoginTimeout(5);
            try (Connection conn = DriverManager.getConnection(url, user, pass)) {
                conn.setReadOnly(true);
                try (PreparedStatement ps = conn.prepareStatement(sql)) {
                    ps.setDouble(1, latitude);
                    ps.setDouble(2, longitude);
                    ps.setInt(3, year);
                    ps.setInt(4, month);
                    ps.setInt(5, day);
                    ps.setString(6, gribFile);
                    try (ResultSet rs = ps.executeQuery()) {
                        if (rs.next()) {
                            Double v = rs.getObject("max_temp") != null ? rs.getDouble("max_temp") : null;
                            if (v == null) {
                                return new DbQueryResult(false, null, "not_found");
                            }
                            return new DbQueryResult(true, v, null);
                        } else {
                            return new DbQueryResult(false, null, "not_found");
                        }
                    }
                }
            }
        } catch (SQLException ex) {
            // Do not leak details
            return new DbQueryResult(false, null, "internal_error");
        }
    }

    // Helpers
    private static Map<String, String> parseQuery(String rawQuery) {
        Map<String, String> map = new HashMap<>();
        if (rawQuery == null || rawQuery.length() > 2048) return map; // cap length
        String[] pairs = rawQuery.split("&");
        for (String p : pairs) {
            int idx = p.indexOf('=');
            if (idx > 0) {
                String key = URLDecoder.decode(p.substring(0, idx), StandardCharsets.UTF_8);
                String val = URLDecoder.decode(p.substring(idx + 1), StandardCharsets.UTF_8);
                if (key.length() <= 64 && val.length() <= 256) {
                    map.put(key, val);
                }
            }
        }
        return map;
    }

    private static Double parseDoubleInRange(String s, double min, double max) {
        if (s == null || s.length() > 64) return null;
        try {
            double v = Double.parseDouble(s);
            if (Double.isFinite(v) && v >= min && v <= max) return v;
        } catch (NumberFormatException ignored) {}
        return null;
    }

    private static Integer parseIntInRange(String s, int min, int max) {
        if (s == null || s.length() > 16) return null;
        try {
            int v = Integer.parseInt(s);
            if (v >= min && v <= max) return v;
        } catch (NumberFormatException ignored) {}
        return null;
    }

    private static String jsonError(String code) {
        return "{\"error\":\"" + code + "\"}";
    }

    private static void sendJson(HttpExchange exchange, int status, String body) throws IOException {
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().set("Content-Type", "application/json; charset=utf-8");
        exchange.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = exchange.getResponseBody()) {
            os.write(bytes);
        }
    }
}