#include <iostream>
#include <string>
#include <optional>
#include <map>
#include <regex>
#include <tuple>

// Mock database using a map
// The key is a tuple of (latitude, longitude, date) for easy lookup
static std::map<std::tuple<double, double, std::string>, double> mock_database = {
    {{34.0522, -118.2437, "2023-10-26"}, 75.5},
    {{40.7128, -74.0060, "2023-10-26"}, 62.1},
    {{48.8566, 2.3522, "2023-10-27"}, 58.3}
};

/**
 * @brief Simulates a database query to get the temperature for a given location and date.
 * This function demonstrates input validation and the use of parameterized queries.
 *
 * @param latitude The latitude of the location (-90.0 to 90.0).
 * @param longitude The longitude of the location (-180.0 to 180.0).
 * @param date The date in 'YYYY-MM-DD' format.
 * @return An optional containing the temperature, or nullopt if not found or inputs are invalid.
 */
std::optional<double> temperature_for_location(double latitude, double longitude, const std::string& date) {
    // 1. Input Validation (Rule #1)
    if (latitude < -90.0 || latitude > 90.0) {
        std::cerr << "Error: Invalid latitude value." << std::endl;
        return std::nullopt;
    }
    if (longitude < -180.0 || longitude > 180.0) {
        std::cerr << "Error: Invalid longitude value." << std::endl;
        return std::nullopt;
    }

    // Date format validation using regex
    const std::regex date_pattern(R"(\d{4}-\d{2}-\d{2})");
    if (!std::regex_match(date, date_pattern)) {
        std::cerr << "Error: Invalid date format. Expected 'YYYY-MM-DD'." << std::endl;
        return std::nullopt;
    }

    // 2. Simulate database connection and parameterized query (prevents SQL injection)
    std::cout << "Simulating database connection..." << std::endl;
    // In a real application with a library like SOCI or ODB:
    // session sql(oracle, "service=... user=... password=...");
    // double temperature;
    // sql << "SELECT temperature FROM weather_data WHERE latitude = :lat AND longitude = :lon AND adate = :dt",
    //      into(temperature), use(latitude, "lat"), use(longitude, "lon"), use(date, "dt");

    std::cout << "Executing mock query for lat=" << latitude << ", lon=" << longitude << ", date=" << date << std::endl;

    auto key = std::make_tuple(latitude, longitude, date);
    auto it = mock_database.find(key);

    std::cout << "Closing database connection..." << std::endl;

    if (it != mock_database.end()) {
        return it->second;
    }

    return std::nullopt;
}

void run_test(const std::string& test_name, double lat, double lon, const std::string& date) {
    std::cout << "\n" << test_name << ":" << std::endl;
    std::optional<double> temp = temperature_for_location(lat, lon, date);
    if (temp.has_value()) {
        std::cout << "Success! Temperature: " << temp.value() << std::endl;
    } else {
        std::cout << "Failed as expected or data not found." << std::endl;
    }
}

int main() {
    std::cout << "--- Test Cases ---" << std::endl;

    // Test Case 1: Valid request
    run_test("1. Valid Request", 34.0522, -118.2437, "2023-10-26");

    // Test Case 2: Data not in database
    run_test("2. Data Not Found", 0.0, 0.0, "2023-01-01");

    // Test Case 3: Invalid latitude
    run_test("3. Invalid Latitude", 100.0, -74.0060, "2023-10-26");

    // Test Case 4: Invalid date format
    run_test("4. Invalid Date Format", 40.7128, -74.0060, "October 26, 2023");

    // Test Case 5: Valid request for Paris
    run_test("5. Valid Request (Paris)", 48.8566, 2.3522, "2023-10-27");
    
    return 0;
}