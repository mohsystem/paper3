#include <iostream>
#include <string>
#include <map>
#include <tuple>
#include <optional>

// Simulating a database table from the "Sensor" database.
// Using std::map with a std::tuple as a key is a structured and safe way
// to represent a composite primary key.
std::map<std::tuple<double, double, std::string>, double> sensorDataTable = {
    {{40.7128, -74.0060, "2023-10-27"}, 15.5},
    {{34.0522, -118.2437, "2023-10-27"}, 22.1},
    {{48.8566, 2.3522, "2023-10-26"}, 12.8},
    {{35.6895, 139.6917, "2023-10-27"}, 18.2}
};

/**
 * @brief Simulates fetching temperature for a given location and date.
 *
 * This function demonstrates a secure way to query data. The lookup using a
 * std::tuple key is analogous to using a prepared statement in a real database
 * API (like SOCI, OTL, or libpqxx). It avoids building an SQL query string from
 * user input, which prevents SQL injection.
 *
 * A real implementation with a C++ DB library would look like this:
 *
 *   db_session << "SELECT temperature FROM sensor_data WHERE latitude = :lat"
 *                 " AND longitude = :lon AND date = :dt",
 *                 soci::use(latitude), soci::use(longitude), soci::use(date);
 *
 * This separates the query logic from the data, which is the key to security.
 *
 * @param latitude The latitude of the location.
 * @param longitude The longitude of the location.
 * @param date The date of the reading (e.g., "2023-10-27").
 * @return An std::optional<double> containing the temperature if found, otherwise std::nullopt.
 */
std::optional<double> temperature_for_location(double latitude, double longitude, const std::string& date) {
    if (date.empty()) {
        return std::nullopt;
    }
    auto key = std::make_tuple(latitude, longitude, date);
    auto it = sensorDataTable.find(key);

    if (it != sensorDataTable.end()) {
        return it->second;
    }
    
    return std::nullopt;
}

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Valid and existing data
    std::cout << "Test Case 1 (NYC):" << std::endl;
    if (auto temp = temperature_for_location(40.7128, -74.0060, "2023-10-27")) {
        std::cout << "  Temperature: " << *temp << std::endl;
    } else {
        std::cout << "  Temperature not found." << std::endl;
    }

    // Test Case 2: Valid and existing data
    std::cout << "Test Case 2 (LA):" << std::endl;
    if (auto temp = temperature_for_location(34.0522, -118.2437, "2023-10-27")) {
        std::cout << "  Temperature: " << *temp << std::endl;
    } else {
        std::cout << "  Temperature not found." << std::endl;
    }

    // Test Case 3: Data not found (wrong date)
    std::cout << "Test Case 3 (Paris, wrong date):" << std::endl;
    if (auto temp = temperature_for_location(48.8566, 2.3522, "2023-10-27")) {
        std::cout << "  Temperature: " << *temp << std::endl;
    } else {
        std::cout << "  Temperature not found." << std::endl;
    }

    // Test Case 4: Data not found (non-existent location)
    std::cout << "Test Case 4 (London):" << std::endl;
    if (auto temp = temperature_for_location(51.5074, -0.1278, "2023-10-27")) {
        std::cout << "  Temperature: " << *temp << std::endl;
    } else {
        std::cout << "  Temperature not found." << std::endl;
    }

    // Test Case 5: Invalid input (empty date)
    std::cout << "Test Case 5 (Invalid Input):" << std::endl;
    if (auto temp = temperature_for_location(35.6895, 139.6917, "")) {
        std::cout << "  Temperature: " << *temp << std::endl;
    } else {
        std::cout << "  Temperature not found." << std::endl;
    }

    return 0;
}