
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

typedef struct {
    double value;
    int is_valid;
} TemperatureResult;

TemperatureResult temperature_for_location(double latitude, double longitude, const char* date) {
    TemperatureResult result = {0.0, 0};
    MYSQL* conn;
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[3];
    MYSQL_BIND result_bind[1];
    double temperature;
    my_bool is_null;
    
    conn = mysql_init(NULL);
    
    if (!mysql_real_connect(conn, "localhost", "root", "password", "weather_db", 0, NULL, 0)) {
        fprintf(stderr, "Connection failed: %s\\n", mysql_error(conn));
        return result;
    }
    
    const char* query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ?";
    stmt = mysql_stmt_init(conn);
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Prepare failed: %s\\n", mysql_stmt_error(stmt));
        mysql_close(conn);
        return result;
    }
    
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[0].buffer = &latitude;
    
    bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[1].buffer = &longitude;
    
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (char*)date;
    bind[2].buffer_length = strlen(date);
    
    mysql_stmt_bind_param(stmt, bind);
    
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Execute failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return result;
    }
    
    memset(result_bind, 0, sizeof(result_bind));
    
    result_bind[0].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[0].buffer = &temperature;
    result_bind[0].is_null = &is_null;
    
    mysql_stmt_bind_result(stmt, result_bind);
    
    if (mysql_stmt_fetch(stmt) == 0 && !is_null) {
        result.value = temperature;
        result.is_valid = 1;
    }
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return result;
}

int main() {
    // Test case 1
    TemperatureResult temp1 = temperature_for_location(40.7128, -74.0060, "2024-01-15");
    printf("Test 1 - Temperature: %s\\n", temp1.is_valid ? "" : "Not found");
    if (temp1.is_valid) printf("%f\\n", temp1.value);
    
    // Test case 2
    TemperatureResult temp2 = temperature_for_location(34.0522, -118.2437, "2024-02-20");
    printf("Test 2 - Temperature: %s\\n", temp2.is_valid ? "" : "Not found");
    if (temp2.is_valid) printf("%f\\n", temp2.value);
    
    // Test case 3
    TemperatureResult temp3 = temperature_for_location(51.5074, -0.1278, "2024-03-10");
    printf("Test 3 - Temperature: %s\\n", temp3.is_valid ? "" : "Not found");
    if (temp3.is_valid) printf("%f\\n", temp3.value);
    
    // Test case 4
    TemperatureResult temp4 = temperature_for_location(35.6762, 139.6503, "2024-04-05");
    printf("Test 4 - Temperature: %s\\n", temp4.is_valid ? "" : "Not found");
    if (temp4.is_valid) printf("%f\\n", temp4.value);
    
    // Test case 5
    TemperatureResult temp5 = temperature_for_location(-33.8688, 151.2093, "2024-05-12");
    printf("Test 5 - Temperature: %s\\n", temp5.is_valid ? "" : "Not found");
    if (temp5.is_valid) printf("%f\\n", temp5.value);
    
    return 0;
}
