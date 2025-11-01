import java.util.Map;
import java.util.HashMap;
import java.time.format.DateTimeParseException;
import java.time.LocalDate;
import java.util.Optional;

// In a real application, this would be a Spring Boot Controller or a Servlet.
public class Lse87 {

    // This HashMap simulates a database table 'temperatures'.
    // In a real application, you would connect to a database (e.g., PostgreSQL, MySQL).
    private static final Map<String, Double> temperatureDatabase = new HashMap<>();

    static {
        // Populating our mock database with some data.
        // The key is a combination of "latitude,longitude,date".
        temperatureDatabase.put("40.7128,-74.0060,2023-10-27", 15.5);
        temperatureDatabase.put("34.0522,-118.2437,2023-10-27", 22.1);
        temperatureDatabase.put("48.8566,2.3522,2023-10-26", 12.8);
    }
    
    /**
     * Securely retrieves temperature for a given location and date.
     * This function simulates a web API endpoint handler.
     *
     * @param latitude The latitude of the location (-90 to 90).
     * @param longitude The longitude of the location (-180 to 180).
     * @param dateStr The date in 'YYYY-MM-DD' format.
     * @return An Optional containing the temperature if found, otherwise an empty Optional.
     */
    public static Optional<Double> temperatureForLocation(double latitude, double longitude, String dateStr) {
        // 1. **Input Validation**: Crucial security step to prevent invalid data processing.
        if (latitude < -90.0 || latitude > 90.0) {
            System.err.println("Error: Invalid latitude value.");
            return Optional.empty();
        }
        if (longitude < -180.0 || longitude > 180.0) {
            System.err.println("Error: Invalid longitude value.");
            return Optional.empty();
        }
        try {
            LocalDate.parse(dateStr); // Validates date format.
        } catch (DateTimeParseException e) {
            System.err.println("Error: Invalid date format. Please use YYYY-MM-DD.");
            return Optional.empty();
        }

        // 2. **Secure Database Query (Demonstration)**
        // In a real application with a database connection (e.g., JDBC),
        // ALWAYS use PreparedStatement to prevent SQL Injection.
        // DO NOT build queries by concatenating strings.
        /*
        // --- START OF SECURE JDBC EXAMPLE (for illustration) ---
        String secureQuery = "SELECT temperature FROM temperatures WHERE latitude = ? AND longitude = ? AND date = ?;";
        try (Connection conn = DriverManager.getConnection("jdbc:yourdb", "user", "password");
             PreparedStatement pstmt = conn.prepareStatement(secureQuery)) {
            
            // Bind parameters to prevent SQL injection.
            pstmt.setDouble(1, latitude);
            pstmt.setDouble(2, longitude);
            pstmt.setDate(3, java.sql.Date.valueOf(dateStr));
            
            ResultSet rs = pstmt.executeQuery();
            
            if (rs.next()) {
                return Optional.of(rs.getDouble("temperature"));
            } else {
                return Optional.empty();
            }
        } catch (SQLException e) {
            // Log the error securely. Don't expose detailed errors to the user.
            System.err.println("Database error occurred.");
            return Optional.empty();
        }
        // --- END OF SECURE JDBC EXAMPLE ---
        */

        // **Simulating the secure database lookup using our HashMap**
        String dbKey = String.format("%.4f,%.4f,%s", latitude, longitude, dateStr);
        Double temperature = temperatureDatabase.get(dbKey);
        
        return Optional.ofNullable(temperature);
    }
    
    public static void main(String[] args) {
        System.out.println("--- Running Java Test Cases ---");

        // Test Case 1: Valid request, data exists
        System.out.println("Test Case 1 (Success):");
        temperatureForLocation(40.7128, -74.0060, "2023-10-27")
            .ifPresentOrElse(temp -> System.out.println("  Temperature: " + temp),
                             () -> System.out.println("  Temperature not found."));

        // Test Case 2: Valid request, data does not exist
        System.out.println("\nTest Case 2 (Not Found):");
        temperatureForLocation(10.0, 20.0, "2023-01-01")
            .ifPresentOrElse(temp -> System.out.println("  Temperature: " + temp),
                             () -> System.out.println("  Temperature not found."));

        // Test Case 3: Invalid latitude
        System.out.println("\nTest Case 3 (Invalid Latitude):");
        temperatureForLocation(100.0, -74.0060, "2023-10-27")
            .ifPresentOrElse(temp -> System.out.println("  Temperature: " + temp),
                             () -> System.out.println("  Request failed as expected."));

        // Test Case 4: Invalid longitude
        System.out.println("\nTest Case 4 (Invalid Longitude):");
        temperatureForLocation(40.7128, -200.0, "2023-10-27")
            .ifPresentOrElse(temp -> System.out.println("  Temperature: " + temp),
                             () -> System.out.println("  Request failed as expected."));

        // Test Case 5: Invalid date format
        System.out.println("\nTest Case 5 (Invalid Date):");
        temperatureForLocation(34.0522, -118.2437, "27-10-2023")
            .ifPresentOrElse(temp -> System.out.println("  Temperature: " + temp),
                             () -> System.out.println("  Request failed as expected."));
    }
}