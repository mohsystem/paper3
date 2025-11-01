#include <stdio.h>
#include <string.h>
#include <math.h> // For fabs

#define DB_SIZE 5
#define DATE_LEN 11 // YYYY-MM-DD + null terminator

// Simulating a database record
typedef struct {
    double latitude;
    double longitude;
    char date[DATE_LEN];
    double temperature;
} SensorReading;

// Simulating a database table called 'Sensor'
SensorReading SensorDatabase[DB_SIZE] = {
    {34.0522, -118.2437, "2023-10-26", 25.5},
    {40.7128, -74.0060, "2023-10-26", 15.2},
    {48.8566, 2.3522, "2023-10-27", 12.8},
    {35.6895, 139.6917, "2023-10-26", 18.0},
    {-33.8688, 151.2093, "2023-10-28", 22.3}
};

/**
 * This function simulates an API endpoint call.
 * It connects to the 'database' and gets the temperature for the given location and date.
 *
 * @param latitude The latitude of the location.
 * @param longitude The longitude of the location.
 * @param date The date in "YYYY-MM-DD" format.
 * @param out_temperature A pointer to a double where the result will be stored if found.
 * @return 1 if a record is found, 0 otherwise.
 */
int temperature_for_location(double latitude, double longitude, const char* date, double* out_temperature) {
    for (int i = 0; i < DB_SIZE; i++) {
        // Compare floats with a small tolerance
        if (fabs(SensorDatabase[i].latitude - latitude) < 0.0001 &&
            fabs(SensorDatabase[i].longitude - longitude) < 0.0001 &&
            strcmp(SensorDatabase[i].date, date) == 0) {
            
            *out_temperature = SensorDatabase[i].temperature;
            return 1; // Found
        }
    }
    return 0; // Not found
}

int main() {
    double temp_result;

    // --- 5 Test Cases ---

    // Test Case 1: Existing data
    printf("Test Case 1: (LA, 2023-10-26)\n");
    if (temperature_for_location(34.0522, -118.2437, "2023-10-26", &temp_result)) {
        printf("  -> Temperature: %.1f\n", temp_result);
    } else {
        printf("  -> Temperature not found.\n");
    }

    // Test Case 2: Existing data
    printf("\nTest Case 2: (NYC, 2023-10-26)\n");
    if (temperature_for_location(40.7128, -74.0060, "2023-10-26", &temp_result)) {
        printf("  -> Temperature: %.1f\n", temp_result);
    } else {
        printf("  -> Temperature not found.\n");
    }

    // Test Case 3: Existing data
    printf("\nTest Case 3: (Paris, 2023-10-27)\n");
    if (temperature_for_location(48.8566, 2.3522, "2023-10-27", &temp_result)) {
        printf("  -> Temperature: %.1f\n", temp_result);
    } else {
        printf("  -> Temperature not found.\n");
    }

    // Test Case 4: Non-existing date
    printf("\nTest Case 4: (Tokyo, Non-existing date)\n");
    if (temperature_for_location(35.6895, 139.6917, "2023-10-27", &temp_result)) {
        printf("  -> Temperature: %.1f\n", temp_result);
    } else {
        printf("  -> Temperature not found.\n");
    }

    // Test Case 5: Non-existing location
    printf("\nTest Case 5: (Non-existing location)\n");
    if (temperature_for_location(0.0, 0.0, "2023-10-26", &temp_result)) {
        printf("  -> Temperature: %.1f\n", temp_result);
    } else {
        printf("  -> Temperature not found.\n");
    }

    return 0;
}