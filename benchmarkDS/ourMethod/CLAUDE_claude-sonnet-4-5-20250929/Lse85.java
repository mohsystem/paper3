
import java.sql.*;
import java.util.*;
import java.util.regex.*;
import java.io.*;

public class Lse85 {
    private static final String DB_URL = "jdbc:sqlite:Sensor.db";
    private static final Pattern DATE_PATTERN = Pattern.compile("^\\\\d{4}-\\\\d{2}-\\\\d{2}$");
    private static final double MIN_LAT = -90.0;
    private static final double MAX_LAT = 90.0;
    private static final double MIN_LON = -180.0;
    private static final double MAX_LON = 180.0;
    
    // Represents the API response
    public static class TemperatureResponse {
        public final boolean success;
        public final Double temperature;
        public final String error;
        
        public TemperatureResponse(boolean success, Double temperature, String error) {
            this.success = success;
            this.temperature = temperature;
            this.error = error;
        }
    }
    
    /**
     * Validates latitude value
     * Prevents injection and ensures valid geographic coordinate
     */
    private static boolean isValidLatitude(double lat) {
        return lat >= MIN_LAT && lat <= MAX_LAT;
    }
    
    /**
     * Validates longitude value
     * Prevents injection and ensures valid geographic coordinate
     */
    private static boolean isValidLongitude(double lon) {
        return lon >= MIN_LON && lon <= MAX_LON;
    }
    
    /**
     * Validates date format (YYYY-MM-DD)
     * Prevents SQL injection via date parameter
     */
    private static boolean isValidDate(String date) {
        if (date == null || date.length() > 10) {
            return false;
        }
        return DATE_PATTERN.matcher(date).matches();
    }
    
    /**
     * Main API function to get temperature for a location
     * Uses parameterized queries to prevent SQL injection
     * Validates all inputs before processing
     * Fails closed on any validation error
     */
    public static TemperatureResponse temperatureForLocation(double latitude, double longitude, String date) {
        // Input validation - reject invalid data early
        if (!isValidLatitude(latitude)) {
            return new TemperatureResponse(false, null, "Invalid request");
        }
        
        if (!isValidLongitude(longitude)) {
            return new TemperatureResponse(false, null, "Invalid request");
        }
        
        if (!isValidDate(date)) {
            return new TemperatureResponse(false, null, "Invalid request");
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Use try-with-resources alternative for older Java versions
            // Load SQLite driver
            Class.forName("org.sqlite.JDBC");
            
            // Connect to database
            conn = DriverManager.getConnection(DB_URL);
            
            // Use parameterized query to prevent SQL injection
            // All user inputs are bound as parameters, never concatenated
            String query = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ? LIMIT 1";
            pstmt = conn.prepareStatement(query);
            
            // Bind parameters with type safety
            pstmt.setDouble(1, latitude);
            pstmt.setDouble(2, longitude);
            pstmt.setString(3, date);
            
            // Execute query
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                double temperature = rs.getDouble("temperature");
                // Check for NULL values in database
                if (rs.wasNull()) {
                    return new TemperatureResponse(false, null, "Data not available");
                }
                return new TemperatureResponse(true, temperature, null);
            } else {
                return new TemperatureResponse(false, null, "Data not found");
            }
            
        } catch (ClassNotFoundException e) {
            // Generic error message - do not leak internal details
            return new TemperatureResponse(false, null, "Service error");
        } catch (SQLException e) {
            // Generic error message - do not leak database structure
            return new TemperatureResponse(false, null, "Service error");
        } finally {
            // Ensure resources are freed exactly once
            try {
                if (rs != null) rs.close();
            } catch (SQLException e) {
                // Log internally but do not expose
            }
            try {
                if (pstmt != null) pstmt.close();
            } catch (SQLException e) {
                // Log internally but do not expose
            }
            try {
                if (conn != null) conn.close();
            } catch (SQLException e) {
                // Log internally but do not expose
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid request
        TemperatureResponse result1 = temperatureForLocation(40.7128, -74.0060, "2024-01-15");
        System.out.println("Test 1 - Success: " + result1.success + ", Temp: " + result1.temperature + ", Error: " + result1.error);
        
        // Test case 2: Invalid latitude
        TemperatureResponse result2 = temperatureForLocation(95.0, -74.0060, "2024-01-15");
        System.out.println("Test 2 - Success: " + result2.success + ", Temp: " + result2.temperature + ", Error: " + result2.error);
        
        // Test case 3: Invalid date format (SQL injection attempt)
        TemperatureResponse result3 = temperatureForLocation(40.7128, -74.0060, "2024-01-15' OR '1'='1");
        System.out.println("Test 3 - Success: " + result3.success + ", Temp: " + result3.temperature + ", Error: " + result3.error);
        
        // Test case 4: Invalid longitude
        TemperatureResponse result4 = temperatureForLocation(40.7128, -200.0, "2024-01-15");
        System.out.println("Test 4 - Success: " + result4.success + ", Temp: " + result4.temperature + ", Error: " + result4.error);
        
        // Test case 5: Valid coordinates, different date
        TemperatureResponse result5 = temperatureForLocation(51.5074, -0.1278, "2024-02-20");
        System.out.println("Test 5 - Success: " + result5.success + ", Temp: " + result5.temperature + ", Error: " + result5.error);
    }
}
