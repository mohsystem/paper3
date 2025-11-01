#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Define a structure to hold our sensor data records
typedef struct {
    double latitude;
    double longitude;
    char date[11]; // "YYYY-MM-DD" + null terminator
    double temperature;
} SensorData;

// Simulating a database table from the "Sensor" database using a fixed-size array.
// In a real application, this would be a connection to a database.
SensorData sensor_data_table[] = {
    {40.7128, -74.0060, "2023-10-27", 15.5},
    {34.0522, -118.2437, "2023-10-27", 22.1},
    {48.8566, 2.3522, "2023-10-26", 12.8},
    {35.6895, 139.6917, "2023-10-27", 18.2}
};
const int NUM_RECORDS = sizeof(sensor_data_table) / sizeof(SensorData);

/**
 * @brief Simulates fetching temperature for a given location and date.
 *
 * This function demonstrates a secure way to query data. It iterates through
 * records and compares values directly, which is analogous to using a
 * prepared statement in a database C API (like libpq for PostgreSQL).
 * It avoids using functions like sprintf() to build a raw SQL query string,
 * which is vulnerable to SQL injection.
 *
 * A real implementation with libpq would look like this:
 *
 *   const char *query = "SELECT temp FROM data WHERE lat = $1 AND lon = $2 AND date = $3";
 *   const char *paramValues[3] = {lat_str, lon_str, date_str};
 *   PGresult *res = PQexecParams(conn, query, 3, NULL, paramValues, NULL, NULL, 0);
 *
 * This separates the command from the data, which is the fundamental principle
 * of preventing SQL injection.
 *
 * @param latitude The latitude of the location.
 * @param longitude The longitude of the location.
 * @param date The date string (e.g., "2023-10-27").
 * @param out_temperature A pointer to a double where the result will be stored.
 * @return `true` if a record was found, `false` otherwise.
 */
bool temperature_for_location(double latitude, double longitude, const char* date, double* out_temperature) {
    if (date == NULL || strlen(date) == 0 || out_temperature == NULL) {
        return false;
    }

    for (int i = 0; i < NUM_RECORDS; ++i) {
        // Direct comparison is safe and avoids injection vulnerabilities.
        // A small epsilon could be used for floating-point comparison if needed.
        if (sensor_data_table[i].latitude == latitude &&
            sensor_data_table[i].longitude == longitude &&
            strcmp(sensor_data_table[i].date, date) == 0)
        {
            *out_temperature = sensor_data_table[i].temperature;
            return true;
        }
    }

    return false; // Record not found
}

int main() {
    printf("--- Running 5 Test Cases ---\n");
    double temperature;

    // Test Case 1: Valid and existing data
    printf("Test Case 1 (NYC):\n");
    if (temperature_for_location(40.7128, -74.0060, "2023-10-27", &temperature)) {
        printf("  Temperature: %.1f\n", temperature);
    } else {
        printf("  Temperature not found.\n");
    }

    // Test Case 2: Valid and existing data
    printf("Test Case 2 (LA):\n");
    if (temperature_for_location(34.0522, -118.2437, "2023-10-27", &temperature)) {
        printf("  Temperature: %.1f\n", temperature);
    } else {
        printf("  Temperature not found.\n");
    }

    // Test Case 3: Data not found (wrong date)
    printf("Test Case 3 (Paris, wrong date):\n");
    if (temperature_for_location(48.8566, 2.3522, "2023-10-27", &temperature)) {
        printf("  Temperature: %.1f\n", temperature);
    } else {
        printf("  Temperature not found.\n");
    }

    // Test Case 4: Data not found (non-existent location)
    printf("Test Case 4 (London):\n");
    if (temperature_for_location(51.5074, -0.1278, "2023-10-27", &temperature)) {
        printf("  Temperature: %.1f\n", temperature);
    } else {
        printf("  Temperature not found.\n");
    }
    
    // Test Case 5: Invalid input (NULL date)
    printf("Test Case 5 (Invalid Input):\n");
    if (temperature_for_location(35.6895, 139.6917, NULL, &temperature)) {
        printf("  Temperature: %.1f\n", temperature);
    } else {
        printf("  Temperature not found.\n");
    }

    return 0;
}