import java.util.HashMap;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

// Main class name as per instruction
class Lse85 {

    // Simulating a database table from the "Sensor" database.
    // In a real application, this would be a connection to a database like PostgreSQL, MySQL, etc.
    // The key is a composite object representing the unique location and date.
    private static final Map<LocationDateKey, Double> sensorDataTable = new HashMap<>();

    // A simple composite key class to represent the primary key of our simulated table.
    // This is good practice to avoid string concatenation for keys.
    private static class LocationDateKey {
        private final double latitude;
        private final double longitude;
        private final String date;

        public LocationDateKey(double latitude, double longitude, String date) {
            this.latitude = latitude;
            this.longitude = longitude;
            this.date = date;
        }

        @Override
        public boolean equals(Object o) {
            if (this == o) return true;
            if (o == null || getClass() != o.getClass()) return false;
            LocationDateKey that = (LocationDateKey) o;
            return Double.compare(that.latitude, latitude) == 0 &&
                   Double.compare(that.longitude, longitude) == 0 &&
                   date.equals(that.date);
        }

        @Override
        public int hashCode() {
            return Objects.hash(latitude, longitude, date);
        }
    }

    /**
     * Simulates fetching temperature for a given location and date.
     * This function demonstrates a secure way to query data by avoiding string concatenation
     * and using a structured key for lookup, which is analogous to using a
     * PreparedStatement in JDBC to prevent SQL injection.
     *
     * In a real JDBC implementation:
     * String query = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ?";
     * PreparedStatement pstmt = connection.prepareStatement(query);
     * pstmt.setDouble(1, latitude);
     * pstmt.setDouble(2, longitude);
     * pstmt.setString(3, date);
     * ResultSet rs = pstmt.executeQuery();
     * // ... process results
     *
     * @param latitude  The latitude of the location.
     * @param longitude The longitude of the location.
     * @param date      The date of the reading (e.g., "2023-10-27").
     * @return An Optional containing the temperature if found, otherwise an empty Optional.
     */
    public Optional<Double> temperature_for_location(double latitude, double longitude, String date) {
        if (date == null || date.trim().isEmpty()) {
            return Optional.empty();
        }
        LocationDateKey key = new LocationDateKey(latitude, longitude, date);
        return Optional.ofNullable(sensorDataTable.get(key));
    }

    // Static initializer to populate the mock database
    static {
        sensorDataTable.put(new LocationDateKey(40.7128, -74.0060, "2023-10-27"), 15.5);
        sensorDataTable.put(new LocationDateKey(34.0522, -118.2437, "2023-10-27"), 22.1);
        sensorDataTable.put(new LocationDateKey(48.8566, 2.3522, "2023-10-26"), 12.8);
        sensorDataTable.put(new LocationDateKey(35.6895, 139.6917, "2023-10-27"), 18.2);
    }

    public static void main(String[] args) {
        Lse85 app = new Lse85();

        // --- Test Cases ---
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Valid and existing data
        System.out.println("Test Case 1 (NYC):");
        app.temperature_for_location(40.7128, -74.0060, "2023-10-27")
           .ifPresentOrElse(temp -> System.out.println("  Temperature: " + temp),
                            () -> System.out.println("  Temperature not found."));

        // Test Case 2: Valid and existing data
        System.out.println("Test Case 2 (LA):");
        app.temperature_for_location(34.0522, -118.2437, "2023-10-27")
           .ifPresentOrElse(temp -> System.out.println("  Temperature: " + temp),
                            () -> System.out.println("  Temperature not found."));

        // Test Case 3: Data not found (wrong date)
        System.out.println("Test Case 3 (Paris, wrong date):");
        app.temperature_for_location(48.8566, 2.3522, "2023-10-27")
           .ifPresentOrElse(temp -> System.out.println("  Temperature: " + temp),
                            () -> System.out.println("  Temperature not found."));

        // Test Case 4: Data not found (non-existent location)
        System.out.println("Test Case 4 (London):");
        app.temperature_for_location(51.5074, -0.1278, "2023-10-27")
           .ifPresentOrElse(temp -> System.out.println("  Temperature: " + temp),
                            () -> System.out.println("  Temperature not found."));

        // Test Case 5: Invalid input (null date)
        System.out.println("Test Case 5 (Invalid Input):");
        app.temperature_for_location(35.6895, 139.6917, null)
           .ifPresentOrElse(temp -> System.out.println("  Temperature: " + temp),
                            () -> System.out.println("  Temperature not found."));
    }
}