#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// C does not have a standard database API. To interact with a database, you would
// use a library like libsqlite3, libpq (for PostgreSQL), or an ODBC connector.
// This example simulates the secure pattern of using prepared statements, which
// is the correct way to prevent SQL injection.

// We will use a simple struct and array to mock a database for this example.
typedef struct {
    double latitude;
    double longitude;
    char date[11]; // YYYY-MM-DD\0
    double temperature;
} WeatherData;

WeatherData mock_db[] = {
    {40.71, -74.00, "2023-10-26", 15.5},
    {34.05, -118.24, "2023-10-26", 22.0},
    {48.85, 2.35, "2023-10-27", 12.3},
    {35.68, 139.69, "2023-10-27", 18.1},
    {0.0, 0.0, "", 0.0} // Sentinel value
};

/**
 * Simulates a secure query to the mock database.
 * Returns a pointer to the temperature value if found, otherwise NULL.
 */
const double* mock_query(double latitude, double longitude, const char* date) {
    printf("Mock DB: Executing secure query for lat=%.2f, lon=%.2f, date=%s\n", latitude, longitude, date);
    for (int i = 0; mock_db[i].date[0] != '\0'; ++i) {
        if (mock_db[i].latitude == latitude &&
            mock_db[i].longitude == longitude &&
            strcmp(mock_db[i].date, date) == 0) {
            return &mock_db[i].temperature;
        }
    }
    return NULL;
}


/**
 * Simulates connecting to a database and securely querying for the temperature.
 *
 * @param latitude The latitude of the location.
 * @param longitude The longitude of the location.
 * @param date The date for the temperature reading (e.g., "YYYY-MM-DD").
 * @param out_temperature A pointer to a double where the result will be stored.
 * @return 1 if found, 0 if not found or an error occurred.
 */
int temperature_for_location(double latitude, double longitude, const char* date, double* out_temperature) {
    // The SQL query with '?' placeholders to prevent SQL injection.
    const char* sql = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND aqi_date = ?";
    
    printf("\n--- C Secure Pattern Demonstration ---\n");
    printf("Preparing statement: %s\n", sql);
    printf("Binding parameter 1 (latitude): %f\n", latitude);
    printf("Binding parameter 2 (longitude): %f\n", longitude);
    printf("Binding parameter 3 (date): %s\n", date);

    /*
    // --- Real C DB Library (e.g., sqlite3) Code would look like this ---
    // sqlite3 *db;
    // sqlite3_stmt *stmt;
    // sqlite3_open("weather.db", &db);
    //
    // // 1. Prepare the statement
    // sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    //
    // // 2. Bind parameters (indices are 1-based)
    // sqlite3_bind_double(stmt, 1, latitude);
    // sqlite3_bind_double(stmt, 2, longitude);
    // sqlite3_bind_text(stmt, 3, date, -1, SQLITE_STATIC);
    //
    // // 3. Execute and process results
    // if (sqlite3_step(stmt) == SQLITE_ROW) {
    //     *out_temperature = sqlite3_column_double(stmt, 0);
    //     sqlite3_finalize(stmt);
    //     sqlite3_close(db);
    //     return 1; // Found
    // }
    //
    // sqlite3_finalize(stmt);
    // sqlite3_close(db);
    // return 0; // Not found or error
    */

    // Using the mock database for a self-contained runnable example.
    const double* temp_ptr = mock_query(latitude, longitude, date);
    if (temp_ptr != NULL) {
        *out_temperature = *temp_ptr;
        return 1; // Found
    }
    
    return 0; // Not found
}

void run_test(int test_num, double lat, double lon, const char* date) {
    double temperature;
    int found = temperature_for_location(lat, lon, date, &temperature);
    printf("Result %d: ", test_num);
    if (found) {
        printf("%f\n", temperature);
    } else {
        printf("Not Found\n");
    }
}

int main() {
    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Valid and existing data
    run_test(1, 40.71, -74.00, "2023-10-26");

    // Test Case 2: Valid and existing data
    run_test(2, 48.85, 2.35, "2023-10-27");

    // Test Case 3: Data that does not exist
    run_test(3, 0.0, 0.0, "2000-01-01");

    // Test Case 4: Malicious-looking input (which is safely handled by prepared statements)
    run_test(4, 0.0, 0.0, "2023-10-27' OR '1'='1");

    // Test Case 5: Another valid case
    run_test(5, 35.68, 139.69, "2023-10-27");

    return 0;
}