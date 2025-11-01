
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <microhttpd.h>
#include <ctype.h>

#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASSWORD "password"
#define DB_NAME "weather_db"
#define MAX_STRING_LEN 100

typedef struct {
    double latitude;
    double longitude;
    int year;
    int month;
    int day;
    char grib_file[MAX_STRING_LEN];
} TemperatureParams;

int validate_double(const char* value, double min, double max, double* result) {
    if (!value) return 0;
    char* endptr;
    *result = strtod(value, &endptr);
    if (*endptr != '\\0') return 0;
    return (*result >= min && *result <= max);
}

int validate_int(const char* value, int min, int max, int* result) {
    if (!value) return 0;
    char* endptr;
    long temp = strtol(value, &endptr, 10);
    if (*endptr != '\\0') return 0;
    *result = (int)temp;
    return (*result >= min && *result <= max);
}

int validate_string(const char* value, char* result, size_t max_length) {
    if (!value || strlen(value) > max_length) return 0;
    
    size_t j = 0;
    for (size_t i = 0; value[i] != '\\0' && j < max_length - 1; i++) {
        if (isalnum(value[i]) || value[i] == '.' || value[i] == '_' || value[i] == '-') {
            result[j++] = value[i];
        }
    }
    result[j] = '\\0';
    return j > 0;
}

double get_max_temperature(double latitude, double longitude, int year, 
                          int month, int day, const char* grib_file) {
    MYSQL* conn = mysql_init(NULL);
    double result = -999.0;
    
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "MySQL connection error: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return result;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    const char* query = "SELECT MAX(temperature) as max_temp FROM temperature_data "
                       "WHERE latitude = ? AND longitude = ? AND year = ? "
                       "AND month = ? AND day = ? AND grib_file = ?";
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Statement prepare error: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return result;
    }
    
    MYSQL_BIND bind[6];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[0].buffer = (char*)&latitude;
    
    bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[1].buffer = (char*)&longitude;
    
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = (char*)&year;
    
    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = (char*)&month;
    
    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = (char*)&day;
    
    unsigned long grib_len = strlen(grib_file);
    bind[5].buffer_type = MYSQL_TYPE_STRING;
    bind[5].buffer = (char*)grib_file;
    bind[5].buffer_length = grib_len;
    bind[5].length = &grib_len;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "Bind error: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return result;
    }
    
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Execute error: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return result;
    }
    
    MYSQL_BIND result_bind;
    double max_temp;
    my_bool is_null;
    
    memset(&result_bind, 0, sizeof(result_bind));
    result_bind.buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind.buffer = (char*)&max_temp;
    result_bind.is_null = &is_null;
    
    if (mysql_stmt_bind_result(stmt, &result_bind)) {
        fprintf(stderr, "Bind result error: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return result;
    }
    
    if (mysql_stmt_fetch(stmt) == 0 && !is_null) {
        result = max_temp;
    }
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return result;
}

int handle_temperature_request(void* cls, struct MHD_Connection* connection,
                               const char* url, const char* method,
                               const char* version, const char* upload_data,
                               size_t* upload_data_size, void** con_cls) {
    if (strcmp(method, "GET") != 0) {
        return MHD_NO;
    }
    
    if (strcmp(url, "/api/temperature_for_location") != 0) {
        return MHD_NO;
    }
    
    TemperatureParams params;
    
    const char* lat_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "latitude");
    const char* lon_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "longitude");
    const char* year_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "year");
    const char* month_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "month");
    const char* day_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "day");
    const char* grib_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "grib_file");
    
    if (!validate_double(lat_str, -90, 90, &params.latitude) ||
        !validate_double(lon_str, -180, 180, &params.longitude) ||
        !validate_int(year_str, 1900, 2100, &params.year) ||
        !validate_int(month_str, 1, 12, &params.month) ||
        !validate_int(day_str, 1, 31, &params.day) ||
        !validate_string(grib_str, params.grib_file, MAX_STRING_LEN)) {
        
        const char* error = "{\\"error\\": \\"Invalid parameters\\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(
            strlen(error), (void*)error, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, 400, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    double max_temp = get_max_temperature(params.latitude, params.longitude, 
                                         params.year, params.month, params.day, 
                                         params.grib_file);
    
    char response_buffer[256];
    int status_code;
    
    if (max_temp != -999.0) {
        snprintf(response_buffer, sizeof(response_buffer), 
                "{\\"max_temperature\\": %.2f}", max_temp);
        status_code = 200;
    } else {
        snprintf(response_buffer, sizeof(response_buffer), 
                "{\\"error\\": \\"No data found\\"}");
        status_code = 404;
    }
    
    struct MHD_Response* response = MHD_create_response_from_buffer(
        strlen(response_buffer), (void*)response_buffer, MHD_RESPMEM_MUST_COPY);
    int ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    
    return ret;
}

void run_test_cases(void) {
    printf("\\n=== Test Cases ===\\n");
    
    double result1 = get_max_temperature(40.7128, -74.0060, 2023, 6, 15, "test.grb");
    printf("Test 1 - NYC coordinates: %.2f\\n", result1);
    
    double result2 = get_max_temperature(51.5074, -0.1278, 2023, 7, 20, "test.grb");
    printf("Test 2 - London coordinates: %.2f\\n", result2);
    
    double result3 = get_max_temperature(35.6762, 139.6503, 2023, 8, 10, "test.grb");
    printf("Test 3 - Tokyo coordinates: %.2f\\n", result3);
    
    double result4 = get_max_temperature(-33.8688, 151.2093, 2023, 12, 25, "test.grb");
    printf("Test 4 - Sydney coordinates: %.2f\\n", result4);
    
    double result5 = get_max_temperature(48.8566, 2.3522, 2023, 5, 1, "test.grb");
    printf("Test 5 - Paris coordinates: %.2f\\n", result5);
}

int main(void) {
    run_test_cases();
    
    struct MHD_Daemon* daemon = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY, 8080, NULL, NULL,
        &handle_temperature_request, NULL, MHD_OPTION_END);
    
    if (daemon == NULL) {
        fprintf(stderr, "Failed to start server\\n");
        return 1;
    }
    
    printf("Server started on port 8080\\n");
    printf("Press Enter to stop the server...\\n");
    getchar();
    
    MHD_stop_daemon(daemon);
    return 0;
}
