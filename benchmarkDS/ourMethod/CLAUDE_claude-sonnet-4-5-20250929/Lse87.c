
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <errno.h>
#include <ctype.h>

/* Maximum allowed values to prevent resource exhaustion */
#define MAX_LATITUDE 90.0
#define MIN_LATITUDE -90.0
#define MAX_LONGITUDE 180.0
#define MIN_LONGITUDE -180.0
#define MAX_ERROR_MSG 256
#define MAX_RESULT_MSG 256

/* Validates and parses a double value from string */
int parse_double(const char* input, double* output) {
    char* end = NULL;
    
    if (input == NULL || *input == '\\0') {
        return 0;
    }
    
    /* Reset errno before conversion */
    errno = 0;
    *output = strtod(input, &end);
    
    /* Check for conversion errors */
    if (errno == ERANGE || end == input || *end != '\\0') {
        return 0;
    }
    
    return 1;
}

/* Validates date format (YYYY-MM-DD) */
int validate_date_format(const char* date) {
    int year = 0, month = 0, day = 0;
    int matched = 0;
    
    if (date == NULL || strlen(date) != 10) {
        return 0;
    }
    
    /* Check format with sscanf */
    matched = sscanf(date, "%4d-%2d-%2d", &year, &month, &day);
    if (matched != 3) {
        return 0;
    }
    
    /* Validate date ranges */
    if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31) {
        return 0;
    }
    
    /* Verify format characters */
    if (date[4] != '-' || date[7] != '-') {
        return 0;
    }
    
    return 1;
}

/**
 * Retrieves temperature for a given location and date.
 * Uses parameterized queries to prevent SQL injection.
 * Validates all inputs before processing.
 * 
 * Returns: Dynamically allocated string with result (caller must free)
 */
char* temperature_for_location(const char* latitude, const char* longitude, const char* date) {
    char* result = NULL;
    double lat = 0.0, lon = 0.0;
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int rc = 0;
    
    /* Allocate result buffer - caller must free */
    result = (char*)malloc(MAX_RESULT_MSG);
    if (result == NULL) {
        return NULL;
    }
    /* Initialize to empty string */
    result[0] = '\\0';
    
    /* Input validation: Check for NULL or empty inputs */
    if (latitude == NULL || *latitude == '\\0') {
        snprintf(result, MAX_RESULT_MSG, "Error: Invalid latitude parameter");
        return result;
    }
    if (longitude == NULL || *longitude == '\\0') {
        snprintf(result, MAX_RESULT_MSG, "Error: Invalid longitude parameter");
        return result;
    }
    if (date == NULL || *date == '\\0') {
        snprintf(result, MAX_RESULT_MSG, "Error: Invalid date parameter");
        return result;
    }
    
    /* Input validation: Parse and validate latitude */
    if (!parse_double(latitude, &lat)) {
        snprintf(result, MAX_RESULT_MSG, "Error: Invalid coordinate format");
        return result;
    }
    if (lat < MIN_LATITUDE || lat > MAX_LATITUDE) {
        snprintf(result, MAX_RESULT_MSG, "Error: Latitude out of valid range");
        return result;
    }
    
    /* Input validation: Parse and validate longitude */
    if (!parse_double(longitude, &lon)) {
        snprintf(result, MAX_RESULT_MSG, "Error: Invalid coordinate format");
        return result;
    }
    if (lon < MIN_LONGITUDE || lon > MAX_LONGITUDE) {
        snprintf(result, MAX_RESULT_MSG, "Error: Longitude out of valid range");
        return result;
    }
    
    /* Input validation: Validate date format */
    if (!validate_date_format(date)) {
        snprintf(result, MAX_RESULT_MSG, "Error: Invalid date format");
        return result;
    }
    
    /* Open database connection */
    rc = sqlite3_open("weather.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database connection error\\n");
        snprintf(result, MAX_RESULT_MSG, "Error: Database operation failed");
        if (db != NULL) {
            sqlite3_close(db);
        }
        return result;
    }
    
    /* Use parameterized query to prevent SQL injection */
    const char* query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ? LIMIT 1";
    
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Query preparation error\\n");
        snprintf(result, MAX_RESULT_MSG, "Error: Database operation failed");
        sqlite3_close(db);
        return result;
    }
    
    /* Bind parameters safely - database driver handles proper escaping */
    sqlite3_bind_double(stmt, 1, lat);
    sqlite3_bind_double(stmt, 2, lon);
    sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);
    
    /* Execute query */
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        double temperature = sqlite3_column_double(stmt, 0);
        snprintf(result, MAX_RESULT_MSG, "%.2f", temperature);
    } else if (rc == SQLITE_DONE) {
        snprintf(result, MAX_RESULT_MSG, "Error: No data found for given parameters");
    } else {
        fprintf(stderr, "Query execution error\\n");
        snprintf(result, MAX_RESULT_MSG, "Error: Database operation failed");
    }
    
    /* Clean up resources */
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main(void) {
    char* result = NULL;
    
    /* Test case 1: Valid input */
    result = temperature_for_location("40.7128", "-74.0060", "2024-01-15");
    if (result != NULL) {
        printf("Test 1: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test case 2: Invalid latitude (out of range) */
    result = temperature_for_location("95.0", "-74.0060", "2024-01-15");
    if (result != NULL) {
        printf("Test 2: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test case 3: Invalid date format */
    result = temperature_for_location("40.7128", "-74.0060", "01/15/2024");
    if (result != NULL) {
        printf("Test 3: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test case 4: NULL input */
    result = temperature_for_location(NULL, "-74.0060", "2024-01-15");
    if (result != NULL) {
        printf("Test 4: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test case 5: SQL injection attempt (safely handled by parameterized query) */
    result = temperature_for_location("40.7128' OR '1'='1", "-74.0060", "2024-01-15");
    if (result != NULL) {
        printf("Test 5: %s\\n", result);
        free(result);
        result = NULL;
    }
    
    return 0;
}
