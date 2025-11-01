import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

class Lse87 {

    // Mock WeatherData class to represent a database record
    static class WeatherData {
        double latitude;
        double longitude;
        String date; // YYYY-MM-DD
        double temperature;

        public WeatherData(double latitude, double longitude, String date, double temperature) {
            this.latitude = latitude;
            this.longitude = longitude;
            this.date = date;
            this.temperature = temperature;
        }
    }

    // Mock database table
    private static final List<WeatherData> mockDatabase = new ArrayList<>();

    static {
        mockDatabase.add(new WeatherData(40.7128, -74.0060, "2023-10-27", 15.5));
        mockDatabase.add(new WeatherData(34.0522, -118.2437, "2023-10-27", 22.0));
        mockDatabase.add(new WeatherData(48.8566, 2.3522, "2023-10-28", 12.3));
        mockDatabase.add(new WeatherData(35.6895, 139.6917, "2023-10-28", 18.7));
    }

    /**
     * Simulates fetching temperature for a given location and date from a database.
     * In a real application, this would involve a database connection and SQL query.
     *
     * @param latitude  The latitude of the location.
     * @param longitude The longitude of the location.
     * @param date      The date for the temperature reading (format YYYY-MM-DD).
     * @return The temperature as a Double, or null if not found.
     */
    public static Double temperature_for_location(double latitude, double longitude, String date) {
        // Simulate "SELECT temperature FROM weather_data WHERE ..."
        for (WeatherData record : mockDatabase) {
            if (record.latitude == latitude && record.longitude == longitude && Objects.equals(record.date, date)) {
                return record.temperature;
            }
        }
        return null; // No record found
    }

    public static void main(String[] args) {
        // 5 Test Cases
        Object[][] testCases = {
            {40.7128, -74.0060, "2023-10-27"}, // Case 1: Exists
            {34.0522, -118.2437, "2023-10-27"}, // Case 2: Exists
            {48.8566, 2.3522, "2023-10-28"},    // Case 3: Exists
            {0.0, 0.0, "2023-10-27"},           // Case 4: Does not exist
            {35.6895, 139.6917, "2023-10-29"}   // Case 5: Exists but wrong date
        };

        System.out.println("--- Java Test Cases ---");
        int testNum = 1;
        for (Object[] testCase : testCases) {
            double lat = (double) testCase[0];
            double lon = (double) testCase[1];
            String date = (String) testCase[2];

            Double temp = temperature_for_location(lat, lon, date);

            System.out.printf("Test Case %d (Lat: %.4f, Lon: %.4f, Date: %s): ", testNum++, lat, lon, date);
            if (temp != null) {
                System.out.printf("Temperature = %.1f C\n", temp);
            } else {
                System.out.println("Temperature data not found.");
            }
        }
    }
}