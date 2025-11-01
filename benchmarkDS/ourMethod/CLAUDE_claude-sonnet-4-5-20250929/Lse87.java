
import java.sql.*;
import java.time.LocalDate;
import java.time.format.DateTimeFormatter;
import java.time.format.DateTimeParseException;
import java.util.Properties;

public class Lse87 {
    private static final String DB_URL = "jdbc:postgresql://localhost:5432/weather";
    private static final String DB_USER = "weatheruser";
    private static final String DB_PASSWORD = "securepassword";
    
    // Maximum allowed values to prevent resource exhaustion
    private static final double MAX_LATITUDE = 90.0;
    private static final double MIN_LATITUDE = -90.0;
    private static final double MAX_LONGITUDE = 180.0;
    private static final double MIN_LONGITUDE = -180.0;
    
    /**
     * Retrieves temperature for a given location and date.
     * Uses parameterized queries to prevent SQL injection.
     * Validates all inputs before processing.
     * 
     * @param latitude The latitude coordinate
     * @param longitude The longitude coordinate
     * @param date The date in ISO format (YYYY-MM-DD)
     * @return Temperature value or error message
     */
    public static String temperatureForLocation(String latitude, String longitude, String date) {
        // Input validation: Check for null or empty inputs
        if (latitude == null || latitude.trim().isEmpty()) {
            return "Error: Invalid latitude parameter";
        }
        if (longitude == null || longitude.trim().isEmpty()) {
            return "Error: Invalid longitude parameter";
        }
        if (date == null || date.trim().isEmpty()) {
            return "Error: Invalid date parameter";
        }
        
        double lat;
        double lon;
        LocalDate parsedDate;
        
        try {
            // Input validation: Parse and validate latitude
            lat = Double.parseDouble(latitude.trim());
            if (lat < MIN_LATITUDE || lat > MAX_LATITUDE) {
                return "Error: Latitude out of valid range";
            }
            
            // Input validation: Parse and validate longitude
            lon = Double.parseDouble(longitude.trim());
            if (lon < MIN_LONGITUDE || lon > MAX_LONGITUDE) {
                return "Error: Longitude out of valid range";
            }
            
            // Input validation: Parse and validate date format
            DateTimeFormatter formatter = DateTimeFormatter.ISO_LOCAL_DATE;
            parsedDate = LocalDate.parse(date.trim(), formatter);
            
        } catch (NumberFormatException e) {
            // Do not leak internal details in error messages
            return "Error: Invalid coordinate format";
        } catch (DateTimeParseException e) {
            return "Error: Invalid date format";
        }
        
        Connection conn = null;
        PreparedStatement pstmt = null;
        ResultSet rs = null;
        
        try {
            // Setup secure database connection properties
            Properties props = new Properties();
            props.setProperty("user", DB_USER);
            props.setProperty("password", DB_PASSWORD);
            props.setProperty("ssl", "true");
            props.setProperty("sslmode", "verify-full");
            
            // Establish database connection
            conn = DriverManager.getConnection(DB_URL, props);
            
            // Use parameterized query to prevent SQL injection
            String query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ? LIMIT 1";
            pstmt = conn.prepareStatement(query);
            
            // Set parameters with proper types
            pstmt.setDouble(1, lat);
            pstmt.setDouble(2, lon);
            pstmt.setDate(3, Date.valueOf(parsedDate));
            
            // Execute query
            rs = pstmt.executeQuery();
            
            if (rs.next()) {
                double temperature = rs.getDouble("temperature");
                return String.format("%.2f", temperature);
            } else {
                return "Error: No data found for given parameters";
            }
            
        } catch (SQLException e) {
            // Log detailed error internally but return generic message
            System.err.println("Database error: " + e.getMessage());
            return "Error: Database operation failed";
        } finally {
            // Ensure all resources are closed properly
            try {
                if (rs != null) rs.close();
            } catch (SQLException e) {
                System.err.println("Error closing ResultSet: " + e.getMessage());
            }
            try {
                if (pstmt != null) pstmt.close();
            } catch (SQLException e) {
                System.err.println("Error closing PreparedStatement: " + e.getMessage());
            }
            try {
                if (conn != null) conn.close();
            } catch (SQLException e) {
                System.err.println("Error closing Connection: " + e.getMessage());
            }
        }
    }
    
    public static void main(String[] args) {
        // Test case 1: Valid input
        System.out.println("Test 1: " + temperatureForLocation("40.7128", "-74.0060", "2024-01-15"));
        
        // Test case 2: Invalid latitude (out of range)
        System.out.println("Test 2: " + temperatureForLocation("95.0", "-74.0060", "2024-01-15"));
        
        // Test case 3: Invalid date format
        System.out.println("Test 3: " + temperatureForLocation("40.7128", "-74.0060", "01/15/2024"));
        
        // Test case 4: Null input
        System.out.println("Test 4: " + temperatureForLocation(null, "-74.0060", "2024-01-15"));
        
        // Test case 5: SQL injection attempt (safely handled by parameterized query)
        System.out.println("Test 5: " + temperatureForLocation("40.7128' OR '1'='1", "-74.0060", "2024-01-15"));
    }
}
