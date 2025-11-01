import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

public class Lse85 {

    // Simulating a database table called 'Sensor' in memory
    private static final Map<String, Double> sensorDatabase = new HashMap<>();

    // Static initializer to populate the 'database' with some data
    static {
        // Key is "latitude:longitude:date"
        sensorDatabase.put("34.0522:-118.2437:2023-10-26", 25.5);
        sensorDatabase.put("40.7128:-74.0060:2023-10-26", 15.2);
        sensorDatabase.put("48.8566:2.3522:2023-10-27", 12.8);
        sensorDatabase.put("35.6895:139.6917:2023-10-26", 18.0);
        sensorDatabase.put("-33.8688:151.2093:2023-10-28", 22.3);
    }

    /**
     * This function simulates an API endpoint call.
     * It connects to the 'database' and gets the temperature for the given location and date.
     *
     * @param latitude  The latitude of the location.
     * @param longitude The longitude of the location.
     * @param date      The date in "YYYY-MM-DD" format.
     * @return An Optional containing the temperature if found, otherwise an empty Optional.
     */
    public static Optional<Double> temperature_for_location(double latitude, double longitude, String date) {
        String key = String.format("%.4f:%.4f:%s", latitude, longitude, date);
        Double temperature = sensorDatabase.get(key);
        return Optional.ofNullable(temperature);
    }

    public static void main(String[] args) {
        // --- 5 Test Cases ---

        // Test Case 1: Existing data
        System.out.println("Test Case 1: (LA, 2023-10-26)");
        temperature_for_location(34.0522, -118.2437, "2023-10-26")
                .ifPresentOrElse(
                        temp -> System.out.println("  -> Temperature: " + temp),
                        () -> System.out.println("  -> Temperature not found.")
                );

        // Test Case 2: Existing data
        System.out.println("\nTest Case 2: (NYC, 2023-10-26)");
        temperature_for_location(40.7128, -74.0060, "2023-10-26")
                .ifPresentOrElse(
                        temp -> System.out.println("  -> Temperature: " + temp),
                        () -> System.out.println("  -> Temperature not found.")
                );

        // Test Case 3: Existing data
        System.out.println("\nTest Case 3: (Paris, 2023-10-27)");
        temperature_for_location(48.8566, 2.3522, "2023-10-27")
                .ifPresentOrElse(
                        temp -> System.out.println("  -> Temperature: " + temp),
                        () -> System.out.println("  -> Temperature not found.")
                );

        // Test Case 4: Non-existing date
        System.out.println("\nTest Case 4: (Tokyo, Non-existing date)");
        temperature_for_location(35.6895, 139.6917, "2023-10-27")
                .ifPresentOrElse(
                        temp -> System.out.println("  -> Temperature: " + temp),
                        () -> System.out.println("  -> Temperature not found.")
                );

        // Test Case 5: Non-existing location
        System.out.println("\nTest Case 5: (Non-existing location)");
        temperature_for_location(0.0, 0.0, "2023-10-26")
                .ifPresentOrElse(
                        temp -> System.out.println("  -> Temperature: " + temp),
                        () -> System.out.println("  -> Temperature not found.")
                );
    }
}