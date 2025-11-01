import java.sql.*;

public class Lse85 {
    public static void initDb() {
        try (Connection conn = DriverManager.getConnection("jdbc:sqlite:Sensor.db")) {
            try (Statement st = conn.createStatement()) {
                st.execute("PRAGMA journal_mode=WAL;");
                st.execute("CREATE TABLE IF NOT EXISTS readings (" +
                           "lat REAL NOT NULL, " +
                           "lon REAL NOT NULL, " +
                           "date TEXT NOT NULL, " +
                           "temperature REAL NOT NULL, " +
                           "PRIMARY KEY(lat, lon, date))");
                st.execute("DELETE FROM readings");
            }
            String insertSql = "INSERT INTO readings(lat, lon, date, temperature) VALUES(?, ?, ?, ?)";
            try (PreparedStatement ps = conn.prepareStatement(insertSql)) {
                insert(ps, 37.7749, -122.4194, "2025-01-01", 15.5);
                insert(ps, 34.0522, -118.2437, "2025-01-01", 18.2);
                insert(ps, 40.7128, -74.0060, "2025-01-02", 5.1);
                insert(ps, 51.5074, -0.1278, "2025-01-03", 7.8);
                insert(ps, -33.8688, 151.2093, "2025-01-04", 22.3);
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    private static void insert(PreparedStatement ps, double lat, double lon, String date, double temp) throws SQLException {
        ps.setDouble(1, lat);
        ps.setDouble(2, lon);
        ps.setString(3, date);
        ps.setDouble(4, temp);
        ps.executeUpdate();
    }

    public static double temperature_for_location(double latitude, double longitude, String date) {
        String url = "jdbc:sqlite:Sensor.db";
        String sql = "SELECT temperature FROM readings WHERE lat = ? AND lon = ? AND date = ? LIMIT 1";
        try (Connection conn = DriverManager.getConnection(url);
             PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setDouble(1, latitude);
            ps.setDouble(2, longitude);
            ps.setString(3, date);
            try (ResultSet rs = ps.executeQuery()) {
                if (rs.next()) {
                    return rs.getDouble(1);
                }
            }
        } catch (SQLException e) {
            // e.printStackTrace();
        }
        return Double.NaN;
    }

    public static void main(String[] args) {
        initDb();
        System.out.println(temperature_for_location(37.7749, -122.4194, "2025-01-01"));
        System.out.println(temperature_for_location(34.0522, -118.2437, "2025-01-01"));
        System.out.println(temperature_for_location(40.7128, -74.0060, "2025-01-02"));
        System.out.println(temperature_for_location(51.5074, -0.1278, "2025-01-03"));
        System.out.println(temperature_for_location(-33.8688, 151.2093, "2025-01-04"));
    }
}