
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct {
    double value;
    int found;
} TemperatureResult;

TemperatureResult temperature_for_location(double latitude, double longitude, const char* date) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    TemperatureResult result = {0.0, 0};
    int rc;
    
    // Input validation
    if (date == NULL || strlen(date) == 0) {
        fprintf(stderr, "Date cannot be NULL or empty\\n");
        return result;
    }
    
    if (latitude < -90 || latitude > 90) {
        fprintf(stderr, "Invalid latitude\\n");
        return result;
    }
    
    if (longitude < -180 || longitude > 180) {
        fprintf(stderr, "Invalid longitude\\n");
        return result;
    }
    
    // Open database connection
    rc = sqlite3_open("weather.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        if (db) sqlite3_close(db);
        return result;
    }
    
    // Prepare parameterized query to prevent SQL injection
    const char* query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ?";
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }
    
    // Bind parameters
    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);
    
    // Execute query
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        result.value = sqlite3_column_double(stmt, 0);
        result.found = 1;
    }
    
    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    // Test cases
    printf("Test Case 1:\\n");
    TemperatureResult temp1 = temperature_for_location(40.7128, -74.0060, "2023-01-15");
    if (temp1.found) {
        printf("Temperature: %.2f\\n", temp1.value);
    } else {
        printf("Temperature: Not found\\n");
    }
    
    printf("\\nTest Case 2:\\n");
    TemperatureResult temp2 = temperature_for_location(51.5074, -0.1278, "2023-06-20");
    if (temp2.found) {
        printf("Temperature: %.2f\\n", temp2.value);
    } else {
        printf("Temperature: Not found\\n");
    }
    
    printf("\\nTest Case 3:\\n");
    TemperatureResult temp3 = temperature_for_location(35.6762, 139.6503, "2023-12-25");
    if (temp3.found) {
        printf("Temperature: %.2f\\n", temp3.value);
    } else {
        printf("Temperature: Not found\\n");
    }
    
    printf("\\nTest Case 4:\\n");
    TemperatureResult temp4 = temperature_for_location(-33.8688, 151.2093, "2023-03-10");
    if (temp4.found) {
        printf("Temperature: %.2f\\n", temp4.value);
    } else {
        printf("Temperature: Not found\\n");
    }
    
    printf("\\nTest Case 5:\\n");
    TemperatureResult temp5 = temperature_for_location(48.8566, 2.3522, "2023-09-05");
    if (temp5.found) {
        printf("Temperature: %.2f\\n", temp5.value);
    } else {
        printf("Temperature: Not found\\n");
    }
    
    return 0;
}
