
import java.sql.*;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeParseException;

public class Lse85 {
    private static final String DB_URL = "jdbc:sqlite:Sensor.db";
    
    public static class TemperatureRequest {
        public double latitude;
        public double longitude;
        public String date;
    }
    
    public static Double temperatureForLocation(double latitude, double longitude, String date) {
        // Input validation
        if (!isValidLatitude(latitude) || !isValidLongitude(longitude) || !isValidDate(date)) {
            return null;
        }
        
        String query = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setDouble(1, latitude);
            pstmt.setDouble(2, longitude);
            pstmt.setString(3, date);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return rs.getDouble("temperature");
                }
            }
        } catch (SQLException e) {
            System.err.println("Database error occurred");
            return null;
        }
        
        return null;
    }
    
    private static boolean isValidLatitude(double lat) {
        return lat >= -90.0 && lat <= 90.0;
    }
    
    private static boolean isValidLongitude(double lon) {
        return lon >= -180.0 && lon <= 180.0;
    }
    
    private static boolean isValidDate(String date) {
        try {
            LocalDate.parse(date, DateTimeFormatter.ISO_LOCAL_DATE);
            return true;
        } catch (DateTimeParseException e) {
            return false;
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: " + temperatureForLocation(40.7128, -74.0060, "2024-01-15"));
        System.out.println("Test 2: " + temperatureForLocation(51.5074, -0.1278, "2024-02-20"));
        System.out.println("Test 3: " + temperatureForLocation(35.6762, 139.6503, "2024-03-10"));
        System.out.println("Test 4 (Invalid lat): " + temperatureForLocation(100.0, -74.0060, "2024-01-15"));
        System.out.println("Test 5 (Invalid date): " + temperatureForLocation(40.7128, -74.0060, "2024-13-45"));
    }
}
