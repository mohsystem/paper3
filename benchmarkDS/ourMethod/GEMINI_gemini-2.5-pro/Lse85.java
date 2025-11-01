import java.util.regex.Pattern;

public class Lse85 {

    // A mock database query function. In a real application, this would connect
    // to a database like PostgreSQL, MySQL, etc., using JDBC.
    // It would use a PreparedStatement to prevent SQL injection.
    private static double queryMockDatabase(double latitude, double longitude, String date) {
        // This is a placeholder logic to generate a deterministic "temperature"
        // based on the inputs, simulating a database lookup.
        // The database would be named "Sensor".
        System.out.println("Connecting to database 'Sensor'...");
        System.out.printf("Querying for latitude=%.4f, longitude=%.4f, date=%s%n", latitude, longitude, date);
        
        // Simple deterministic calculation for demonstration
        double baseTemp = (latitude + 45.0) / 10.0; // Base temp related to latitude
        int dateHash = date.hashCode();
        double dailyVariation = (dateHash % 200) / 10.0 - 10.0; // Variation from -10 to +10
        
        return baseTemp + dailyVariation;
    }

    /**
     * Simulates an API endpoint function that retrieves temperature for a given location and date.
     * It validates inputs and fetches data from a mock database.
     *
     * @param latitude The geographical latitude (-90.0 to 90.0).
     * @param longitude The geographical longitude (-180.0 to 180.0).
     * @param date The date in YYYY-MM-DD format.
     * @return The temperature in Celsius.
     * @throws IllegalArgumentException if the inputs are invalid.
     */
    public static double temperature_for_location(double latitude, double longitude, String date) {
        // Rule #1: Ensure all input is validated.
        if (latitude < -90.0 || latitude > 90.0) {
            throw new IllegalArgumentException("Invalid latitude. Must be between -90.0 and 90.0.");
        }
        if (longitude < -180.0 || longitude > 180.0) {
            throw new IllegalArgumentException("Invalid longitude. Must be between -180.0 and 180.0.");
        }

        // Validate date format (YYYY-MM-DD)
        if (date == null || !Pattern.matches("^\\d{4}-\\d{2}-\\d{2}$", date)) {
            throw new IllegalArgumentException("Invalid date format. Expected YYYY-MM-DD.");
        }

        // In a real application, database connection logic would be here.
        // We are simulating it with a mock function.
        return queryMockDatabase(latitude, longitude, date);
    }

    public static void main(String[] args) {
        // 5 Test Cases
        System.out.println("--- Running Test Cases ---");

        // Test Case 1: Valid input
        try {
            double temp1 = temperature_for_location(40.7128, -74.0060, "2023-10-27");
            System.out.printf("Test 1 PASSED: Temperature for NYC on 2023-10-27 is %.2f°C%n", temp1);
        } catch (IllegalArgumentException e) {
            System.out.println("Test 1 FAILED: " + e.getMessage());
        }
        System.out.println();

        // Test Case 2: Another valid input
        try {
            double temp2 = temperature_for_location(34.0522, -118.2437, "2024-01-15");
            System.out.printf("Test 2 PASSED: Temperature for LA on 2024-01-15 is %.2f°C%n", temp2);
        } catch (IllegalArgumentException e) {
            System.out.println("Test 2 FAILED: " + e.getMessage());
        }
        System.out.println();

        // Test Case 3: Invalid latitude
        try {
            temperature_for_location(100.0, -74.0060, "2023-10-27");
            System.out.println("Test 3 FAILED: Exception was not thrown for invalid latitude.");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 3 PASSED: Caught expected exception: " + e.getMessage());
        }
        System.out.println();

        // Test Case 4: Invalid date format
        try {
            temperature_for_location(40.7128, -74.0060, "27-10-2023");
            System.out.println("Test 4 FAILED: Exception was not thrown for invalid date format.");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 4 PASSED: Caught expected exception: " + e.getMessage());
        }
        System.out.println();

        // Test Case 5: Null date
        try {
            temperature_for_location(40.7128, -74.0060, null);
            System.out.println("Test 5 FAILED: Exception was not thrown for null date.");
        } catch (IllegalArgumentException e) {
            System.out.println("Test 5 PASSED: Caught expected exception: " + e.getMessage());
        }
    }
}