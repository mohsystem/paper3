/*
 * Dependencies:
 * 1. libmicrohttpd: A small C library for running an HTTP server.
 *    On Debian/Ubuntu: sudo apt-get install libmicrohttpd-dev
 *    On RHEL/CentOS: sudo yum install libmicrohttpd-devel
 * 2. libmysqlclient: The MySQL C client library.
 *    On Debian/Ubuntu: sudo apt-get install libmysqlclient-dev
 *    On RHEL/CentOS: sudo yum install mysql-devel
 *
 * Database Setup:
 * CREATE DATABASE weather_db;
 * USE weather_db;
 * CREATE TABLE weather_data (
 *     id INT AUTO_INCREMENT PRIMARY KEY,
 *     latitude DECIMAL(9, 6) NOT NULL,
 *     longitude DECIMAL(9, 6) NOT NULL,
 *     year INT NOT NULL,
 *     month INT NOT NULL,
 *     day INT NOT NULL,
 *     grib_file VARCHAR(255) NOT NULL,
 *     temperature DECIMAL(5, 2) NOT NULL,
 *     INDEX idx_location_date (latitude, longitude, year, month, day, grib_file)
 * );
 *
 * How to compile:
 * gcc -o lse86_c_server Lse86.c -lmicrohttpd -lmysqlclient -std=c11
 *
 * How to run:
 * ./lse86_c_server
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <microhttpd.h>

#define PORT 8082

// In a real application, use a config file or environment variables.
const char* DB_HOST = "127.0.0.1";
const char* DB_USER = "your_user";
const char* DB_PASS = "your_password";
const char* DB_NAME = "weather_db";
const unsigned int DB_PORT = 3306;

struct ResponseData {
    char* data;
    int status_code;
};

// Callback for MHD_get_connection_values to iterate over query parameters
int get_query_param(void* cls, enum MHD_ValueKind kind, const char* key, const char* value) {
    // This is a simplified handler. We are processing specific keys in the main handler.
    // A more robust solution would use a map/hash table here.
    return MHD_YES;
}

// Main request handler
int answer_to_connection(void* cls, struct MHD_Connection* connection,
                         const char* url, const char* method,
                         const char* version, const char* upload_data,
                         size_t* upload_data_size, void** con_cls) {

    struct MHD_Response* response;
    int ret;
    struct ResponseData resp_data = {NULL, 500}; // Default to internal error

    if (strcmp(method, "GET") != 0 || strcmp(url, "/api/temperature_for_location") != 0) {
        const char* page = "{\"error\":\"Not Found\"}";
        response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/json");
        ret = MHD_queue_response(connection, 404, response);
        MHD_destroy_response(response);
        return ret;
    }

    // Extract parameters
    const char* lat_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "latitude");
    const char* lon_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "longitude");
    const char* year_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "year");
    const char* month_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "month");
    const char* day_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "day");
    const char* grib_file = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "grib_file");

    if (!lat_str || !lon_str || !year_str || !month_str || !day_str || !grib_file) {
        resp_data.data = strdup("{\"error\":\"Missing required parameters\"}");
        resp_data.status_code = 400;
    } else {
        char* end;
        double lat = strtod(lat_str, &end); if (*end) { resp_data.status_code = 400; }
        double lon = strtod(lon_str, &end); if (*end) { resp_data.status_code = 400; }
        long year = strtol(year_str, &end, 10); if (*end) { resp_data.status_code = 400; }
        long month = strtol(month_str, &end, 10); if (*end) { resp_data.status_code = 400; }
        long day = strtol(day_str, &end, 10); if (*end) { resp_data.status_code = 400; }

        if (resp_data.status_code == 400) {
            resp_data.data = strdup("{\"error\":\"Invalid data type for one or more parameters.\"}");
        } else {
            // Database Logic
            MYSQL* conn = mysql_init(NULL);
            if (!conn) {
                resp_data.data = strdup("{\"error\":\"mysql_init failed\"}");
            } else if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0)) {
                char err_buf[256];
                snprintf(err_buf, sizeof(err_buf), "{\"error\":\"DB Connection failed: %s\"}", mysql_error(conn));
                resp_data.data = strdup(err_buf);
                mysql_close(conn);
            } else {
                const char* sql = "SELECT MAX(temperature) FROM weather_data WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? AND day = ? AND grib_file = ?";
                MYSQL_STMT* stmt = mysql_stmt_init(conn);
                if (mysql_stmt_prepare(stmt, sql, strlen(sql)) == 0) {
                    MYSQL_BIND params[6];
                    memset(params, 0, sizeof(params));
                    params[0].buffer_type = MYSQL_TYPE_DOUBLE; params[0].buffer = &lat;
                    params[1].buffer_type = MYSQL_TYPE_DOUBLE; params[1].buffer = &lon;
                    params[2].buffer_type = MYSQL_TYPE_LONG; params[2].buffer = &year;
                    params[3].buffer_type = MYSQL_TYPE_LONG; params[3].buffer = &month;
                    params[4].buffer_type = MYSQL_TYPE_LONG; params[4].buffer = &day;
                    params[5].buffer_type = MYSQL_TYPE_STRING; params[5].buffer = (char*)grib_file; params[5].buffer_length = strlen(grib_file);

                    mysql_stmt_bind_param(stmt, params);
                    mysql_stmt_execute(stmt);

                    char result_data[32];
                    my_bool is_null;
                    MYSQL_BIND result_bind[1];
                    memset(result_bind, 0, sizeof(result_bind));
                    result_bind[0].buffer_type = MYSQL_TYPE_STRING;
                    result_bind[0].buffer = result_data;
                    result_bind[0].buffer_length = sizeof(result_data);
                    result_bind[0].is_null = &is_null;

                    mysql_stmt_bind_result(stmt, result_bind);
                    if (mysql_stmt_fetch(stmt) == 0) {
                        if (is_null) {
                            resp_data.data = strdup("{\"message\":\"No temperature data found for the specified criteria.\"}");
                            resp_data.status_code = 404;
                        } else {
                            char json_buf[128];
                            snprintf(json_buf, sizeof(json_buf), "{\"max_temperature\":%s}", result_data);
                            resp_data.data = strdup(json_buf);
                            resp_data.status_code = 200;
                        }
                    } else {
                         resp_data.data = strdup("{\"message\":\"No temperature data found for the specified criteria.\"}");
                         resp_data.status_code = 404;
                    }
                    mysql_stmt_close(stmt);
                }
                mysql_close(conn);
            }
        }
    }
    
    if (resp_data.data == NULL) {
        resp_data.data = strdup("{\"error\":\"Internal Server Error\"}");
        resp_data.status_code = 500;
    }

    response = MHD_create_response_from_buffer(strlen(resp_data.data), resp_data.data, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, "Content-Type", "application/json");
    ret = MHD_queue_response(connection, resp_data.status_code, response);
    MHD_destroy_response(response);
    
    return ret;
}

int main() {
    struct MHD_Daemon* daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server.\n");
        return 1;
    }

    printf("Server starting on port %d\n", PORT);
    printf("API endpoint available at: http://localhost:%d/api/temperature_for_location\n", PORT);

    printf("\n--- How to Test ---\n");
    printf("Use a tool like curl to send requests to the running server.\n");
    printf("1. Valid request with data:\n");
    printf("   curl \"http://localhost:8082/api/temperature_for_location?latitude=34.0522&longitude=-118.2437&year=2023&month=10&day=26&grib_file=gfs.t00z.pgrb2.0p25.f000\"\n");
    printf("\n2. Request for data that doesn't exist:\n");
    printf("   curl \"http://localhost:8082/api/temperature_for_location?latitude=0&longitude=0&year=1900&month=1&day=1&grib_file=none\"\n");
    printf("\n3. Request with missing parameters:\n");
    printf("   curl \"http://localhost:8082/api/temperature_for_location?latitude=34.0522&longitude=-118.2437\"\n");
    printf("\n4. Request with invalid parameter type:\n");
    printf("   curl \"http://localhost:8082/api/temperature_for_location?latitude=invalid&longitude=-118.2437&year=2023&month=10&day=26&grib_file=file.grib\"\n");
    printf("\n5. Another valid request:\n");
    printf("   curl \"http://localhost:8082/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=10&day=27&grib_file=gfs.t00z.pgrb2.0p25.f001\"\n\n");
    
    printf("Press Enter to stop the server.\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    return 0;
}