
import java.sql.*;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeParseException;

public class Lse87 {
    private static final String DB_URL = "jdbc:sqlite:weather.db";
    
    public static class TemperatureResult {
        public boolean success;
        public Double temperature;
        public String error;
        
        public TemperatureResult(boolean success, Double temperature, String error) {
            this.success = success;
            this.temperature = temperature;
            this.error = error;
        }
    }
    
    public static TemperatureResult temperatureForLocation(String latitudeStr, String longitudeStr, String dateStr) {
        // Input validation
        if (latitudeStr == null || longitudeStr == null || dateStr == null) {
            return new TemperatureResult(false, null, "Missing parameters");
        }
        
        double latitude, longitude;
        LocalDate date;
        
        try {
            latitude = Double.parseDouble(latitudeStr);
            longitude = Double.parseDouble(longitudeStr);
            
            // Validate latitude and longitude ranges
            if (latitude < -90 || latitude > 90) {
                return new TemperatureResult(false, null, "Invalid latitude range");
            }
            if (longitude < -180 || longitude > 180) {
                return new TemperatureResult(false, null, "Invalid longitude range");
            }
            
            // Parse and validate date
            DateTimeFormatter formatter = DateTimeFormatter.ofPattern("yyyy-MM-dd");
            date = LocalDate.parse(dateStr, formatter);
            
        } catch (NumberFormatException e) {
            return new TemperatureResult(false, null, "Invalid number format");
        } catch (DateTimeParseException e) {
            return new TemperatureResult(false, null, "Invalid date format");
        }
        
        // Database query with parameterized statement
        String query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ?";
        
        try (Connection conn = DriverManager.getConnection(DB_URL);
             PreparedStatement pstmt = conn.prepareStatement(query)) {
            
            pstmt.setDouble(1, latitude);
            pstmt.setDouble(2, longitude);
            pstmt.setString(3, dateStr);
            
            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    double temperature = rs.getDouble("temperature");
                    return new TemperatureResult(true, temperature, null);
                } else {
                    return new TemperatureResult(false, null, "No data found");
                }
            }
            
        } catch (SQLException e) {
            return new TemperatureResult(false, null, "Database error");
        }
    }
    
    public static void main(String[] args) {
        // Test cases
        System.out.println("Test 1: Valid input");
        TemperatureResult result1 = temperatureForLocation("40.7128", "-74.0060", "2024-01-15");
        System.out.println("Success: " + result1.success + ", Temp: " + result1.temperature + ", Error: " + result1.error);
        
        System.out.println("\\nTest 2: Invalid latitude");
        TemperatureResult result2 = temperatureForLocation("100.0", "-74.0060", "2024-01-15");
        System.out.println("Success: " + result2.success + ", Error: " + result2.error);
        
        System.out.println("\\nTest 3: Invalid date format");
        TemperatureResult result3 = temperatureForLocation("40.7128", "-74.0060", "15-01-2024");
        System.out.println("Success: " + result3.success + ", Error: " + result3.error);
        
        System.out.println("\\nTest 4: Null parameters");
        TemperatureResult result4 = temperatureForLocation(null, "-74.0060", "2024-01-15");
        System.out.println("Success: " + result4.success + ", Error: " + result4.error);
        
        System.out.println("\\nTest 5: Invalid number format");
        TemperatureResult result5 = temperatureForLocation("abc", "-74.0060", "2024-01-15");
        System.out.println("Success: " + result5.success + ", Error: " + result5.error);
    }
}
