import java.sql.*;
import java.time.LocalDate;
import java.time.format.DateTimeParseException;
import java.util.Objects;

public class Lse85 {
    private static final String DEFAULT_DB_URL = "jdbc:sqlite:Sensor.db";

    // Initialize database with a SensorReadings table and sample data
    public static void initDatabase(String dbUrl) {
        Objects.requireNonNull(dbUrl, "dbUrl");
        try (Connection conn = DriverManager.getConnection(dbUrl)) {
            if (conn == null) return;
            try (Statement st = conn.createStatement()) {
                st.execute("PRAGMA foreign_keys = ON;");
                st.execute("PRAGMA busy_timeout = 3000;");
                st.execute("CREATE TABLE IF NOT EXISTS SensorReadings (" +
                        "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                        "latitude REAL NOT NULL," +
                        "longitude REAL NOT NULL," +
                        "date TEXT NOT NULL," +
                        "temperature REAL NOT NULL," +
                        "UNIQUE(latitude, longitude, date)" +
                        ");");
            }
            String upsert = "INSERT OR IGNORE INTO SensorReadings(latitude, longitude, date, temperature) VALUES(?, ?, ?, ?);";
            try (PreparedStatement ps = conn.prepareStatement(upsert)) {
                insertSample(ps, 37.7749, -122.4194, "2023-08-15", 21.5);
                insertSample(ps, 37.7749, -122.4194, "2023-08-16", 20.0);
                insertSample(ps, 40.7128, -74.0060, "2023-08-15", 25.2);
                insertSample(ps, 34.0522, -118.2437, "2023-08-15", 27.1);
            }
        } catch (SQLException e) {
            // Minimal error output for security; in production use proper logging
            System.err.println("Database initialization error.");
        }
    }

    private static void insertSample(PreparedStatement ps, double lat, double lon, String date, double temp) throws SQLException {
        ps.setDouble(1, lat);
        ps.setDouble(2, lon);
        ps.setString(3, date);
        ps.setDouble(4, temp);
        ps.executeUpdate();
    }

    private static void validateInputs(double latitude, double longitude, String date) {
        if (Double.isNaN(latitude) || latitude < -90.0 || latitude > 90.0) {
            throw new IllegalArgumentException("Invalid latitude");
        }
        if (Double.isNaN(longitude) || longitude < -180.0 || longitude > 180.0) {
            throw new IllegalArgumentException("Invalid longitude");
        }
        try {
            LocalDate.parse(date);
        } catch (DateTimeParseException ex) {
            throw new IllegalArgumentException("Invalid date format, expected YYYY-MM-DD");
        }
    }

    // Function called for /api/temperature_for_location: connects to DB "Sensor" and returns temperature
    public static Double temperature_for_location(double latitude, double longitude, String date, String dbUrl) {
        Objects.requireNonNull(date, "date");
        if (dbUrl == null || dbUrl.isEmpty()) dbUrl = DEFAULT_DB_URL;
        validateInputs(latitude, longitude, date);

        String sql = "SELECT temperature FROM SensorReadings WHERE latitude = ? AND longitude = ? AND date = ?;";
        try (Connection conn = DriverManager.getConnection(dbUrl)) {
            if (conn == null) return null;
            try (Statement st = conn.createStatement()) {
                st.execute("PRAGMA foreign_keys = ON;");
                st.execute("PRAGMA busy_timeout = 3000;");
            }
            try (PreparedStatement ps = conn.prepareStatement(sql)) {
                ps.setDouble(1, latitude);
                ps.setDouble(2, longitude);
                ps.setString(3, date);
                try (ResultSet rs = ps.executeQuery()) {
                    if (rs.next()) {
                        return rs.getDouble("temperature");
                    }
                }
            }
        } catch (SQLException e) {
            System.err.println("Database query error.");
        }
        return null;
    }

    public static void main(String[] args) {
        String dbUrl = DEFAULT_DB_URL;
        initDatabase(dbUrl);

        // 5 test cases
        Double t1 = temperature_for_location(37.7749, -122.4194, "2023-08-15", dbUrl);
        Double t2 = temperature_for_location(37.7749, -122.4194, "2023-08-16", dbUrl);
        Double t3 = temperature_for_location(40.7128, -74.0060, "2023-08-15", dbUrl);
        Double t4 = temperature_for_location(34.0522, -118.2437, "2023-08-15", dbUrl);
        Double t5 = temperature_for_location(0.0, 0.0, "2023-01-01", dbUrl);

        System.out.println("Test1: " + (t1 != null ? t1 : "not found"));
        System.out.println("Test2: " + (t2 != null ? t2 : "not found"));
        System.out.println("Test3: " + (t3 != null ? t3 : "not found"));
        System.out.println("Test4: " + (t4 != null ? t4 : "not found"));
        System.out.println("Test5: " + (t5 != null ? t5 : "not found"));
    }
}