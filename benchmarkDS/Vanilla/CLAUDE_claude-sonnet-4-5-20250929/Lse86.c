
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <mysql/mysql.h>

#define PORT 8080

typedef struct {
    const char* db_host;
    const char* db_user;
    const char* db_password;
    const char* db_name;
} DBConfig;

double get_max_temperature(DBConfig* config, const char* latitude, const char* longitude,
                          const char* year, const char* month, const char* day, 
                          const char* grib_file) {
    MYSQL* conn = mysql_init(NULL);
    double max_temp = -999.0;
    
    if (conn == NULL) {
        return max_temp;
    }
    
    if (mysql_real_connect(conn, config->db_host, config->db_user, 
                          config->db_password, config->db_name, 0, NULL, 0) == NULL) {
        mysql_close(conn);
        return max_temp;
    }
    
    char query[1024];
    snprintf(query, sizeof(query), 
             "SELECT MAX(temperature) as max_temp FROM temperature_data "
             "WHERE latitude = '%s' AND longitude = '%s' AND year = '%s' "
             "AND month = '%s' AND day = '%s' AND grib_file = '%s'",
             latitude, longitude, year, month, day, grib_file);
    
    if (mysql_query(conn, query) == 0) {
        MYSQL_RES* result = mysql_store_result(conn);
        if (result != NULL) {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row != NULL && row[0] != NULL) {
                max_temp = atof(row[0]);
            }
            mysql_free_result(result);
        }
    }
    
    mysql_close(conn);
    return max_temp;
}

static int handle_request(void* cls, struct MHD_Connection* connection,
                         const char* url, const char* method,
                         const char* version, const char* upload_data,
                         size_t* upload_data_size, void** con_cls) {
    
    if (strcmp(method, "GET") != 0) {
        return MHD_NO;
    }
    
    if (strcmp(url, "/api/temperature_for_location") != 0) {
        return MHD_NO;
    }
    
    const char* latitude = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "latitude");
    const char* longitude = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "longitude");
    const char* year = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "year");
    const char* month = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "month");
    const char* day = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "day");
    const char* grib_file = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "grib_file");
    
    DBConfig* config = (DBConfig*)cls;
    double max_temp = get_max_temperature(config, latitude, longitude, year, month, day, grib_file);
    
    char response[256];
    if (max_temp != -999.0) {
        snprintf(response, sizeof(response), "{\\"max_temperature\\": %.2f}", max_temp);
    } else {
        snprintf(response, sizeof(response), "{\\"error\\": \\"No data found\\"}");
    }
    
    struct MHD_Response* mhd_response = MHD_create_response_from_buffer(
        strlen(response), (void*)response, MHD_RESPMEM_MUST_COPY);
    
    MHD_add_response_header(mhd_response, "Content-Type", "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, mhd_response);
    MHD_destroy_response(mhd_response);
    
    return ret;
}

int main() {
    DBConfig config = {
        .db_host = "localhost",
        .db_user = "root",
        .db_password = "password",
        .db_name = "weather_db"
    };
    
    struct MHD_Daemon* daemon = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
        &handle_request, &config, MHD_OPTION_END);
    
    if (daemon == NULL) {
        printf("Failed to start server\\n");
        return 1;
    }
    
    printf("Server listening on port %d\\n", PORT);
    printf("Test cases (use curl):\\n");
    printf("1. curl 'http://localhost:8080/api/temperature_for_location?latitude=40.7&longitude=-74.0&year=2023&month=6&day=15&grib_file=data.grib'\\n");
    printf("2. curl 'http://localhost:8080/api/temperature_for_location?latitude=51.5&longitude=-0.1&year=2023&month=7&day=20&grib_file=data.grib'\\n");
    printf("3. curl 'http://localhost:8080/api/temperature_for_location?latitude=35.6&longitude=139.6&year=2023&month=8&day=10&grib_file=data.grib'\\n");
    printf("4. curl 'http://localhost:8080/api/temperature_for_location?latitude=48.8&longitude=2.3&year=2023&month=9&day=25&grib_file=data.grib'\\n");
    printf("5. curl 'http://localhost:8080/api/temperature_for_location?latitude=-33.8&longitude=151.2&year=2023&month=10&day=5&grib_file=data.grib'\\n");
    printf("Press Enter to stop server...\\n");
    
    getchar();
    
    MHD_stop_daemon(daemon);
    return 0;
}
