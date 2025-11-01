
import java.sql.*;
import java.util.HashMap;
import java.util.Map;

public class Lse87 {
    private static final String DB_URL = "jdbc:mysql://localhost:3306/weather_db";
    private static final String DB_USER = "root";
    private static final String DB_PASSWORD = "password";
    
    public static Double temperatureForLocation(double latitude, double longitude, String date) {
        Connection conn = null;
        PreparedStatement stmt = null;
        ResultSet rs = null;
        Double temperature = null;
        
        try {
            conn = DriverManager.getConnection(DB_URL, DB_USER, DB_PASSWORD);
            
            String query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ?";
            stmt = conn.prepareStatement(query);
            stmt.setDouble(1, latitude);
            stmt.setDouble(2, longitude);
            stmt.setString(3, date);
            
            rs = stmt.executeQuery();
            
            if (rs.next()) {
                temperature = rs.getDouble("temperature");
            }
        } catch (SQLException e) {
            e.printStackTrace();
        } finally {
            try {
                if (rs != null) rs.close();
                if (stmt != null) stmt.close();
                if (conn != null) conn.close();
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
        
        return temperature;
    }
    
    public static void main(String[] args) {
        // Test case 1
        Double temp1 = temperatureForLocation(40.7128, -74.0060, "2024-01-15");
        System.out.println("Test 1 - Temperature: " + temp1);
        
        // Test case 2
        Double temp2 = temperatureForLocation(34.0522, -118.2437, "2024-02-20");
        System.out.println("Test 2 - Temperature: " + temp2);
        
        // Test case 3
        Double temp3 = temperatureForLocation(51.5074, -0.1278, "2024-03-10");
        System.out.println("Test 3 - Temperature: " + temp3);
        
        // Test case 4
        Double temp4 = temperatureForLocation(35.6762, 139.6503, "2024-04-05");
        System.out.println("Test 4 - Temperature: " + temp4);
        
        // Test case 5
        Double temp5 = temperatureForLocation(-33.8688, 151.2093, "2024-05-12");
        System.out.println("Test 5 - Temperature: " + temp5);
    }
}
