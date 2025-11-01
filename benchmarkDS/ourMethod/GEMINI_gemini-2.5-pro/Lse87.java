import java.time.LocalDate;
import java.time.format.DateTimeParseException;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.regex.Pattern;

public class Lse87 {

    // Mock database connection and data
    private static final Map<LocationDate, Double> mockDatabase = new HashMap<>();

    static {
        mockDatabase.put(new LocationDate(34.0522, -118.2437, "2023-10-26"), 75.5);
        mockDatabase.put(new LocationDate(40.7128, -74.0060, "2023-10-26"), 62.1);
        mockDatabase.put(new LocationDate(48.8566, 2.3522, "2023-10-27"), 58.3);
    }

    // A simple record-like class to use as a key in our mock database
    private static class LocationDate {
        double latitude;
        double longitude;
        String date;

        public LocationDate(double latitude, double longitude, String date) {
            this.latitude = latitude;
            this.longitude = longitude;
            this.date = date;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            LocationDate that = (LocationDate) o;
            return Double.compare(that.latitude, latitude) == 0 &&
                   Double.compare(that.longitude, longitude) == 0 &&
                   Objects.equals(date, that.date);
        }

        @Override
        public int hashCode() {
            return Objects.hash(latitude, longitude, date);
        }
    }

    /**
     * Simulates a database query to get the temperature for a given location and date.
     * This function demonstrates input validation and the use of parameterized queries.
     *
     * @param latitude  The latitude of the location (-90.0 to 90.0).
     * @param longitude The longitude of the location (-180.0 to 180.0).
     * @param date      The date in 'YYYY-MM-DD' format.
     * @return An Optional containing the temperature, or an empty Optional if not found or inputs are invalid.
     */
    public static Optional<Double> temperature_for_location(double latitude, double longitude, String date) {
        // 1. Input Validation (Rule #1)
        if (latitude < -90.0 || latitude > 90.0) {
            System.err.println("Error: Invalid latitude value.");
            return Optional.empty();
        }
        if (longitude < -180.0 || longitude > 180.0) {
            System.err.println("Error: Invalid longitude value.");
            return Optional.empty();
        }
        if (date == null) {
             System.err.println("Error: Date string cannot be null.");
             return Optional.empty();
        }
        try {
            LocalDate.parse(date); // Validates date format "YYYY-MM-DD"
        } catch (DateTimeParseException e) {
            System.err.println("Error: Invalid date format. Expected 'YYYY-MM-DD'.");
            return Optional.empty();
        }

        // 2. Simulate database connection and parameterized query (prevents SQL injection)
        System.out.println("Simulating database connection...");
        // In a real application, this would use a PreparedStatement
        // String sql = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ?;";
        // PreparedStatement stmt = connection.prepareStatement(sql);
        // stmt.setDouble(1, latitude);
        // stmt.setDouble(2, longitude);
        // stmt.setDate(3, java.sql.Date.valueOf(date));
        // ResultSet rs = stmt.executeQuery();
        
        System.out.println("Executing mock query for lat=" + latitude + ", lon=" + longitude + ", date=" + date);
        
        LocationDate key = new LocationDate(latitude, longitude, date);
        Double temperature = mockDatabase.get(key);

        System.out.println("Closing database connection...");

        return Optional.ofNullable(temperature);
    }

    public static void main(String[] args) {
        // Test Cases
        System.out.println("--- Test Cases ---");

        // Test Case 1: Valid request
        System.out.println("\n1. Valid Request:");
        temperature_for_location(34.0522, -118.2437, "2023-10-26")
                .ifPresentOrElse(
                        temp -> System.out.println("Success! Temperature: " + temp),
                        () -> System.out.println("Failed: Temperature not found.")
                );

        // Test Case 2: Data not in database
        System.out.println("\n2. Data Not Found:");
        temperature_for_location(0.0, 0.0, "2023-01-01")
                .ifPresentOrElse(
                        temp -> System.out.println("Success! Temperature: " + temp),
                        () -> System.out.println("Failed as expected: Temperature not found.")
                );

        // Test Case 3: Invalid latitude
        System.out.println("\n3. Invalid Latitude:");
        temperature_for_location(100.0, -74.0060, "2023-10-26")
                .ifPresentOrElse(
                        temp -> System.out.println("Success! Temperature: " + temp),
                        () -> System.out.println("Failed as expected: Invalid input.")
                );

        // Test Case 4: Invalid date format
        System.out.println("\n4. Invalid Date Format:");
        temperature_for_location(40.7128, -74.0060, "26-10-2023")
                .ifPresentOrElse(
                        temp -> System.out.println("Success! Temperature: " + temp),
                        () -> System.out.println("Failed as expected: Invalid input.")
                );

        // Test Case 5: Valid request for Paris
        System.out.println("\n5. Valid Request (Paris):");
        temperature_for_location(48.8566, 2.3522, "2023-10-27")
                .ifPresentOrElse(
                        temp -> System.out.println("Success! Temperature: " + temp),
                        () -> System.out.println("Failed: Temperature not found.")
                );
    }
}