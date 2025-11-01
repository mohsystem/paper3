#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h> // For NAN

// A mock database query function. In a real application, this would use a library
// like libpq (for PostgreSQL) or the MySQL C API.
// It would use prepared statements to prevent SQL injection.
static double queryMockDatabase(double latitude, double longitude, const char* date) {
    // This is a placeholder logic to generate a deterministic "temperature"
    // based on the inputs, simulating a database lookup.
    // The database would be named "Sensor".
    printf("Connecting to database 'Sensor'...\n");
    printf("Querying for latitude=%.4f, longitude=%.4f, date=%s\n", latitude, longitude, date);

    // Simple deterministic calculation for demonstration
    double baseTemp = (latitude + 45.0) / 10.0; // Base temp related to latitude
    
    // Simple hash-like function for the date string
    unsigned long date_hash = 5381;
    int c;
    const char *str = date;
    while ((c = *str++)) {
        date_hash = ((date_hash << 5) + date_hash) + c; // djb2 hash
    }

    double dailyVariation = (date_hash % 200) / 10.0 - 10.0; // Variation from -10 to +10

    return baseTemp + dailyVariation;
}

// Helper function to validate date format (YYYY-MM-DD)
// Rule #1 and #3: Validating format and length to prevent processing errors and potential overflows.
static bool is_valid_date_format(const char* date) {
    if (date == NULL) {
        return false;
    }
    
    if (strlen(date) != 10) {
        return false;
    }

    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) {
            if (date[i] != '-') {
                return false;
            }
        } else {
            if (!isdigit((unsigned char)date[i])) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Simulates an API endpoint function that retrieves temperature for a given location and date.
 * It validates inputs and fetches data from a mock database.
 * 
 * @param latitude The geographical latitude (-90.0 to 90.0).
 * @param longitude The geographical longitude (-180.0 to 180.0).
 * @param date The date in YYYY-MM-DD format.
 * @return The temperature in Celsius, or NAN if an error occurs.
 */
double temperature_for_location(double latitude, double longitude, const char* date) {
    // Rule #1: Ensure all input is validated.
    if (latitude < -90.0 || latitude > 90.0) {
        fprintf(stderr, "Error: Invalid latitude. Must be between -90.0 and 90.0.\n");
        return NAN;
    }
    if (longitude < -180.0 || longitude > 180.0) {
        fprintf(stderr, "Error: Invalid longitude. Must be between -180.0 and 180.0.\n");
        return NAN;
    }

    if (!is_valid_date_format(date)) {
        fprintf(stderr, "Error: Invalid date format. Expected YYYY-MM-DD.\n");
        return NAN;
    }

    // In a real application, database connection logic would be here.
    return queryMockDatabase(latitude, longitude, date);
}

int main() {
    printf("--- Running Test Cases ---\n");
    
    // Test Case 1: Valid input
    double temp1 = temperature_for_location(40.7128, -74.0060, "2023-10-27");
    if (!isnan(temp1)) {
        printf("Test 1 PASSED: Temperature is %.2f°C\n", temp1);
    } else {
        printf("Test 1 FAILED\n");
    }
    printf("\n");

    // Test Case 2: Another valid input
    double temp2 = temperature_for_location(34.0522, -118.2437, "2024-01-15");
    if (!isnan(temp2)) {
        printf("Test 2 PASSED: Temperature is %.2f°C\n", temp2);
    } else {
        printf("Test 2 FAILED\n");
    }
    printf("\n");

    // Test Case 3: Invalid latitude
    double temp3 = temperature_for_location(95.0, -74.0060, "2023-10-27");
    if (isnan(temp3)) {
        printf("Test 3 PASSED: Correctly handled invalid latitude.\n");
    } else {
        printf("Test 3 FAILED: Did not handle invalid latitude.\n");
    }
    printf("\n");

    // Test Case 4: Invalid date format
    double temp4 = temperature_for_location(40.7128, -74.0060, "2023-10-27T10:00:00");
    if (isnan(temp4)) {
        printf("Test 4 PASSED: Correctly handled invalid date format.\n");
    } else {
        printf("Test 4 FAILED: Did not handle invalid date format.\n");
    }
    printf("\n");

    // Test Case 5: NULL date
    double temp5 = temperature_for_location(40.7128, -74.0060, NULL);
    if (isnan(temp5)) {
        printf("Test 5 PASSED: Correctly handled NULL date.\n");
    } else {
        printf("Test 5 FAILED: Did not handle NULL date.\n");
    }
    printf("\n");

    return 0;
}