#include <stdio.h>
#include <string.h>

// Mock WeatherData struct to represent a database record
typedef struct {
    double latitude;
    double longitude;
    char date[11]; // YYYY-MM-DD + null terminator
    double temperature;
} WeatherData;

// Mock database table
WeatherData mockDatabase[] = {
    {40.7128, -74.0060, "2023-10-27", 15.5},
    {34.0522, -118.2437, "2023-10-27", 22.0},
    {48.8566, 2.3522, "2023-10-28", 12.3},
    {35.6895, 139.6917, "2023-10-28", 18.7}
};
const int db_size = sizeof(mockDatabase) / sizeof(mockDatabase[0]);

/**
 * Simulates fetching temperature for a given location and date from a database.
 *
 * @param latitude The latitude of the location.
 * @param longitude The longitude of the location.
 * @param date The date for the temperature reading (format YYYY-MM-DD).
 * @param out_temperature A pointer to a double where the result will be stored.
 * @return 1 if the temperature was found, 0 otherwise.
 */
int temperature_for_location(double latitude, double longitude, const char* date, double* out_temperature) {
    // Simulate "SELECT temperature FROM weather_data WHERE ..."
    for (int i = 0; i < db_size; ++i) {
        if (mockDatabase[i].latitude == latitude &&
            mockDatabase[i].longitude == longitude &&
            strcmp(mockDatabase[i].date, date) == 0) {
            
            *out_temperature = mockDatabase[i].temperature;
            return 1; // Found
        }
    }
    return 0; // Not found
}

int main() {
    // Mock test case struct
    typedef struct {
        double lat;
        double lon;
        const char* date;
    } TestCase;
    
    // 5 Test Cases
    TestCase testCases[] = {
        {40.7128, -74.0060, "2023-10-27"},   // Case 1: Exists
        {34.0522, -118.2437, "2023-10-27"},  // Case 2: Exists
        {48.8566, 2.3522, "2023-10-28"},     // Case 3: Exists
        {0.0, 0.0, "2023-10-27"},            // Case 4: Does not exist
        {35.6895, 139.6917, "2023-10-29"}    // Case 5: Exists but wrong date
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        double temp;
        int found = temperature_for_location(testCases[i].lat, testCases[i].lon, testCases[i].date, &temp);

        printf("Test Case %d (Lat: %.4f, Lon: %.4f, Date: %s): ", 
               i + 1, testCases[i].lat, testCases[i].lon, testCases[i].date);

        if (found) {
            printf("Temperature = %.1f C\n", temp);
        } else {
            printf("Temperature data not found.\n");
        }
    }

    return 0;
}