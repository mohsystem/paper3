#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Mock database table
typedef struct {
    double latitude;
    double longitude;
    char date[11]; // "YYYY-MM-DD" + null terminator
    double temperature;
} WeatherData;

WeatherData mock_database[] = {
    {34.0522, -118.2437, "2023-10-26", 75.5},
    {40.7128, -74.0060, "2023-10-26", 62.1},
    {48.8566, 2.3522, "2023-10-27", 58.3}
};
const int DB_SIZE = sizeof(mock_database) / sizeof(WeatherData);

/**
 * @brief Validates if a string is in "YYYY-MM-DD" format.
 * This is a safe alternative to sscanf for format validation.
 *
 * @param date_str The string to validate.
 * @return true if the format is valid, false otherwise.
 */
bool is_valid_date_format(const char* date_str) {
    if (date_str == NULL || strlen(date_str) != 10) {
        return false;
    }
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) {
            if (date_str[i] != '-') {
                return false;
            }
        } else {
            if (!isdigit((unsigned char)date_str[i])) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Simulates a database query to get the temperature.
 *
 * @param latitude The latitude of the location (-90.0 to 90.0).
 * @param longitude The longitude of the location (-180.0 to 180.0).
 * @param date The date in 'YYYY-MM-DD' format.
 * @param out_temperature Pointer to a double where the result will be stored.
 * @return 0 on success, -1 on failure (invalid input or data not found).
 */
int temperature_for_location(double latitude, double longitude, const char* date, double* out_temperature) {
    // 1. Input Validation (Rule #1)
    if (out_temperature == NULL) {
        fprintf(stderr, "Error: Output parameter cannot be NULL.\n");
        return -1;
    }
    
    if (latitude < -90.0 || latitude > 90.0) {
        fprintf(stderr, "Error: Invalid latitude value.\n");
        return -1;
    }
    if (longitude < -180.0 || longitude > 180.0) {
        fprintf(stderr, "Error: Invalid longitude value.\n");
        return -1;
    }
    if (!is_valid_date_format(date)) {
        fprintf(stderr, "Error: Invalid date format. Expected 'YYYY-MM-DD'.\n");
        return -1;
    }

    // 2. Simulate database connection and parameterized query (prevents SQL injection)
    printf("Simulating database connection...\n");
    // In a real C API (like sqlite3):
    // sqlite3_stmt *stmt;
    // const char *sql = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ?;";
    // sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    // sqlite3_bind_double(stmt, 1, latitude);
    // sqlite3_bind_double(stmt, 2, longitude);
    // sqlite3_bind_text(stmt, 3, date, -1, SQLITE_STATIC);
    // if (sqlite3_step(stmt) == SQLITE_ROW) { ... }

    printf("Executing mock query for lat=%f, lon=%f, date=%s\n", latitude, longitude, date);

    bool found = false;
    for (int i = 0; i < DB_SIZE; i++) {
        if (mock_database[i].latitude == latitude &&
            mock_database[i].longitude == longitude &&
            strcmp(mock_database[i].date, date) == 0) {
            *out_temperature = mock_database[i].temperature;
            found = true;
            break;
        }
    }

    printf("Closing database connection...\n");

    if (found) {
        return 0; // Success
    } else {
        return -1; // Not found
    }
}

void run_test(const char* test_name, double lat, double lon, const char* date) {
    printf("\n%s:\n", test_name);
    double temperature;
    if (temperature_for_location(lat, lon, date, &temperature) == 0) {
        printf("Success! Temperature: %f\n", temperature);
    } else {
        printf("Failed as expected or data not found.\n");
    }
}

int main() {
    printf("--- Test Cases ---\n");
    
    // Test Case 1: Valid request
    run_test("1. Valid Request", 34.0522, -118.2437, "2023-10-26");

    // Test Case 2: Data not in database
    run_test("2. Data Not Found", 0.0, 0.0, "2023-01-01");

    // Test Case 3: Invalid latitude
    run_test("3. Invalid Latitude", 95.0, -74.0060, "2023-10-26");

    // Test Case 4: Invalid date format
    run_test("4. Invalid Date Format", 40.7128, -74.0060, "2023.10.26");

    // Test Case 5: Valid request for Paris
    run_test("5. Valid Request (Paris)", 48.8566, 2.3522, "2023-10-27");
    
    return 0;
}