import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

public class Lse85 {

    // Simulating a database table record
    static class SensorReading {
        double latitude;
        double longitude;
        String date; // Using "YYYY-MM-DD" format
        double temperature;

        public SensorReading(double latitude, double longitude, String date, double temperature) {
            this.latitude = latitude;
            this.longitude = longitude;
            this.date = date;
            this.temperature = temperature;
        }

        // Helper for matching record criteria
        public boolean matches(double lat, double lon, String d) {
            // Using Double.compare for safe double comparison and Objects.equals for null-safe string comparison
            return Double.compare(this.latitude, lat) == 0 &&
                   Double.compare(this.longitude, lon) == 0 &&
                   Objects.equals(this.date, d);
        }
    }

    // In-memory list to simulate the "Sensor" database
    private static final List<SensorReading> sensorDatabase = new ArrayList<>();

    // Static initializer to populate the simulated database.
    // This represents connecting to and retrieving data from the "Sensor" database.
    static {
        sensorDatabase.add(new SensorReading(34.0522, -118.2437, "2023-10-26", 25.5));
        sensorDatabase.add(new SensorReading(40.7128, -74.0060, "2023-10-26", 15.2));
        sensorDatabase.add(new SensorReading(35.6895, 139.6917, "2023-10-27", 18.9));
        sensorDatabase.add(new SensorReading(48.8566, 2.3522, "2023-10-27", 14.1));
        sensorDatabase.add(new SensorReading(-33.8688, 151.2093, "2023-10-28", 22.8));
    }

    /**
     * Simulates getting the temperature for a given location and date from the database.
     * In a real web application, this function would be called when the URL 
     * /api/temperature_for_location is requested.
     *
     * @param latitude  The latitude from the request payload.
     * @param longitude The longitude from the request payload.
     * @param date      The date from the request payload (format "YYYY-MM-DD").
     * @return The temperature as a double, or Double.NaN if no record is found.
     */
    public static double temperature_for_location(double latitude, double longitude, String date) {
        // In a real application, this would be a prepared SQL statement to prevent SQL injection:
        // "SELECT temperature FROM readings WHERE latitude = ? AND longitude = ? AND reading_date = ?"
        for (SensorReading reading : sensorDatabase) {
            if (reading.matches(latitude, longitude, date)) {
                return reading.temperature; // Return the temperature if a match is found
            }
        }
        return Double.NaN; // Return NaN (Not a Number) to indicate no data was found
    }

    public static void main(String[] args) {
        System.out.println("--- Running 5 Test Cases ---");

        // Test Case 1: Success Case 1
        double temp1 = temperature_for_location(34.0522, -118.2437, "2023-10-26");
        System.out.println("Test 1 (LA, 2023-10-26): " + (Double.isNaN(temp1) ? "Not Found" : temp1));

        // Test Case 2: Success Case 2
        double temp2 = temperature_for_location(40.7128, -74.0060, "2023-10-26");
        System.out.println("Test 2 (NYC, 2023-10-26): " + (Double.isNaN(temp2) ? "Not Found" : temp2));
        
        // Test Case 3: Failure Case (Wrong Date)
        double temp3 = temperature_for_location(34.0522, -118.2437, "2023-10-27");
        System.out.println("Test 3 (LA, Wrong Date): " + (Double.isNaN(temp3) ? "Not Found" : temp3));

        // Test Case 4: Failure Case (Wrong Location)
        double temp4 = temperature_for_location(0.0, 0.0, "2023-10-26");
        System.out.println("Test 4 (Null Island): " + (Double.isNaN(temp4) ? "Not Found" : temp4));

        // Test Case 5: Success Case 3
        double temp5 = temperature_for_location(35.6895, 139.6917, "2023-10-27");
        System.out.println("Test 5 (Tokyo, 2023-10-27): " + (Double.isNaN(temp5) ? "Not Found" : temp5));
    }
}