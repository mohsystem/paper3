#include <stdio.h>
#include <string.h>
#include <math.h>    // For fabs and NAN
#include <stdbool.h> // For bool type

// Simulating a database table record
struct SensorReading {
    double latitude;
    double longitude;
    char date[11]; // "YYYY-MM-DD" + null terminator
    double temperature;
};

// In-memory array to simulate the "Sensor" database.
#define DATABASE_SIZE 5
struct SensorReading sensorDatabase[DATABASE_SIZE] = {
    {34.0522, -118.2437, "2023-10-26", 25.5},
    {40.7128, -74.0060, "2023-10-26", 15.2},
    {35.6895, 139.6917, "2023-10-27", 18.9},
    {48.8566, 2.3522, "2023-10-27", 14.1},
    {-33.8688, 151.2093, "2023-10-28", 22.8}
};

// Helper for comparing doubles with a small tolerance
bool are_doubles_equal(double a, double b) {
    double epsilon = 1e-9;
    return fabs(a - b) < epsilon;
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
double temperature_for_location(double latitude, double longitude, const char* date) {
    // In a real application, this would use a database API with parameterized 
    // queries to prevent SQL injection.
    for (int i = 0; i < DATABASE_SIZE; ++i) {
        if (are_doubles_equal(sensorDatabase[i].latitude, latitude) &&
            are_doubles_equal(sensorDatabase[i].longitude, longitude) &&
            strcmp(sensorDatabase[i].date, date) == 0) {
            return sensorDatabase[i].temperature; // Return temperature on match
        }
    }
    return NAN; // Return Not-a-Number to indicate no data was found
}

void run_test(int testNum, const char* testName, double lat, double lon, const char* date) {
    double temp = temperature_for_location(lat, lon, date);
    printf("Test %d (%s): ", testNum, testName);
    if (isnan(temp)) {
        printf("Not Found\n");
    } else {
        printf("%.1f\n", temp);
    }
}

int main() {
    printf("--- Running 5 Test Cases ---\n");

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