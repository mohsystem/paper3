import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;

import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.URI;
import java.net.URLEncoder;
import java.net.URLDecoder;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.time.Duration;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Executors;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;

public class Lse86 {

    // Query function that accepts inputs as parameters and returns the max temperature (nullable)
    public static Double queryMaxTemperature(
            double latitude,
            double longitude,
            int year,
            int month,
            int day,
            String gribFile,
            String jdbcUrl,
            String dbUser,
            String dbPass
    ) throws SQLException {
        String sql = "SELECT MAX(temperature) AS max_temp " +
                     "FROM temperatures " +
                     "WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? AND day = ? AND grib_file = ?";

        try (Connection conn = DriverManager.getConnection(jdbcUrl, dbUser, dbPass);
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setDouble(1, latitude);
            ps.setDouble(2, longitude);
            ps.setInt(3, year);
            ps.setInt(4, month);
            ps.setInt(5, day);
            ps.setString(6, gribFile);

            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    double val = rs.getDouble("max_temp");
                    if (rs.wasNull()) return null;
                    return val;
                } else {
                    return null;
                }
            }
        }
    }

    // Simple query parser
    static Map<String, String> queryToMap(String query) {
        Map<String, String> map = new HashMap<>();
        if (query == null || query.isEmpty()) return map;
        for (String pair : query.split("&")) {
            int idx = pair.indexOf('=');
            if (idx < 0) {
                map.put(urlDecode(pair), "");
            } else {
                String k = urlDecode(pair.substring(0, idx));
                String v = urlDecode(pair.substring(idx + 1));
                map.put(k, v);
            }
        }
        return map;
    }

    static String urlDecode(String s) {
        return URLDecoder.decode(s, StandardCharsets.UTF_8);
    }

    static String jsonEscape(String s) {
        return s == null ? "null" : "\"" + s.replace("\\", "\\\\").replace("\"", "\\\"") + "\"";
    }

    public static void main(String[] args) throws Exception {
        // JDBC configuration from environment or defaults
        String host = System.getenv().getOrDefault("DB_HOST", "127.0.0.1");
        String port = System.getenv().getOrDefault("DB_PORT", "3306");
        String dbName = System.getenv().getOrDefault("DB_NAME", "weather");
        String dbUser = System.getenv().getOrDefault("DB_USER", "root");
        String dbPass = System.getenv().getOrDefault("DB_PASS", "");
        String jdbcUrl = System.getenv().getOrDefault("DB_URL",
                "jdbc:mysql://" + host + ":" + port + "/" + dbName + "?serverTimezone=UTC&useSSL=false");

        int serverPort = Integer.parseInt(System.getenv().getOrDefault("JAVA_PORT", "8080"));

        HttpServer server = HttpServer.create(new InetSocketAddress(serverPort), 0);
        server.createContext("/api/temperature_for_location", new HttpHandler() {
            @Override
            public void handle(HttpExchange exchange) throws IOException {
                if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                    String resp = "{\"error\":\"Method not allowed\"}";
                    exchange.sendResponseHeaders(405, resp.getBytes(StandardCharsets.UTF_8).length);
                    try (OutputStream os = exchange.getResponseBody()) { os.write(resp.getBytes(StandardCharsets.UTF_8)); }
                    return;
                }
                URI uri = exchange.getRequestURI();
                Map<String, String> params = queryToMap(uri.getRawQuery());

                String[] required = {"latitude","longitude","year","month","day","grib_file"};
                for (String r : required) {
                    if (!params.containsKey(r) || params.get(r).isEmpty()) {
                        String resp = "{\"error\":\"Missing parameter: " + r + "\"}";
                        exchange.getResponseHeaders().add("Content-Type", "application/json");
                        exchange.sendResponseHeaders(400, resp.getBytes(StandardCharsets.UTF_8).length);
                        try (OutputStream os = exchange.getResponseBody()) { os.write(resp.getBytes(StandardCharsets.UTF_8)); }
                        return;
                    }
                }

                try {
                    double lat = Double.parseDouble(params.get("latitude"));
                    double lon = Double.parseDouble(params.get("longitude"));
                    int year = Integer.parseInt(params.get("year"));
                    int month = Integer.parseInt(params.get("month"));
                    int day = Integer.parseInt(params.get("day"));
                    String grib = params.get("grib_file");

                    Double maxTemp = null;
                    String error = null;
                    try {
                        maxTemp = queryMaxTemperature(lat, lon, year, month, day, grib, jdbcUrl, dbUser, dbPass);
                    } catch (SQLException sqle) {
                        error = sqle.getMessage();
                    }

                    String body;
                    if (error != null) {
                        body = "{\"error\":"+jsonEscape(error)+"}";
                        exchange.getResponseHeaders().add("Content-Type", "application/json");
                        exchange.sendResponseHeaders(500, body.getBytes(StandardCharsets.UTF_8).length);
                    } else {
                        String value = (maxTemp == null) ? "null" : String.valueOf(maxTemp);
                        body = "{"
                                + "\"latitude\":" + lat + ","
                                + "\"longitude\":" + lon + ","
                                + "\"year\":" + year + ","
                                + "\"month\":" + month + ","
                                + "\"day\":" + day + ","
                                + "\"grib_file\":" + jsonEscape(grib) + ","
                                + "\"max_temperature\":" + value
                                + "}";
                        exchange.getResponseHeaders().add("Content-Type", "application/json");
                        exchange.sendResponseHeaders(200, body.getBytes(StandardCharsets.UTF_8).length);
                    }
                    try (OutputStream os = exchange.getResponseBody()) { os.write(body.getBytes(StandardCharsets.UTF_8)); }
                } catch (NumberFormatException nfe) {
                    String resp = "{\"error\":\"Invalid numeric parameter\"}";
                    exchange.getResponseHeaders().add("Content-Type", "application/json");
                    exchange.sendResponseHeaders(400, resp.getBytes(StandardCharsets.UTF_8).length);
                    try (OutputStream os = exchange.getResponseBody()) { os.write(resp.getBytes(StandardCharsets.UTF_8)); }
                }
            }
        });
        server.setExecutor(Executors.newFixedThreadPool(8));
        server.start();

        // 5 test cases using built-in HttpClient after the server starts
        HttpClient client = HttpClient.newBuilder().connectTimeout(Duration.ofSeconds(3)).build();
        String base = "http://127.0.0.1:" + serverPort + "/api/temperature_for_location";

        String[] tests = new String[] {
                base + "?latitude=40.7128&longitude=-74.0060&year=2024&month=1&day=15&grib_file=" + URLEncoder.encode("gfs_20240115.grb2", StandardCharsets.UTF_8),
                base + "?latitude=34.0522&longitude=-118.2437&year=2023&month=7&day=4&grib_file=" + URLEncoder.encode("era5_20230704.grib", StandardCharsets.UTF_8),
                base + "?latitude=51.5074&longitude=-0.1278&year=2022&month=12&day=25&grib_file=" + URLEncoder.encode("ukmet_20221225.grb", StandardCharsets.UTF_8),
                base + "?latitude=35.6895&longitude=139.6917&year=2021&month=8&day=1&grib_file=" + URLEncoder.encode("jma_20210801.grb2", StandardCharsets.UTF_8),
                base + "?latitude=-33.8688&longitude=151.2093&year=2020&month=2&day=29&grib_file=" + URLEncoder.encode("bom_20200229.grib", StandardCharsets.UTF_8)
        };

        for (String url : tests) {
            try {
                HttpRequest req = HttpRequest.newBuilder().uri(URI.create(url)).GET().timeout(Duration.ofSeconds(5)).build();
                HttpResponse<String> resp = client.send(req, HttpResponse.BodyHandlers.ofString());
                System.out.println("Java test response (" + url + "): " + resp.statusCode() + " " + resp.body());
            } catch (Exception e) {
                System.out.println("Java test error: " + e.getMessage());
            }
        }
    }
}