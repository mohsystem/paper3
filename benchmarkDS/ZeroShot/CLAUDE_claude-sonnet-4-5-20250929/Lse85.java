
import java.sql.*;
import java.util.HashMap;
import java.util.Map;

public class Lse85 {
    private static final String DB_URL = "jdbc:sqlite:Sensor.db";
    
    public static Map<String, Object> temperatureForLocation(double latitude, double longitude, String date) {
        Map<String, Object> result = new HashMap<>();
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Input validation
            if (date == null || date.trim().isEmpty()) {
                result.put("error", "Invalid date");
                return result;
            }
            
            if (latitude < -90 || latitude > 90 || longitude < -180 || longitude > 180) {
                result.put("error", "Invalid coordinates");
                return result;
            }
            
            // Connect to database
            conn = DriverManager.getConnection(DB_URL);
            
            // Prepare SQL query with parameterized statement to prevent SQL injection
            String sql = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ?";
            pstmt = conn.prepareStatement(sql);
            pstmt.setDouble(1, latitude);
            pstmt.setDouble(2, longitude);
            pstmt.setString(3, date);
            
            // Execute query
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                result.put("temperature", rs.getDouble("temperature"));
                result.put("success", true);
            } else {
                result.put("error", "No data found");
                result.put("success", false);
            }
            
        } catch (SQLException e) {
            result.put("error", "Database error: " + e.getMessage());
            result.put("success", false);
        } finally {
            // Close resources
            try {
                if (rs != null) rs.close();
                if (pstmt != null) pstmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test Case 1:");
        System.out.println(temperatureForLocation(40.7128, -74.0060, "2023-01-15"));
        
        System.out.println("\\nTest Case 2:");
        System.out.println(temperatureForLocation(51.5074, -0.1278, "2023-02-20"));
        
        System.out.println("\\nTest Case 3:");
        System.out.println(temperatureForLocation(35.6762, 139.6503, "2023-03-10"));
        
        System.out.println("\\nTest Case 4 (Invalid coordinates):");
        System.out.println(temperatureForLocation(100.0, -200.0, "2023-04-05"));
        
        System.out.println("\\nTest Case 5 (Empty date):");
        System.out.println(temperatureForLocation(48.8566, 2.3522, ""));
    }
}
