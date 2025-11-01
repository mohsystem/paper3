#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// Since C doesn't have a built-in map, we use a struct array to simulate a database.
// In a real C application, you would use a database library like libpq (for PostgreSQL).
typedef struct {
    double latitude;
    double longitude;
    char date[11]; // YYYY-MM-DD
    double temperature;
} TemperatureData;

TemperatureData temperature_database[] = {
    {40.7128, -74.0060, "2023-10-27", 15.5},
    {34.0522, -118.2437, "2023-10-27", 22.1},
    {48.8566, 2.3522, "2023-10-26", 12.8}
};
const int DB_SIZE = sizeof(temperature_database) / sizeof(TemperatureData);


/**
 * Securely retrieves temperature for a given location and date.
 *
 * @param latitude The latitude of the location (-90 to 90).
 * @param longitude The longitude of the location (-180 to 180).
 * @param date_str The date in 'YYYY-MM-DD' format.
 * @param out_temp A pointer to a double where the result will be stored.
 * @return True if a temperature was found, false otherwise.
 */
bool temperature_for_location(double latitude, double longitude, const char* date_str, double* out_temp) {
    // 1. **Input Validation**: Crucial security step.
    if (latitude < -90.0 || latitude > 90.0) {
        fprintf(stderr, "Error: Invalid latitude value.\n");
        return false;
    }
    if (longitude < -180.0 || longitude > 180.0) {
        fprintf(stderr, "Error: Invalid longitude value.\n");
        return false;
    }
    // Basic date format validation
    int year, month, day;
    if (strlen(date_str) != 10 || sscanf(date_str, "%d-%d-%d", &year, &month, &day) != 3) {
        fprintf(stderr, "Error: Invalid date format. Please use YYYY-MM-DD.\n");
        return false;
    }
    // A more robust check would validate year/month/day ranges.

    // 2. **Secure Database Query (Demonstration)**
    // In a real application with a C database library like libpq,
    // ALWAYS use parameterized query functions (e.g., PQexecParams).
    // DO NOT build queries by concatenating strings using sprintf.
    /*
    // --- START OF SECURE libpq EXAMPLE (for illustration) ---
    // #include <libpq-fe.h>
    // PGconn *conn = PQconnectdb("dbname=mydb user=myuser password=mypass");
    // if (PQstatus(conn) == CONNECTION_OK) {
    //     const char *paramValues[3];
    //     char lat_str[32], lon_str[32];
    //
    //     snprintf(lat_str, sizeof(lat_str), "%f", latitude);
    //     snprintf(lon_str, sizeof(lon_str), "%f", longitude);
    //
    //     paramValues[0] = lat_str;
    //     paramValues[1] = lon_str;
    //     paramValues[2] = date_str;
    //
    //     const char* query = "SELECT temperature FROM temperatures WHERE latitude = $1::numeric AND longitude = $2::numeric AND date = $3::date";
    //
    //     // PQexecParams is the safe, parameterized function.
    //     PGresult *res = PQexecParams(conn, query, 3, NULL, paramValues, NULL, NULL, 0);
    //
    //     if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) == 1) {
    //         // Found data
    //         *out_temp = atof(PQgetvalue(res, 0, 0));
    //         PQclear(res);
    //         PQfinish(conn);
    //         return true;
    //     }
    //     PQclear(res);
    // }
    // PQfinish(conn);
    // return false; // Not found or error
    // --- END OF SECURE libpq EXAMPLE ---
    */

    // **Simulating the secure database lookup by iterating our array**
    // Using a small epsilon for floating point comparison.
    const double epsilon = 0.0001;
    for (int i = 0; i < DB_SIZE; ++i) {
        if (fabs(temperature_database[i].latitude - latitude) < epsilon &&
            fabs(temperature_database[i].longitude - longitude) < epsilon &&
            strcmp(temperature_database[i].date, date_str) == 0) {
            
            *out_temp = temperature_database[i].temperature;
            return true;
        }
    }

    return false; // Not found
}

int main() {
    printf("--- Running C Test Cases ---\n");
    double temp_result;

    // Test Case 1: Valid request, data exists
    printf("Test Case 1 (Success):\n");
    if (temperature_for_location(40.7128, -74.0060, "2023-10-27", &temp_result)) {
        printf("  Temperature: %.1f\n", temp_result);
    } else {
        printf("  Temperature not found.\n");
    }

    // Test Case 2: Valid request, data does not exist
    printf("\nTest Case 2 (Not Found):\n");
    if (temperature_for_location(10.0, 20.0, "2023-01-01", &temp_result)) {
        printf("  Temperature: %.1f\n", temp_result);
    } else {
        printf("  Temperature not found.\n");
    }

    // Test Case 3: Invalid latitude
    printf("\nTest Case 3 (Invalid Latitude):\n");
    if (temperature_for_location(100.0, -74.0060, "2023-10-27", &temp_result)) {
        printf("  Temperature: %.1f\n", temp_result);
    } else {
        printf("  Request failed as expected.\n");
    }

    // Test Case 4: Invalid longitude
    printf("\nTest Case 4 (Invalid Longitude):\n");
    if (temperature_for_location(40.7128, -200.0, "2023-10-27", &temp_result)) {
        printf("  Temperature: %.1f\n", temp_result);
    } else {
        printf("  Request failed as expected.\n");
    }

    // Test Case 5: Invalid date format
    printf("\nTest Case 5 (Invalid Date):\n");
    if (temperature_for_location(34.0522, -118.2437, "27-10-2023", &temp_result)) {
        printf("  Temperature: %.1f\n", temp_result);
    } else {
        printf("  Request failed as expected.\n");
    }

    return 0;
}