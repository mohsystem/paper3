import java.sql.*;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.time.format.ResolverStyle;
import java.util.Arrays;

public class Lse85 {

    private static final String DB_FILE = "Sensor.db";
    private static final String DB_URL = "jdbc:sqlite:" + DB_FILE;

    private static final DateTimeFormatter DATE_FMT = DateTimeFormatter.ISO_LOCAL_DATE.withResolverStyle(ResolverStyle.STRICT);

    // Converts degrees to microdegrees (rounded) to avoid floating point equality issues
    private static long toMicro(double deg) {
        return Math.round(deg * 1_000_000.0d);
    }

    private static boolean isValidLat(double lat) {
        return !Double.isNaN(lat) && lat >= -90.0d && lat <= 90.0d;
    }

    private static boolean isValidLon(double lon) {
        return !Double.isNaN(lon) && lon >= -180.0d && lon <= 180.0d;
    }

    private static boolean isValidDate(String date) {
        if (date == null || date.length() != 10) return false;
        // Quick pattern check YYYY-MM-DD
        for (int i = 0; i < 10; i++) {
            char c = date.charAt(i);
            if (i == 4 || i == 7) {
                if (c != '-') return false;
            } else {
                if (c < '0' || c > '9') return false;
            }
        }
        try {
            LocalDate parsed = LocalDate.parse(date, DATE_FMT);
            int year = parsed.getYear();
            return year >= 1900 && year <= 2100;
        } catch (Exception e) {
            return false;
        }
    }

    private static void initDatabase() {
        // Attempt to load SQLite JDBC driver if present; ignore if not found (modern JDBC may auto-load)
        try {
            Class.forName("org.sqlite.JDBC");
        } catch (ClassNotFoundException ignore) {
        }

        String createSql = "CREATE TABLE IF NOT EXISTS temperatures ("
                + "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                + "latitude_micro INTEGER NOT NULL,"
                + "longitude_micro INTEGER NOT NULL,"
                + "date TEXT NOT NULL,"
                + "temperature REAL NOT NULL,"
                + "UNIQUE(latitude_micro, longitude_micro, date)"
                + ");";

        try (Connection conn = DriverManager.getConnection(DB_URL)) {
            try (Statement st = conn.createStatement()) {
                st.execute(createSql);
            }

            // Seed sample data (idempotent)
            insertSample(conn, 37.7749, -122.4194, "2025-01-01", 15.2);
            insertSample(conn, 40.7128,  -74.0060, "2025-01-01", 5.4);
            insertSample(conn, 51.5074,   -0.1278, "2025-01-02", 7.1);
            insertSample(conn, -33.8688, 151.2093, "2025-01-01", 26.3);
            insertSample(conn, 35.6895,  139.6917, "2025-01-03", 8.9);
        } catch (SQLException e) {
            // Fail closed; do not leak details
            System.err.println("Database initialization error.");
        }
    }

    private static void insertSample(Connection conn, double lat, double lon, String date, double temp) throws SQLException {
        String ins = "INSERT OR IGNORE INTO temperatures(latitude_micro, longitude_micro, date, temperature) VALUES (?, ?, ?, ?)";
        try (PreparedStatement ps = conn.prepareStatement(ins)) {
            ps.setLong(1, toMicro(lat));
            ps.setLong(2, toMicro(lon));
            ps.setString(3, date);
            ps.setDouble(4, temp);
            ps.executeUpdate();
        }
    }

    // The API handler logic: returns temperature or NaN if not found/invalid
    public static double temperature_for_location(double latitude, double longitude, String date) {
        if (!isValidLat(latitude) || !isValidLon(longitude) || !isValidDate(date)) {
            return Double.NaN;
        }

        String sql = "SELECT temperature FROM temperatures WHERE latitude_micro=? AND longitude_micro=? AND date=? LIMIT 1";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setLong(1, toMicro(latitude));
            ps.setLong(2, toMicro(longitude));
            ps.setString(3, date);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    return rs.getDouble(1);
                } else {
                    return Double.NaN;
                }
            }
        } catch (SQLException e) {
            // Fail closed, do not expose details
            return Double.NaN;
        }
    }

    public static void main(String[] args) {
        initDatabase();

        // 5 test cases
        double[] lats = {37.7749, 40.7128, 51.5074, -33.8688, 91.0}; // last invalid
        double[] lons = {-122.4194, -74.0060, -0.1278, 151.2093, 0.0};
        String[] dates = {"2025-01-01", "2025-01-01", "2025-01-02", "2025-01-01", "2025-13-01"}; // last invalid

        for (int i = 0; i < 5; i++) {
            double t = temperature_for_location(lats[i], lons[i], dates[i]);
            String out = Double.isNaN(t) ? "not found or invalid" : String.format("%.2f", t);
            System.out.println("Test " + (i + 1) + ": temp=" + out);
        }

        // Additional demonstration: a non-existent date for a valid location
        double t2 = temperature_for_location(37.7749, -122.4194, "2025-01-03");
        System.out.println("Extra: temp=" + (Double.isNaN(t2) ? "not found or invalid" : String.format("%.2f", t2)));
    }
}