
import java.sql.*;
import java.util.HashMap;
import java.util.Map;

public class Lse87 {
    private static final String DB_URL = "jdbc:sqlite:weather.db";
    
    public static Double temperatureForLocation(double latitude, double longitude, String date) {
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        Double temperature = null;
        
        try {
            // Input validation
            if (date == null || date.trim().isEmpty()) {
                throw new IllegalArgumentException("Date cannot be null or empty");
            }
            
            if (latitude < -90 || latitude > 90) {
                throw new IllegalArgumentException("Invalid latitude");
            }
            
            if (longitude < -180 || longitude > 180) {
                throw new IllegalArgumentException("Invalid longitude");
            }
            
            // Connect to database
            conn = DriverManager.getConnection(DB_URL);
            
            // Use parameterized query to prevent SQL injection
            String query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ?";
            pstmt = conn.prepareStatement(query);
            pstmt.setDouble(1, latitude);
            pstmt.setDouble(2, longitude);
            pstmt.setString(3, date);
            
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                temperature = rs.getDouble("temperature");
            }
            
        } catch (SQLException e) {
            System.err.println("Database error: " + e.getMessage());
        } catch (IllegalArgumentException e) {
            System.err.println("Validation error: " + e.getMessage());
        } finally {
            // Close resources in reverse order of creation
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing resources: " + e.getMessage());
            }
        }
        
        return temperature;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        Double temp1 = temperatureForLocation(40.7128, -74.0060, "2023-01-15");
        System.out.println("Temperature: " + temp1);
        
        System.out.println("\\nTest Case 2:");
        Double temp2 = temperatureForLocation(51.5074, -0.1278, "2023-06-20");
        System.out.println("Temperature: " + temp2);
        
        System.out.println("\\nTest Case 3:");
        Double temp3 = temperatureForLocation(35.6762, 139.6503, "2023-12-25");
        System.out.println("Temperature: " + temp3);
        
        System.out.println("\\nTest Case 4:");
        Double temp4 = temperatureForLocation(-33.8688, 151.2093, "2023-03-10");
        System.out.println("Temperature: " + temp4);
        
        System.out.println("\\nTest Case 5:");
        Double temp5 = temperatureForLocation(48.8566, 2.3522, "2023-09-05");
        System.out.println("Temperature: " + temp5);
    }
}
