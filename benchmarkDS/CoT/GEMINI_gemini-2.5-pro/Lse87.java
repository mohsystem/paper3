import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

// Main class name as per instruction
public class Lse87 {

    // Mock Database class to simulate a real database for this example.
    private static class MockDatabase {
        private static final Map<LocationDate, Double> temperatures = new HashMap<>();
        
        // A simple composite key for our mock data
        private static class LocationDate {
            double lat, lon;
            String date;
            LocationDate(double lat, double lon, String date) { this.lat = lat; this.lon = lon; this.date = date; }
            @Override
            public boolean equals(Object o) {
                if (this == o) return true;
                if (o == null || getClass() != o.getClass()) return false;
                LocationDate that = (LocationDate) o;
                return Double.compare(that.lat, lat) == 0 && Double.compare(that.lon, lon) == 0 && Objects.equals(date, that.date);
            }
            @Override
            public int hashCode() { return Objects.hash(lat, lon, date); }
        }

        static {
            // Populate with some data
            temperatures.put(new LocationDate(40.71, -74.00, "2023-10-26"), 15.5);
            temperatures.put(new LocationDate(34.05, -118.24, "2023-10-26"), 22.0);
            temperatures.put(new LocationDate(48.85, 2.35, "2023-10-27"), 12.3);
            temperatures.put(new LocationDate(35.68, 139.69, "2023-10-27"), 18.1);
        }

        public static Double query(double lat, double lon, String date) {
            System.out.println("Mock DB: Executing secure query for lat=" + lat + ", lon=" + lon + ", date=" + date);
            return temperatures.get(new LocationDate(lat, lon, date));
        }
    }

    /**
     * Connects to a database and securely queries for the temperature
     * at a given latitude, longitude, and date.
     *
     * @param latitude  The latitude of the location.
     * @param longitude The longitude of the location.
     * @param date      The date for the temperature reading (e.g., "YYYY-MM-DD").
     * @return The temperature as a Double, or null if not found or an error occurs.
     */
    public static Double temperature_for_location(double latitude, double longitude, String date) {
        // In a real application, connection details would come from a config file.
        // String dbUrl = "jdbc:postgresql://localhost:5432/weatherdb";
        // String user = "user";
        // String password = "password";

        // The SQL query with '?' placeholders to prevent SQL injection.
        String sql = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND aqi_date = ?";

        // This block demonstrates the secure JDBC pattern.
        // We will use our MockDatabase instead of a real connection for this example.
        System.out.println("\n--- Java/JDBC Secure Pattern Demonstration ---");
        System.out.println("Preparing statement: " + sql);
        System.out.println("Binding parameter 1 (latitude): " + latitude);
        System.out.println("Binding parameter 2 (longitude): " + longitude);
        System.out.println("Binding parameter 3 (date): " + date);
        
        /*
        // --- Real JDBC Code would look like this ---
        try (Connection conn = DriverManager.getConnection(dbUrl, user, password);
             PreparedStatement pstmt = conn.prepareStatement(sql)) {

            pstmt.setDouble(1, latitude);
            pstmt.setDouble(2, longitude);
            pstmt.setDate(3, java.sql.Date.valueOf(date));

            try (ResultSet rs = pstmt.executeQuery()) {
                if (rs.next()) {
                    return rs.getDouble("temperature");
                }
            }
        } catch (SQLException e) {
            // In a real app, log the error
            e.printStackTrace();
            return null;
        }
        return null; // Not found
        */

        // Using the mock database for a self-contained runnable example.
        return MockDatabase.query(latitude, longitude, date);
    }

    public static void main(String[] args) {
        // 5 Test Cases
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Valid and existing data
        Double temp1 = temperature_for_location(40.71, -74.00, "2023-10-26");
        System.out.println("Result 1: " + (temp1 != null ? temp1 : "Not Found"));

        // Test Case 2: Valid and existing data
        Double temp2 = temperature_for_location(48.85, 2.35, "2023-10-27");
        System.out.println("Result 2: " + (temp2 != null ? temp2 : "Not Found"));

        // Test Case 3: Data that does not exist
        Double temp3 = temperature_for_location(0.0, 0.0, "2000-01-01");
        System.out.println("Result 3: " + (temp3 != null ? temp3 : "Not Found"));

        // Test Case 4: Malicious-looking input (which is safely handled by PreparedStatement)
        Double temp4 = temperature_for_location(0, 0, "2023-10-27' OR '1'='1");
        System.out.println("Result 4: " + (temp4 != null ? temp4 : "Not Found"));

        // Test Case 5: Another valid case
        Double temp5 = temperature_for_location(35.68, 139.69, "2023-10-27");
        System.out.println("Result 5: " + (temp5 != null ? temp5 : "Not Found"));
    }
}