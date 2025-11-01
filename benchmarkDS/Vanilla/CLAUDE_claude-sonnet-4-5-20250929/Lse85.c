
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct {
    double temperature;
    int success;
    char message[256];
} Result;

Result temperature_for_location(double latitude, double longitude, const char* date) {
    Result result;
    result.temperature = 0.0;
    result.success = 0;
    strcpy(result.message, "");
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open("Sensor.db", &db);
    
    if (rc) {
        result.success = 0;
        strcpy(result.message, "Cannot open database");
        return result;
    }
    
    const char* query = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ?";
    
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        result.success = 0;
        strcpy(result.message, "Failed to prepare statement");
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, date, -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        result.temperature = sqlite3_column_double(stmt, 0);
        result.success = 1;
        strcpy(result.message, "Success");
    } else {
        result.success = 0;
        strcpy(result.message, "No data found");
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    // Test case 1
    Result result1 = temperature_for_location(40.7128, -74.0060, "2023-01-15");
    printf("Test 1: temperature=%.2f, success=%d, message=%s\\n", 
           result1.temperature, result1.success, result1.message);
    
    // Test case 2
    Result result2 = temperature_for_location(34.0522, -118.2437, "2023-02-20");
    printf("Test 2: temperature=%.2f, success=%d, message=%s\\n", 
           result2.temperature, result2.success, result2.message);
    
    // Test case 3
    Result result3 = temperature_for_location(51.5074, -0.1278, "2023-03-10");
    printf("Test 3: temperature=%.2f, success=%d, message=%s\\n", 
           result3.temperature, result3.success, result3.message);
    
    // Test case 4
    Result result4 = temperature_for_location(35.6762, 139.6503, "2023-04-05");
    printf("Test 4: temperature=%.2f, success=%d, message=%s\\n", 
           result4.temperature, result4.success, result4.message);
    
    // Test case 5
    Result result5 = temperature_for_location(48.8566, 2.3522, "2023-05-12");
    printf("Test 5: temperature=%.2f, success=%d, message=%s\\n", 
           result5.temperature, result5.success, result5.message);
    
    return 0;
}
