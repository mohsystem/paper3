import java.util.*;
import java.time.*;
import java.time.format.DateTimeParseException;

public class Lse87 {

    private static final class SecureDB {
        private final Map<String, Double> data = Collections.synchronizedMap(new HashMap<>());

        private static String key(double lat, double lon, LocalDate date) {
            return String.format(Locale.ROOT, "%.6f|%.6f|%s", lat, lon, date.toString());
        }

        public void insert(double lat, double lon, LocalDate date, double temperature) {
            data.put(key(lat, lon, date), temperature);
        }

        public Double getTemperature(double lat, double lon, LocalDate date) {
            return data.get(key(lat, lon, date));
        }
    }

    private static final SecureDB DB = new SecureDB();

    private static boolean isValidLat(double lat) {
        return !Double.isNaN(lat) && !Double.isInfinite(lat) && lat >= -90.0 && lat <= 90.0;
    }

    private static boolean isValidLon(double lon) {
        return !Double.isNaN(lon) && !Double.isInfinite(lon) && lon >= -180.0 && lon <= 180.0;
    }

    private static LocalDate parseISODate(String dateISO) {
        try {
            return LocalDate.parse(dateISO);
        } catch (DateTimeParseException e) {
            return null;
        }
    }

    // Function: accepts inputs as parameters and returns the temperature; returns NaN if not found or invalid input
    public static double temperature_for_location(double latitude, double longitude, String dateISO) {
        if (!isValidLat(latitude) || !isValidLon(longitude)) {
            return Double.NaN;
        }
        LocalDate date = parseISODate(dateISO);
        if (date == null) {
            return Double.NaN;
        }
        // "Connect" to database (simulated secure in-memory DB) and perform a parameterized-style lookup
        Double result = DB.getTemperature(latitude, longitude, date);
        return result != null ? result : Double.NaN;
    }

    private static void seedSampleData() {
        DB.insert(37.7749, -122.4194, LocalDate.parse("2023-10-01"), 20.5);
        DB.insert(40.7128, -74.0060, LocalDate.parse("2023-10-01"), 18.1);
        DB.insert(51.5074, -0.1278, LocalDate.parse("2023-12-25"), 5.2);
        DB.insert(-33.8688, 151.2093, LocalDate.parse("2024-01-15"), 27.3);
        DB.insert(35.6895, 139.6917, LocalDate.parse("2023-07-20"), 30.4);
    }

    public static void main(String[] args) {
        seedSampleData();

        // 5 test cases
        double t1 = temperature_for_location(37.7749, -122.4194, "2023-10-01"); // expected 20.5
        double t2 = temperature_for_location(35.6895, 139.6917, "2023-07-20");  // expected 30.4
        double t3 = temperature_for_location(37.7749, -122.4194, "2023-10-02"); // not found -> NaN
        double t4 = temperature_for_location(100.0, -122.4194, "2023-10-01");   // invalid lat -> NaN
        double t5 = temperature_for_location(40.7128, -74.0060, "2023-13-01");  // invalid date -> NaN

        System.out.println("Test1: " + t1);
        System.out.println("Test2: " + t2);
        System.out.println("Test3: " + t3);
        System.out.println("Test4: " + t4);
        System.out.println("Test5: " + t5);
    }
}