import java.sql.*;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.time.format.ResolverStyle;
import java.util.Objects;
import java.nio.charset.StandardCharsets;

public class Lse87 {

    // Validate latitude and longitude ranges and date format YYYY-MM-DD with calendar validation
    private static boolean isValidLatLon(double lat, double lon) {
        return !Double.isNaN(lat) && !Double.isNaN(lon) && lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0;
    }

    private static boolean isValidDate(String date) {
        if (date == null) return false;
        if (date.length() != 10) return false;
        // quick structural check
        for (int i = 0; i < 10; i++) {
            char c = date.charAt(i);
            if (i == 4 || i == 7) {
                if (c != '-') return false;
            } else {
                if (c < '0' || c > '9') return false;
            }
        }
        try {
            DateTimeFormatter fmt = DateTimeFormatter.ofPattern("uuuu-MM-dd").withResolverStyle(ResolverStyle.STRICT);
            LocalDate.parse(date, fmt);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    // Returns the temperature for (lat, lon, date) or Double.NaN on error/not found.
    // This function is intended to be invoked upon a request to /api/temperature_for_location
    public static double temperature_for_location(double latitude, double longitude, String date) {
        if (!isValidLatLon(latitude, longitude) || !isValidDate(date)) {
            return Double.NaN;
        }

        // Connect to SQLite securely via JDBC and run a parameterized query
        String url = "jdbc:sqlite:weather_secure_java.db";
        String sql = "SELECT temp FROM temperatures WHERE lat = ? AND lon = ? AND date = ? LIMIT 1";

        try {
            // Load driver if present (modern JDBC may auto-load; this is safe)
            try {
                Class.forName("org.sqlite.JDBC");
            } catch (ClassNotFoundException ignore) {
                // If driver not present, SQLite JDBC may still work via Service Provider, otherwise connection will fail
            }

            try (Connection conn = DriverManager.getConnection(url);
                 PreparedStatement ps = conn.prepareStatement(sql)) {

                ps.setDouble(1, latitude);
                ps.setDouble(2, longitude);
                ps.setString(3, date);

                try (ResultSet rs = ps.executeQuery()) {
                    if (rs.next()) {
                        return rs.getDouble(1);
                    } else {
                        return Double.NaN;
                    }
                }
            }
        } catch (SQLException e) {
            // Minimal error signaling, do not leak details
            return Double.NaN;
        }
    }

    private static void safeExecUpdate(Connection conn, String sql) throws SQLException {
        try (Statement st = conn.createStatement()) {
            st.executeUpdate(sql);
        }
    }

    private static void seedDatabase() {
        String url = "jdbc:sqlite:weather_secure_java.db";
        try (Connection conn = DriverManager.getConnection(url)) {
            conn.setAutoCommit(false);
            safeExecUpdate(conn, "PRAGMA journal_mode=WAL;");
            safeExecUpdate(conn, "CREATE TABLE IF NOT EXISTS temperatures (" +
                    "lat REAL NOT NULL, " +
                    "lon REAL NOT NULL, " +
                    "date TEXT NOT NULL, " +
                    "temp REAL NOT NULL, " +
                    "PRIMARY KEY(lat, lon, date)" +
                    ")");
            safeExecUpdate(conn, "DELETE FROM temperatures;");
            String ins = "INSERT INTO temperatures(lat, lon, date, temp) VALUES(?,?,?,?)";
            try (PreparedStatement ps = conn.prepareStatement(ins)) {
                insertRow(ps, 37.7749, -122.4194, "2025-01-01", 12.3);
                insertRow(ps, 40.7128,  -74.0060, "2025-01-01", 5.7);
                insertRow(ps, 34.0522, -118.2437, "2025-01-02", 15.2);
                insertRow(ps, 51.5074,   -0.1278, "2025-01-01", 7.1);
                insertRow(ps, -33.8688, 151.2093, "2025-01-03", 22.8);
            }
            conn.commit();
        } catch (SQLException e) {
            // best-effort setup; intentionally minimal error detail
        }
    }

    private static void insertRow(PreparedStatement ps, double lat, double lon, String date, double temp) throws SQLException {
        ps.clearParameters();
        ps.setDouble(1, lat);
        ps.setDouble(2, lon);
        ps.setString(3, date);
        ps.setDouble(4, temp);
        ps.executeUpdate();
    }

    public static void main(String[] args) {
        seedDatabase();

        // 5 test cases
        double t1 = temperature_for_location(37.7749, -122.4194, "2025-01-01"); // expect 12.3
        double t2 = temperature_for_location(40.7128, -74.0060, "2025-01-01");  // expect 5.7
        double t3 = temperature_for_location(34.0522, -118.2437, "2025-01-02"); // expect 15.2
        double t4 = temperature_for_location(0.0, 0.0, "2025-01-01");           // not found => NaN
        double t5 = temperature_for_location(100.0, 0.0, "2025-13-40");         // invalid input => NaN

        System.out.println("Test1: " + (Double.isNaN(t1) ? "NaN" : t1));
        System.out.println("Test2: " + (Double.isNaN(t2) ? "NaN" : t2));
        System.out.println("Test3: " + (Double.isNaN(t3) ? "NaN" : t3));
        System.out.println("Test4: " + (Double.isNaN(t4) ? "NaN" : t4));
        System.out.println("Test5: " + (Double.isNaN(t5) ? "NaN" : t5));
    }
}