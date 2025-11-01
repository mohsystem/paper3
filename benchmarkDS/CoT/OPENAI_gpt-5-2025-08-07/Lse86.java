import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;

import java.net.InetSocketAddress;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLDecoder;

import java.nio.charset.StandardCharsets;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;

import java.time.Duration;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

public class Lse86 {

    private static final Pattern GRIB_FILE_PATTERN = Pattern.compile("^[A-Za-z0-9._-]{1,128}$");

    public static void main(String[] args) throws Exception {
        int port = Integer.parseInt(getEnv("PORT", "8080"));
        HttpServer server = HttpServer.create(new InetSocketAddress(port), 0);
        server.createContext("/api/temperature_for_location", new TemperatureHandler());
        server.setExecutor(null);
        server.start();

        // Simple delay to ensure server is up before tests
        Thread.sleep(500);

        // 5 test cases using HTTP requests (may return error if DB not configured)
        String base = "http://localhost:" + port + "/api/temperature_for_location";
        String[] testUrls = new String[] {
                base + "?latitude=40.7128&longitude=-74.0060&year=2023&month=7&day=15&grib_file=sample1.grb",
                base + "?latitude=34.0522&longitude=-118.2437&year=2022&month=12&day=1&grib_file=sample2.grb",
                base + "?latitude=-33.8688&longitude=151.2093&year=2021&month=1&day=31&grib_file=sample3.grb",
                base + "?latitude=51.5074&longitude=-0.1278&year=2020&month=6&day=21&grib_file=sample4.grb",
                base + "?latitude=35.6895&longitude=139.6917&year=2019&month=3&day=10&grib_file=sample5.grb"
        };
        for (String u : testUrls) {
            try {
                System.out.println(httpGet(u));
            } catch (Exception e) {
                System.out.println("{\"error\":\"" + e.getMessage().replace("\"", "'") + "\"}");
            }
        }
        // Keep server running; comment out next line to keep alive indefinitely
        // server.stop(0);
    }

    static class TemperatureHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws IOException {
            if (!"GET".equalsIgnoreCase(exchange.getRequestMethod())) {
                sendResponse(exchange, 405, "{\"error\":\"Method Not Allowed\"}");
                return;
            }
            String query = exchange.getRequestURI().getRawQuery();
            Map<String, String> params = parseQuery(query);

            try {
                double latitude = parseDoubleParam(params, "latitude", -90.0, 90.0);
                double longitude = parseDoubleParam(params, "longitude", -180.0, 180.0);
                int year = parseIntParam(params, "year", 1900, 2100);
                int month = parseIntParam(params, "month", 1, 12);
                int day = parseIntParam(params, "day", 1, 31);
                String gribFile = parseGribFile(params);

                String json = temperatureForLocationJSON(latitude, longitude, year, month, day, gribFile);
                sendResponse(exchange, 200, json);
            } catch (IllegalArgumentException e) {
                sendResponse(exchange, 400, "{\"error\":\"" + e.getMessage().replace("\"", "'") + "\"}");
            } catch (Exception e) {
                sendResponse(exchange, 500, "{\"error\":\"Internal Server Error\"}");
            }
        }
    }

    public static String temperatureForLocationJSON(double latitude, double longitude, int year, int month, int day, String gribFile) throws Exception {
        Double maxTemp = queryMaxTemperature(latitude, longitude, year, month, day, gribFile);
        if (maxTemp == null) {
            return "{\"max_temperature\":null}";
        }
        return "{\"max_temperature\":" + maxTemp + "}";
    }

    public static Double queryMaxTemperature(double latitude, double longitude, int year, int month, int day, String gribFile) throws Exception {
        if (!GRIB_FILE_PATTERN.matcher(gribFile).matches()) {
            throw new IllegalArgumentException("Invalid grib_file");
        }
        String dbUrl = getEnv("DB_URL", "jdbc:mysql://localhost:3306/weather?useSSL=true&requireSSL=false&allowPublicKeyRetrieval=true&serverTimezone=UTC");
        String dbUser = getEnv("DB_USER", "root");
        String dbPass = getEnv("DB_PASS", "");

        String sql = "SELECT MAX(temperature) AS max_temp FROM temperatures WHERE latitude=? AND longitude=? AND year=? AND month=? AND day=? AND grib_file=?";

        try (Connection conn = DriverManager.getConnection(dbUrl, dbUser, dbPass);
             PreparedStatement ps = conn.prepareStatement(sql)) {

            ps.setQueryTimeout((int) Duration.ofSeconds(10).toSeconds());
            ps.setDouble(1, latitude);
            ps.setDouble(2, longitude);
            ps.setInt(3, year);
            ps.setInt(4, month);
            ps.setInt(5, day);
            ps.setString(6, gribFile);

            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    double val = rs.getDouble(1);
                    if (rs.wasNull()) {
                        return null;
                    }
                    return val;
                }
                return null;
            }
        }
    }

    private static double parseDoubleParam(Map<String, String> params, String key, double min, double max) {
        String v = params.get(key);
        if (v == null) throw new IllegalArgumentException("Missing parameter: " + key);
        try {
            double d = Double.parseDouble(v);
            if (Double.isNaN(d) || Double.isInfinite(d)) throw new NumberFormatException();
            if (d < min || d > max) throw new IllegalArgumentException("Parameter out of range: " + key);
            return d;
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid number for: " + key);
        }
    }

    private static int parseIntParam(Map<String, String> params, String key, int min, int max) {
        String v = params.get(key);
        if (v == null) throw new IllegalArgumentException("Missing parameter: " + key);
        try {
            int i = Integer.parseInt(v);
            if (i < min || i > max) throw new IllegalArgumentException("Parameter out of range: " + key);
            return i;
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException("Invalid integer for: " + key);
        }
    }

    private static String parseGribFile(Map<String, String> params) {
        String v = params.get("grib_file");
        if (v == null) throw new IllegalArgumentException("Missing parameter: grib_file");
        if (!GRIB_FILE_PATTERN.matcher(v).matches()) {
            throw new IllegalArgumentException("Invalid grib_file");
        }
        return v;
    }

    private static Map<String, String> parseQuery(String rawQuery) throws IOException {
        Map<String, String> map = new HashMap<>();
        if (rawQuery == null || rawQuery.isEmpty()) return map;
        String[] pairs = rawQuery.split("&");
        for (String p : pairs) {
            int idx = p.indexOf('=');
            if (idx >= 0) {
                String k = URLDecoder.decode(p.substring(0, idx), StandardCharsets.UTF_8.name());
                String v = URLDecoder.decode(p.substring(idx + 1), StandardCharsets.UTF_8.name());
                map.put(k, v);
            } else {
                String k = URLDecoder.decode(p, StandardCharsets.UTF_8.name());
                map.put(k, "");
            }
        }
        return map;
    }

    private static String getEnv(String name, String defVal) {
        String v = System.getenv(name);
        return (v == null || v.isEmpty()) ? defVal : v;
    }

    private static void sendResponse(HttpExchange ex, int status, String body) throws IOException {
        byte[] bytes = body.getBytes(StandardCharsets.UTF_8);
        ex.getResponseHeaders().set("Content-Type", "application/json; charset=utf-8");
        ex.sendResponseHeaders(status, bytes.length);
        try (OutputStream os = ex.getResponseBody()) {
            os.write(bytes);
        }
    }

    private static String httpGet(String urlStr) throws IOException {
        URL url = new URL(urlStr);
        HttpURLConnection c = (HttpURLConnection) url.openConnection();
        c.setConnectTimeout(3000);
        c.setReadTimeout(5000);
        c.setRequestMethod("GET");
        int code = c.getResponseCode();
        try (BufferedReader br = new BufferedReader(new InputStreamReader(
                code >= 400 ? c.getErrorStream() : c.getInputStream(), StandardCharsets.UTF_8))) {
            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) sb.append(line);
            return sb.toString();
        }
    }
}