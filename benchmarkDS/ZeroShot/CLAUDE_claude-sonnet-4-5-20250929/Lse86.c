
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <microhttpd.h>

#define PORT 8080
#define DB_HOST "localhost"
#define DB_USER "your_username"
#define DB_PASSWORD "your_password"
#define DB_NAME "weatherdb"

typedef struct {
    char* latitude;
    char* longitude;
    char* year;
    char* month;
    char* day;
    char* grib_file;
} RequestParams;

double getMaxTemperature(const char* latitude, const char* longitude,
                         const char* year, const char* month,
                         const char* day, const char* grib_file) {
    MYSQL* conn = mysql_init(NULL);
    
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "Database connection failed: %s\\n", mysql_error(conn));
        return -999.0;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    const char* query = "SELECT MAX(temperature) as max_temp FROM temperature_data "
                       "WHERE latitude = ? AND longitude = ? AND year = ? "
                       "AND month = ? AND day = ? AND grib_file = ?";
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Statement preparation failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return -999.0;
    }
    
    MYSQL_BIND bind[6];
    memset(bind, 0, sizeof(bind));
    
    double lat = atof(latitude);
    double lon = atof(longitude);
    int yr = atoi(year);
    int mn = atoi(month);
    int dy = atoi(day);
    unsigned long grib_len = strlen(grib_file);
    
    bind[0].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[0].buffer = &lat;
    
    bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[1].buffer = &lon;
    
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = &yr;
    
    bind[3].buffer_type = MYSQL_TYPE_LONG;
    bind[3].buffer = &mn;
    
    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = &dy;
    
    bind[5].buffer_type = MYSQL_TYPE_STRING;
    bind[5].buffer = (char*)grib_file;
    bind[5].buffer_length = grib_len;
    bind[5].length = &grib_len;
    
    mysql_stmt_bind_param(stmt, bind);
    mysql_stmt_execute(stmt);
    
    double max_temp = -999.0;
    MYSQL_BIND result_bind;
    memset(&result_bind, 0, sizeof(result_bind));
    result_bind.buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind.buffer = &max_temp;
    
    mysql_stmt_bind_result(stmt, &result_bind);
    
    if (mysql_stmt_fetch(stmt) == 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return max_temp;
    }
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    return -999.0;
}

static int answer_to_connection(void* cls, struct MHD_Connection* connection,
                                const char* url, const char* method,
                                const char* version, const char* upload_data,
                                size_t* upload_data_size, void** con_cls) {
    
    if (strcmp(method, "GET") != 0 || strcmp(url, "/api/temperature_for_location") != 0) {
        return MHD_NO;
    }
    
    const char* latitude = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "latitude");
    const char* longitude = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "longitude");
    const char* year = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "year");
    const char* month = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "month");
    const char* day = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "day");
    const char* grib_file = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "grib_file");
    
    char response[256];
    struct MHD_Response* mhd_response;
    int ret;
    
    if (!latitude || !longitude || !year || !month || !day || !grib_file) {
        strcpy(response, "{\\"error\\": \\"Missing required parameters\\"}");
        mhd_response = MHD_create_response_from_buffer(strlen(response), response, MHD_RESPMEM_MUST_COPY);
        ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, mhd_response);
    } else {
        double max_temp = getMaxTemperature(latitude, longitude, year, month, day, grib_file);
        
        if (max_temp != -999.0) {
            sprintf(response, "{\\"max_temperature\\": %.2f}", max_temp);
            mhd_response = MHD_create_response_from_buffer(strlen(response), response, MHD_RESPMEM_MUST_COPY);
            ret = MHD_queue_response(connection, MHD_HTTP_OK, mhd_response);
        } else {
            strcpy(response, "{\\"error\\": \\"No data found\\"}");
            mhd_response = MHD_create_response_from_buffer(strlen(response), response, MHD_RESPMEM_MUST_COPY);
            ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, mhd_response);
        }
    }
    
    MHD_add_response_header(mhd_response, "Content-Type", "application/json");
    MHD_destroy_response(mhd_response);
    return ret;
}

int main() {
    struct MHD_Daemon* daemon;
    
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                             &answer_to_connection, NULL, MHD_OPTION_END);
    
    if (daemon == NULL) {
        fprintf(stderr, "Failed to start server\\n");
        return 1;
    }
    
    printf("Server listening on http://localhost:%d/api/temperature_for_location\\n", PORT);
    printf("\\nTest cases:\\n");
    printf("1. GET http://localhost:%d/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=1&day=15&grib_file=test.grib\\n", PORT);
    printf("2. GET http://localhost:%d/api/temperature_for_location?latitude=51.5074&longitude=-0.1278&year=2023&month=6&day=20&grib_file=test2.grib\\n", PORT);
    printf("3. GET http://localhost:%d/api/temperature_for_location?latitude=35.6762&longitude=139.6503&year=2023&month=12&day=25&grib_file=test3.grib\\n", PORT);
    printf("4. GET http://localhost:%d/api/temperature_for_location?latitude=48.8566&longitude=2.3522&year=2023&month=3&day=10&grib_file=test4.grib\\n", PORT);
    printf("5. GET http://localhost:%d/api/temperature_for_location?latitude=-33.8688&longitude=151.2093&year=2023&month=9&day=5&grib_file=test5.grib\\n", PORT);
    printf("\\nPress Enter to stop the server...\\n");
    
    getchar();
    
    MHD_stop_daemon(daemon);
    return 0;
}
