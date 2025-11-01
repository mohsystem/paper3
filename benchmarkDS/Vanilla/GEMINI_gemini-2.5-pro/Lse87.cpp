#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <iomanip>

// Mock WeatherData struct to represent a database record
struct WeatherData {
    double latitude;
    double longitude;
    std::string date; // YYYY-MM-DD
    double temperature;
};

// Mock database table
const std::vector<WeatherData> mockDatabase = {
    {40.7128, -74.0060, "2023-10-27", 15.5},
    {34.0522, -118.2437, "2023-10-27", 22.0},
    {48.8566, 2.3522, "2023-10-28", 12.3},
    {35.6895, 139.6917, "2023-10-28", 18.7}
};

/**
 * Simulates fetching temperature for a given location and date from a database.
 * In a real application, this would involve a database connection and SQL query.
 *
 * @param latitude The latitude of the location.
 * @param longitude The longitude of the location.
 * @param date The date for the temperature reading (format YYYY-MM-DD).
 * @return An optional containing the temperature if found, otherwise an empty optional.
 */
std::optional<double> temperature_for_location(double latitude, double longitude, const std::string& date) {
    // Simulate "SELECT temperature FROM weather_data WHERE ..."
    for (const auto& record : mockDatabase) {
        if (record.latitude == latitude && record.longitude == longitude && record.date == date) {
            return record.temperature;
        }
    }
    return std::nullopt; // No record found
}

int main() {
    // Mock test case struct
    struct TestCase {
        double lat;
        double lon;
        std::string date;
    };

    // 5 Test Cases
    std::vector<TestCase> testCases = {
        {40.7128, -74.0060, "2023-10-27"},   // Case 1: Exists
        {34.0522, -118.2437, "2023-10-27"},  // Case 2: Exists
        {48.8566, 2.3522, "2023-10-28"},     // Case 3: Exists
        {0.0, 0.0, "2023-10-27"},            // Case 4: Does not exist
        {35.6895, 139.6917, "2023-10-29"}    // Case 5: Exists but wrong date
    };
    
    std::cout << "--- C++ Test Cases ---" << std::endl;
    int testNum = 1;
    for (const auto& tc : testCases) {
        auto temp = temperature_for_location(tc.lat, tc.lon, tc.date);

        std::cout << "Test Case " << testNum++ 
                  << " (Lat: " << std::fixed << std::setprecision(4) << tc.lat 
                  << ", Lon: " << tc.lon 
                  << ", Date: " << tc.date << "): ";

        if (temp.has_value()) {
            std::cout << "Temperature = " << std::fixed << std::setprecision(1) << temp.value() << " C" << std::endl;
        } else {
            std::cout << "Temperature data not found." << std::endl;
        }
    }

    return 0;
}