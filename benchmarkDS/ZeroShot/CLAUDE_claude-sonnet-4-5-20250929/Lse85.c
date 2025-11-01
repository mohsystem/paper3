
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <stdbool.h>

typedef struct {
    double temperature;
    bool success;
    char error[256];
} TemperatureResult;

TemperatureResult temperature_for_location(double latitude, double longitude, const char* date) {
    TemperatureResult result;
    result.temperature = 0.0;
    result.success = false;
    memset(result.error, 0, sizeof(result.error));
    
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    
    // Input validation
    if (date == NULL || strlen(date) == 0) {
        strcpy(result.error, "Invalid date");
        return result;
    }
    
    if (latitude < -90 || latitude > 90 || longitude < -180 || longitude > 180) {
        strcpy(result.error, "Invalid coordinates");
        return result;
    }
    
    // Open database
    int rc = sqlite3_open("Sensor.db", &db);
    if (rc != SQLITE_OK) {
        snprintf(result.error, sizeof(result.error), "Cannot open database: %s", sqlite3_errmsg(db));
        if (db) sqlite3_close(db);
        return result;
    }
    
    // Prepare parameterized statement
    const char* sql = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        snprintf(result.error, sizeof(result.error), "Failed to prepare statement: %s", sqlite3_errmsg(db));
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
        result.temperature = sqlite3_column_double(stmt, 0);
        result.success = true;
    } else {
        strcpy(result.error, "No data found");
    }
    
    // Cleanup
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    // Test cases
    printf("Test Case 1:\\n");
    TemperatureResult r1 = temperature_for_location(40.7128, -74.0060, "2023-01-15");
    printf("Success: %d, Temperature: %.2f, Error: %s\\n\\n", r1.success, r1.temperature, r1.error);
    
    printf("Test Case 2:\\n");
    TemperatureResult r2 = temperature_for_location(51.5074, -0.1278, "2023-02-20");
    printf("Success: %d, Temperature: %.2f, Error: %s\\n\\n", r2.success, r2.temperature, r2.error);
    
    printf("Test Case 3:\\n");
    TemperatureResult r3 = temperature_for_location(35.6762, 139.6503, "2023-03-10");
    printf("Success: %d, Temperature: %.2f, Error: %s\\n\\n", r3.success, r3.temperature, r3.error);
    
    printf("Test Case 4 (Invalid coordinates):\\n");
    TemperatureResult r4 = temperature_for_location(100.0, -200.0, "2023-04-05");
    printf("Success: %d, Temperature: %.2f, Error: %s\\n\\n", r4.success, r4.temperature, r4.error);
    
    printf("Test Case 5 (Empty date):\\n");
    TemperatureResult r5 = temperature_for_location(48.8566, 2.3522, "");
    printf("Success: %d, Temperature: %.2f, Error: %s\\n", r5.success, r5.temperature, r5.error);
    
    return 0;
}
