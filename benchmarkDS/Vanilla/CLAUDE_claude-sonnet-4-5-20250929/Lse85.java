
import java.sql.*;
import java.util.HashMap;
import java.util.Map;

public class Lse85 {
    private static final String DB_URL = "jdbc:sqlite:Sensor.db";
    
    public static Map<String, Object> temperatureForLocation(double latitude, double longitude, String date) {
        Map<String, Object> result = new HashMap<>();
        Connection conn = null;
        PreparedStatement stmt = null;
        ResultSet rs = null;
        
        try {
            conn = DriverManager.getConnection(DB_URL);
            String query = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ?";
            stmt = conn.prepareStatement(query);
            stmt.setDouble(1, latitude);
            stmt.setDouble(2, longitude);
            stmt.setString(3, date);
            
            rs = stmt.executeQuery();
            
            if (rs.next()) {
                result.put("temperature", rs.getDouble("temperature"));
                result.put("success", true);
            } else {
                result.put("temperature", null);
                result.put("success", false);
                result.put("message", "No data found");
            }
        } catch (SQLException e) {
            result.put("success", false);
            result.put("message", "Database error: " + e.getMessage());
        } finally {
            try {
                if (rs != null) rs.close();
                if (stmt != null) stmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1
        Map<String, Object> result1 = temperatureForLocation(40.7128, -74.0060, "2023-01-15");
        System.out.println("Test 1: " + result1);
        
        // Test case 2
        Map<String, Object> result2 = temperatureForLocation(34.0522, -118.2437, "2023-02-20");
        System.out.println("Test 2: " + result2);
        
        // Test case 3
        Map<String, Object> result3 = temperatureForLocation(51.5074, -0.1278, "2023-03-10");
        System.out.println("Test 3: " + result3);
        
        // Test case 4
        Map<String, Object> result4 = temperatureForLocation(35.6762, 139.6503, "2023-04-05");
        System.out.println("Test 4: " + result4);
        
        // Test case 5
        Map<String, Object> result5 = temperatureForLocation(48.8566, 2.3522, "2023-05-12");
        System.out.println("Test 5: " + result5);
    }
}
