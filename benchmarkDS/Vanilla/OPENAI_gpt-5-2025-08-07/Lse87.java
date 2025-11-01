import java.sql.*;

public class Lse87 {

    public static Double temperature_for_location(double latitude, double longitude, String date) {
        Connection conn = null;
        try {
            try {
                Class.forName("org.sqlite.JDBC");
            } catch (ClassNotFoundException ignore) {
                // Driver may auto-load if available
            }
            conn = DriverManager.getConnection("jdbc:sqlite::memory:");
            try (Statement st = conn.createStatement()) {
                st.executeUpdate("CREATE TABLE IF NOT EXISTS temperatures (lat REAL, lon REAL, date TEXT, temp REAL)");
            }
            try (PreparedStatement ins = conn.prepareStatement("INSERT INTO temperatures(lat, lon, date, temp) VALUES(?,?,?,?)")) {
                insertRow(ins, 37.7749, -122.4194, "2025-01-01", 15.2);
                insertRow(ins, 40.7128, -74.0060, "2025-01-01", 3.4);
                insertRow(ins, 34.0522, -118.2437, "2025-01-01", 20.1);
                insertRow(ins, 51.5074, -0.1278, "2025-01-01", 7.8);
                insertRow(ins, 35.6895, 139.6917, "2025-01-01", 8.9);
                insertRow(ins, 37.7749, -122.4194, "2025-02-01", 16.0);
            }
            try (PreparedStatement ps = conn.prepareStatement("SELECT temp FROM temperatures WHERE lat=? AND lon=? AND date=? LIMIT 1")) {
                ps.setDouble(1, latitude);
                ps.setDouble(2, longitude);
                ps.setString(3, date);
                try (ResultSet rs = ps.executeQuery()) {
                    if (rs.next()) {
                        return rs.getDouble(1);
                    }
                }
            }
            return null;
        } catch (Exception e) {
            return null;
        } finally {
            if (conn != null) {
                try { conn.close(); } catch (Exception ignore) {}
            }
        }
    }

    private static void insertRow(PreparedStatement ins, double lat, double lon, String date, double temp) throws SQLException {
        ins.setDouble(1, lat);
        ins.setDouble(2, lon);
        ins.setString(3, date);
        ins.setDouble(4, temp);
        ins.executeUpdate();
    }

    public static void main(String[] args) {
        System.out.println(temperature_for_location(37.7749, -122.4194, "2025-01-01")); // 15.2
        System.out.println(temperature_for_location(40.7128, -74.0060, "2025-01-01"));  // 3.4
        System.out.println(temperature_for_location(34.0522, -118.2437, "2025-01-01")); // 20.1
        System.out.println(temperature_for_location(51.5074, -0.1278, "2025-01-01"));   // 7.8
        System.out.println(temperature_for_location(37.7749, -122.4194, "2025-02-01")); // 16.0
    }
}