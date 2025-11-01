
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

/* Validation constants */
#define MIN_LAT -90.0
#define MAX_LAT 90.0
#define MIN_LON -180.0
#define MAX_LON 180.0
#define MAX_DATE_LEN 10
#define MAX_ERROR_LEN 256

/* Result structure for API response */
typedef struct {
    bool success;
    double temperature;
    char error[MAX_ERROR_LEN];
} TemperatureResponse;

/**
 * Validates latitude value
 * Prevents injection and ensures valid geographic coordinate
 */
bool isValidLatitude(double lat) {
    return lat >= MIN_LAT && lat <= MAX_LAT && !isnan(lat) && !isinf(lat);
}

/**
 * Validates longitude value
 * Prevents injection and ensures valid geographic coordinate
 */
bool isValidLongitude(double lon) {
    return lon >= MIN_LON && lon <= MAX_LON && !isnan(lon) && !isinf(lon);
}

/**
 * Validates date format (YYYY-MM-DD)
 * Prevents SQL injection via date parameter
 */
bool isValidDate(const char* date) {
    size_t len;
    
    /* NULL pointer check */
    if (date == NULL) {
        return false;
    }
    
    /* Check length with strnlen to prevent buffer over-read */
    len = strnlen(date, MAX_DATE_LEN + 1);
    if (len != MAX_DATE_LEN) {
        return false;
    }
    
    /* Validate format: YYYY-MM-DD */
    for (size_t i = 0; i < MAX_DATE_LEN; i++) {
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
 * Main API function to get temperature for a location
 * Uses parameterized queries to prevent SQL injection
 * Validates all inputs before processing
 * Fails closed on any validation error
 */
TemperatureResponse temperatureForLocation(double latitude, double longitude, const char* date) {
    TemperatureResponse result;
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int rc;
    
    /* Initialize result structure */
    result.success = false;
    result.temperature = 0.0;
    memset(result.error, 0, MAX_ERROR_LEN);
    
    /* Input validation - reject invalid data early */
    if (!isValidLatitude(latitude)) {
        snprintf(result.error, MAX_ERROR_LEN, "Invalid request");
        return result;
    }
    
    if (!isValidLongitude(longitude)) {
        snprintf(result.error, MAX_ERROR_LEN, "Invalid request");
        return result;
    }
    
    if (!isValidDate(date)) {
        snprintf(result.error, MAX_ERROR_LEN, "Invalid request");
        return result;
    }
    
    /* Open database connection */
    rc = sqlite3_open("Sensor.db", &db);
    if (rc != SQLITE_OK) {
        /* Generic error message - do not leak internal details */
        snprintf(result.error, MAX_ERROR_LEN, "Service error");
        if (db != NULL) {
            sqlite3_close(db);
        }
        return result;
    }
    
    /* Use parameterized query to prevent SQL injection */
    /* All user inputs are bound as parameters, never concatenated */
    const char* query = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ? LIMIT 1";
    
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        /* Generic error message - do not leak database structure */
        snprintf(result.error, MAX_ERROR_LEN, "Service error");
        sqlite3_close(db);
        return result;
    }
    
    /* Bind parameters with type safety */
    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);
    
    /* Execute query */
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        /* Check if value is NULL */
        if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            result.success = true;
            result.temperature = sqlite3_column_double(stmt, 0);
        } else {
            snprintf(result.error, MAX_ERROR_LEN, "Data not available");
        }
    } else if (rc == SQLITE_DONE) {
        snprintf(result.error, MAX_ERROR_LEN, "Data not found");
    } else {
        /* Generic error message - do not leak database details */
        snprintf(result.error, MAX_ERROR_LEN, "Service error");
    }
    
    /* Cleanup resources - freed exactly once */
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main(void) {
    TemperatureResponse result;
    
    /* Test case 1: Valid request */
    result = temperatureForLocation(40.7128, -74.0060, "2024-01-15");
    printf("Test 1 - Success: %d, Temp: %.2f, Error: %s\\n", 
           result.success, result.temperature, result.error);
    
    /* Test case 2: Invalid latitude */
    result = temperatureForLocation(95.0, -74.0060, "2024-01-15");
    printf("Test 2 - Success: %d, Temp: %.2f, Error: %s\\n", 
           result.success, result.temperature, result.error);
    
    /* Test case 3: Invalid date format (SQL injection attempt) */
    result = temperatureForLocation(40.7128, -74.0060, "2024-01-15' OR '1'='1");
    printf("Test 3 - Success: %d, Temp: %.2f, Error: %s\\n", 
           result.success, result.temperature, result.error);
    
    /* Test case 4: Invalid longitude */
    result = temperatureForLocation(40.7128, -200.0, "2024-01-15");
    printf("Test 4 - Success: %d, Temp: %.2f, Error: %s\\n", 
           result.success, result.temperature, result.error);
    
    /* Test case 5: Valid coordinates, different date */
    result = temperatureForLocation(51.5074, -0.1278, "2024-02-20");
    printf("Test 5 - Success: %d, Temp: %.2f, Error: %s\\n", 
           result.success, result.temperature, result.error);
    
    return 0;
}
