/*
NOTE:
1. THIS IS A COMPLEX EXAMPLE. C does not have built-in support for HTTP or database connections.
   This code requires external libraries:
   - libmicrohttpd: A small C library for running an HTTP server.
   - libmysqlclient: The MySQL C client library.

2. Installation (example on Debian/Ubuntu):
   sudo apt-get install libmicrohttpd-dev libmysqlclient-dev

3. Compilation:
   gcc -o Lse86_server Lse86.c -lmicrohttpd -lmysqlclient

4. Before running, set up a MySQL database with the following table and credentials.
   - Database: weatherdb
   - User:     weather_user
   - Password: password
   - Table:
     CREATE TABLE weather_data (
         id INT AUTO_INCREMENT PRIMARY KEY,
         latitude DECIMAL(9, 6) NOT NULL,
         longitude DECIMAL(9, 6) NOT NULL,
         year INT NOT NULL,
         month INT NOT NULL,
         day INT NOT NULL,
         grib_file VARCHAR(255) NOT NULL,
         temperature DECIMAL(5, 2) NOT NULL
     );
     -- Add some sample data to the table for testing.
     INSERT INTO weather_data (latitude, longitude, year, month, day, grib_file, temperature) VALUES
     (34.05, -118.24, 2023, 7, 15, 'gfs.t00z.pgrb2.0p25.f000', 29.5),
     (34.05, -118.24, 2023, 7, 15, 'gfs.t00z.pgrb2.0p25.f000', 31.2),
     (40.71, -74.00, 2023, 8, 20, 'gfs.t06z.pgrb2.0p25.f001', 25.0),
     (48.85, 2.35, 2024, 1, 10, 'era5.12z.an.sfc', 5.5),
     (48.85, 2.35, 2024, 1, 10, 'era5.12z.an.sfc', 6.1);
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <mysql/mysql.h>

#define PORT 8080

// --- DATABASE CONFIGURATION ---
#define DB_HOST "localhost"
#define DB_USER "weather_user"
#define DB_PASS "password"
#define DB_NAME "weatherdb"

int answer_to_connection(void *cls, struct MHD_Connection *connection,
                         const char *url, const char *method,
                         const char *version, const char *upload_data,
                         size_t *upload_data_size, void **con_cls);

int main() {
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Server started on port %d\n", PORT);
    printf("--- 5 Test Cases (run in a separate terminal) ---\n");
    printf("1. curl \"http://localhost:%d/api/temperature_for_location?latitude=34.05&longitude=-118.24&year=2023&month=7&day=15&grib_file=gfs.t00z.pgrb2.0p25.f000\"\n", PORT);
    printf("2. curl \"http://localhost:%d/api/temperature_for_location?latitude=40.71&longitude=-74.00&year=2023&month=8&day=20&grib_file=gfs.t06z.pgrb2.0p25.f001\"\n", PORT);
    printf("3. curl \"http://localhost:%d/api/temperature_for_location?latitude=48.85&longitude=2.35&year=2024&month=1&day=10&grib_file=era5.12z.an.sfc\"\n", PORT);
    printf("4. curl \"http://localhost:%d/api/temperature_for_location?latitude=99.99&longitude=99.99&year=2025&month=1&day=1&grib_file=none\" # Should return not found\n", PORT);
    printf("5. curl \"http://localhost:%d/api/temperature_for_location?latitude=40.71\" # Should return bad request\n", PORT);
    
    printf("Press Enter to stop the server...\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    return 0;
}

int answer_to_connection(void *cls, struct MHD_Connection *connection,
                         const char *url, const char *method,
                         const char *version, const char *upload_data,
                         size_t *upload_data_size, void **con_cls) {
    
    const char *err_bad_request = "{\"error\": \"Bad Request: Missing or invalid parameters.\"}";
    const char *err_not_found = "{\"message\": \"Temperature data not found for the given criteria.\"}";
    const char *err_internal = "{\"error\": \"Internal Server Error.\"}";
    const char *err_method = "{\"error\": \"Method not allowed.\"}";

    struct MHD_Response *response;
    int ret;
    int status_code = MHD_HTTP_INTERNAL_SERVER_ERROR;
    const char* response_str = err_internal;

    if (strcmp(method, "GET") != 0) {
        response_str = err_method;
        status_code = MHD_HTTP_METHOD_NOT_ALLOWED;
    } else if (strcmp(url, "/api/temperature_for_location") == 0) {
        const char *lat_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "latitude");
        const char *lon_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "longitude");
        const char *year_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "year");
        const char *month_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "month");
        const char *day_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "day");
        const char *grib_file = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "grib_file");

        if (!lat_str || !lon_str || !year_str || !month_str || !day_str || !grib_file) {
            response_str = err_bad_request;
            status_code = MHD_HTTP_BAD_REQUEST;
        } else {
            // All params exist, now connect to DB
            MYSQL *conn = mysql_init(NULL);
            if (conn == NULL) {
                fprintf(stderr, "mysql_init() failed\n");
            } else if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0) == NULL) {
                fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
                mysql_close(conn);
            } else {
                char query[512];
                snprintf(query, sizeof(query),
                    "SELECT MAX(temperature) FROM weather_data "
                    "WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? AND day = ? AND grib_file = ?");

                MYSQL_STMT *stmt = mysql_stmt_init(conn);
                mysql_stmt_prepare(stmt, query, strlen(query));
                
                MYSQL_BIND params[6];
                memset(params, 0, sizeof(params));

                double lat = atof(lat_str);
                double lon = atof(lon_str);
                int year = atoi(year_str);
                int month = atoi(month_str);
                int day = atoi(day_str);
                
                params[0].buffer_type = MYSQL_TYPE_DOUBLE;
                params[0].buffer = (char *)&lat;
                params[1].buffer_type = MYSQL_TYPE_DOUBLE;
                params[1].buffer = (char *)&lon;
                params[2].buffer_type = MYSQL_TYPE_LONG;
                params[2].buffer = (char *)&year;
                params[3].buffer_type = MYSQL_TYPE_LONG;
                params[3].buffer = (char *)&month;
                params[4].buffer_type = MYSQL_TYPE_LONG;
                params[4].buffer = (char *)&day;
                params[5].buffer_type = MYSQL_TYPE_STRING;
                params[5].buffer = (char *)grib_file;
                params[5].buffer_length = strlen(grib_file);

                mysql_stmt_bind_param(stmt, params);
                mysql_stmt_execute(stmt);

                MYSQL_BIND result[1];
                memset(result, 0, sizeof(result));
                double max_temp;
                my_bool is_null;

                result[0].buffer_type = MYSQL_TYPE_DOUBLE;
                result[0].buffer = (char *)&max_temp;
                result[0].is_null = &is_null;

                mysql_stmt_bind_result(stmt, result);
                
                if (mysql_stmt_fetch(stmt) == 0) { // Success
                    if (is_null) {
                        response_str = err_not_found;
                        status_code = MHD_HTTP_NOT_FOUND;
                    } else {
                        static char temp_buffer[64];
                        snprintf(temp_buffer, sizeof(temp_buffer), "{\"max_temperature\": %.2f}", max_temp);
                        response_str = temp_buffer;
                        status_code = MHD_HTTP_OK;
                    }
                } else {
                    response_str = err_not_found; // No rows found
                    status_code = MHD_HTTP_NOT_FOUND;
                }

                mysql_stmt_close(stmt);
                mysql_close(conn);
            }
        }
    } else {
         response_str = "{\"error\": \"Not Found\"}";
         status_code = MHD_HTTP_NOT_FOUND;
    }

    response = MHD_create_response_from_buffer(strlen(response_str), (void *)response_str, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, "Content-Type", "application/json");
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    return ret;
}