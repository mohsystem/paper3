#include <iostream>
#include <string>
#include <stdexcept>
#include <regex>
#include <vector>
#include <utility>

// A mock database query function. In a real application, this would use a library
// like libpqxx (for PostgreSQL) or MySQL C++ Connector.
// It would use prepared statements to prevent SQL injection.
double queryMockDatabase(double latitude, double longitude, const std::string& date) {
    // This is a placeholder logic to generate a deterministic "temperature"
    // based on the inputs, simulating a database lookup.
    // The database would be named "Sensor".
    std::cout << "Connecting to database 'Sensor'..." << std::endl;
    std::cout << "Querying for latitude=" << latitude << ", longitude=" << longitude << ", date=" << date << std::endl;

    // Simple deterministic calculation for demonstration
    double baseTemp = (latitude + 45.0) / 10.0; // Base temp related to latitude
    std::hash<std::string> hasher;
    double dailyVariation = (hasher(date) % 200) / 10.0 - 10.0; // Variation from -10 to +10

    return baseTemp + dailyVariation;
}

/**
 * @brief Simulates an API endpoint function that retrieves temperature for a given location and date.
 * It validates inputs and fetches data from a mock database.
 * 
 * @param latitude The geographical latitude (-90.0 to 90.0).
 * @param longitude The geographical longitude (-180.0 to 180.0).
 * @param date The date in YYYY-MM-DD format.
 * @return The temperature in Celsius.
 * @throws std::invalid_argument if the inputs are invalid.
 */
double temperature_for_location(double latitude, double longitude, const std::string& date) {
    // Rule #1: Ensure all input is validated.
    if (latitude < -90.0 || latitude > 90.0) {
        throw std::invalid_argument("Invalid latitude. Must be between -90.0 and 90.0.");
    }
    if (longitude < -180.0 || longitude > 180.0) {
        throw std::invalid_argument("Invalid longitude. Must be between -180.0 and 180.0.");
    }

    // Validate date format (YYYY-MM-DD) using regex
    const std::regex date_pattern("^\\d{4}-\\d{2}-\\d{2}$");
    if (!std::regex_match(date, date_pattern)) {
        throw std::invalid_argument("Invalid date format. Expected YYYY-MM-DD.");
    }

    // In a real application, database connection logic would be here.
    return queryMockDatabase(latitude, longitude, date);
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid input
    try {
        double temp1 = temperature_for_location(40.7128, -74.0060, "2023-10-27");
        std::cout << "Test 1 PASSED: Temperature for NYC on 2023-10-27 is " << temp1 << "°C" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Test 1 FAILED: " << e.what() << std::endl;
    }
    std::cout << std::endl;

    // Test Case 2: Another valid input
    try {
        double temp2 = temperature_for_location(34.0522, -118.2437, "2024-01-15");
        std::cout << "Test 2 PASSED: Temperature for LA on 2024-01-15 is " << temp2 << "°C" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Test 2 FAILED: " << e.what() << std::endl;
    }
    std::cout << std::endl;

    // Test Case 3: Invalid longitude
    try {
        temperature_for_location(40.7128, 200.0, "2023-10-27");
        std::cerr << "Test 3 FAILED: Exception was not thrown for invalid longitude." << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test 3 PASSED: Caught expected exception: " << e.what() << std::endl;
    }
    std::cout << std::endl;
    
    // Test Case 4: Invalid date format
    try {
        temperature_for_location(40.7128, -74.0060, "2023/10/27");
        std::cerr << "Test 4 FAILED: Exception was not thrown for invalid date format." << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test 4 PASSED: Caught expected exception: " << e.what() << std::endl;
    }
    std::cout << std::endl;

    // Test Case 5: Empty date
    try {
        temperature_for_location(40.7128, -74.0060, "");
        std::cerr << "Test 5 FAILED: Exception was not thrown for empty date." << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Test 5 PASSED: Caught expected exception: " << e.what() << std::endl;
    }

    return 0;
}