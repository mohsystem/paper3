#include <iostream>
#include <vector>
#include <string>
#include <cmath> // For std::fabs and NAN
#include <iomanip> // For std::fixed and std::setprecision

// Simulating a database table record
struct SensorReading {
    double latitude;
    double longitude;
    std::string date; // "YYYY-MM-DD" format
    double temperature;
};

// In-memory vector to simulate the "Sensor" database.
const std::vector<SensorReading> sensorDatabase = {
    {34.0522, -118.2437, "2023-10-26", 25.5},
    {40.7128, -74.0060, "2023-10-26", 15.2},
    {35.6895, 139.6917, "2023-10-27", 18.9},
    {48.8566, 2.3522, "2023-10-27", 14.1},
    {-33.8688, 151.2093, "2023-10-28", 22.8}
};

// Helper for comparing doubles with a small tolerance
bool areDoublesEqual(double a, double b, double epsilon = 1e-9) {
    return std::fabs(a - b) < epsilon;
}

/**
 * @brief Simulates getting the temperature for a given location and date from the database.
 * 
 * In a real web application, this function would be called when the URL 
 * /api/temperature_for_location is requested.
 * 
 * @param latitude The latitude from the request payload.
 * @param longitude The longitude from the request payload.
 * @param date The date from the request payload (format "YYYY-MM-DD").
 * @return The temperature as a double, or NAN if no record is found.
 */
double temperature_for_location(double latitude, double longitude, const std::string& date) {
    // In a real application, this would use a database library with prepared 
    // statements to prevent SQL injection.
    for (const auto& reading : sensorDatabase) {
        if (areDoublesEqual(reading.latitude, latitude) &&
            areDoublesEqual(reading.longitude, longitude) &&
            reading.date == date) {
            return reading.temperature; // Return the temperature if a match is found
        }
    }
    return NAN; // Return Not-a-Number to indicate no data was found
}

void run_test(int testNum, const std::string& testName, double lat, double lon, const std::string& date) {
    double temp = temperature_for_location(lat, lon, date);
    std::cout << "Test " << testNum << " (" << testName << "): ";
    if (std::isnan(temp)) {
        std::cout << "Not Found" << std::endl;
    } else {
        std::cout << std::fixed << std::setprecision(1) << temp << std::endl;
    }
}

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Success Case 1
    run_test(1, "LA, 2023-10-26", 34.0522, -118.2437, "2023-10-26");

    // Test Case 2: Success Case 2
    run_test(2, "NYC, 2023-10-26", 40.7128, -74.0060, "2023-10-26");
    
    // Test Case 3: Failure Case (Wrong Date)
    run_test(3, "LA, Wrong Date", 34.0522, -118.2437, "2023-10-27");

    // Test Case 4: Failure Case (Wrong Location)
    run_test(4, "Null Island", 0.0, 0.0, "2023-10-26");

    // Test Case 5: Success Case 3
    run_test(5, "Tokyo, 2023-10-27", 35.6895, 139.6917, "2023-10-27");
    
    return 0;
}