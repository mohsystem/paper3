import java.sql.*;
import java.util.regex.Pattern;

public class Lse85 {
    private static final String DB_URL = "jdbc:sqlite:Sensor.db";
    private static final Pattern DATE_RE = Pattern.compile("^\\d{4}-\\d{2}-\\d{2}$");

    // Initialize the database schema securely
    private static void initDatabase() {
        String createTableSql = "CREATE TABLE IF NOT EXISTS temperatures (" +
                "id INTEGER PRIMARY KEY AUTOINCREMENT," +
                "lat REAL NOT NULL," +
                "lon REAL NOT NULL," +
                "date TEXT NOT NULL," +
                "temp REAL NOT NULL," +
                "UNIQUE(lat, lon, date)" +
                ");";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             Statement stmt = conn.createStatement()) {
            stmt.execute(createTableSql);
        } catch (SQLException e) {
            System.err.println("DB init error: " + e.getMessage());
        }
    }

    // Seed some sample data using parameterized queries
    private static void seedSampleData() {
        String insertSql = "INSERT OR REPLACE INTO temperatures (lat, lon, date, temp) VALUES (?, ?, ?, ?);";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement ps = conn.prepareStatement(insertSql)) {
            conn.setAutoCommit(false);
            addRow(ps, 37.7749, -122.4194, "2023-07-01", 20.5);
            addRow(ps, 40.7128, -74.0060, "2023-07-01", 27.3);
            addRow(ps, 51.5074, -0.1278, "2023-12-25", 5.2);
            addRow(ps, 35.6895, 139.6917, "2023-04-15", 18.0);
            addRow(ps, 48.8566, 2.3522, "2023-07-14", 25.0);
            conn.commit();
        } catch (SQLException e) {
            System.err.println("Seeding error: " + e.getMessage());
        }
    }

    private static void addRow(PreparedStatement ps, double lat, double lon, String date, double temp) throws SQLException {
        ps.clearParameters();
        ps.setDouble(1, lat);
        ps.setDouble(2, lon);
        ps.setString(3, date);
        ps.setDouble(4, temp);
        ps.executeUpdate();
    }

    private static boolean validLatLon(double lat, double lon) {
        return lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0;
    }

    private static boolean validDate(String date) {
        return date != null && DATE_RE.matcher(date).matches();
    }

    // Function to fetch temperature; this would be called when /api/temperature_for_location is requested
    public static Double temperature_for_location(double lat, double lon, String date) {
        if (!validLatLon(lat, lon) || !validDate(date)) {
            return null;
        }
        String query = "SELECT temp FROM temperatures WHERE lat = ? AND lon = ? AND date = ?;";
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement ps = conn.prepareStatement(query)) {
            ps.setDouble(1, lat);
            ps.setDouble(2, lon);
            ps.setString(3, date);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    return rs.getDouble(1);
                }
            }
        } catch (SQLException e) {
            System.err.println("Query error: " + e.getMessage());
        }
        return null;
    }

    // 5 test cases
    public static void main(String[] args) {
        initDatabase();
        seedSampleData();

        Double t1 = temperature_for_location(37.7749, -122.4194, "2023-07-01");
        Double t2 = temperature_for_location(40.7128, -74.0060, "2023-07-01");
        Double t3 = temperature_for_location(51.5074, -0.1278, "2023-01-01"); // not found
        Double t4 = temperature_for_location(95.0, 10.0, "2023-07-01"); // invalid lat
        Double t5 = temperature_for_location(48.8566, 2.3522, "2023-07-14");

        System.out.println("Test1: " + (t1 != null ? t1 : "not found or invalid"));
        System.out.println("Test2: " + (t2 != null ? t2 : "not found or invalid"));
        System.out.println("Test3: " + (t3 != null ? t3 : "not found or invalid"));
        System.out.println("Test4: " + (t4 != null ? t4 : "not found or invalid"));
        System.out.println("Test5: " + (t5 != null ? t5 : "not found or invalid"));
    }
}