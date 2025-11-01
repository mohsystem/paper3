import java.sql.*;
import java.time.LocalDate;
import java.time.format.DateTimeParseException;

public class Lse87 {
    private static final String DB_PATH = "weather.db";

    public static Double temperature_for_location(double latitude, double longitude, String date) {
        if (!isValidLatitude(latitude) || !isValidLongitude(longitude) || !isValidDate(date)) {
            return null;
        }

        String url = "jdbc:sqlite:" + DB_PATH;
        String sql = "SELECT temperature FROM temperatures WHERE latitude = ? AND longitude = ? AND date = ? LIMIT 1";

        try (Connection conn = DriverManager.getConnection(url);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            pstmt.setDouble(1, latitude);
            pstmt.setDouble(2, longitude);
            pstmt.setString(3, date);

            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return rs.getDouble(1);
                }
            }
        } catch (SQLException e) {
            // Log securely in real applications
            return null;
        }
        return null;
    }

    private static boolean isValidLatitude(double lat) {
        return Double.isFinite(lat) && lat >= -90.0 && lat <= 90.0;
    }

    private static boolean isValidLongitude(double lon) {
        return Double.isFinite(lon) && lon >= -180.0 && lon <= 180.0;
    }

    private static boolean isValidDate(String dateStr) {
        if (dateStr == null) return false;
        try {
            if (dateStr.length() != 10) return false;
            if (dateStr.charAt(4) != '-' || dateStr.charAt(7) != '-') return false;
            LocalDate.parse(dateStr);
            return true;
        } catch (DateTimeParseException ex) {
            return false;
        }
    }

    private static void setupDatabase() {
        String url = "jdbc:sqlite:" + DB_PATH;
        String createTable = "CREATE TABLE IF NOT EXISTS temperatures (" +
                "latitude REAL NOT NULL," +
                "longitude REAL NOT NULL," +
                "date TEXT NOT NULL," +
                "temperature REAL NOT NULL," +
                "PRIMARY KEY (latitude, longitude, date)" +
                ")";
        String clear = "DELETE FROM temperatures";
        try (Connection conn = DriverManager.getConnection(url);
             Statement stmt = conn.createStatement()) {
            stmt.execute(createTable);
            stmt.execute(clear);
        } catch (SQLException e) {
            // Log securely in real applications
        }

        insertTemperature(10.0, 20.0, "2025-01-01", 23.5);
        insertTemperature(10.0, 20.0, "2025-01-02", 24.0);
        insertTemperature(40.7128, -74.0060, "2023-12-25", -1.0);
        insertTemperature(-33.8688, 151.2093, "2024-07-04", 12.3);
        insertTemperature(51.5074, -0.1278, "2022-11-11", 8.5);
    }

    private static void insertTemperature(double lat, double lon, String date, double temp) {
        String url = "jdbc:sqlite:" + DB_PATH;
        String sql = "INSERT OR REPLACE INTO temperatures (latitude, longitude, date, temperature) VALUES (?, ?, ?, ?)";
        try (Connection conn = DriverManager.getConnection(url);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {
            pstmt.setDouble(1, lat);
            pstmt.setDouble(2, lon);
            pstmt.setString(3, date);
            pstmt.setDouble(4, temp);
            pstmt.executeUpdate();
        } catch (SQLException e) {
            // Log securely in real applications
        }
    }

    public static void main(String[] args) {
        setupDatabase();
        Double[] results = new Double[5];
        results[0] = temperature_for_location(10.0, 20.0, "2025-01-01");
        results[1] = temperature_for_location(40.7128, -74.0060, "2023-12-25");
        results[2] = temperature_for_location(10.0, 20.0, "2025-01-03"); // not found
        results[3] = temperature_for_location(100.0, 20.0, "2025-01-01"); // invalid lat
        results[4] = temperature_for_location(10.0, 20.0, "2025/01/01");  // invalid date

        for (int i = 0; i < results.length; i++) {
            System.out.println("Test " + (i + 1) + ": " + (results[i] == null ? "null" : results[i]));
        }
    }
}