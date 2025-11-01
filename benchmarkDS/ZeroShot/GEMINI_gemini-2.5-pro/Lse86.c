/*
 * INSTRUCTIONS:
 * 1. This code requires 'libmicrohttpd' and the MySQL C Connector library.
 * 2. Install the required development libraries.
 *    On Debian/Ubuntu: sudo apt-get install libmicrohttpd-dev libmysqlclient-dev
 *    On RedHat/CentOS: sudo yum install libmicrohttpd-devel mysql-devel
 * 3. Compile the code with:
 *    gcc Lse86.c -o Lse86 -lmicrohttpd -lmysqlclient -std=c99
 * 4. Set database credentials before running.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <microhttpd.h>
#include <regex.h>

#define PORT 8082
#define API_ENDPOINT "/api/temperature_for_location"

// --- Database Configuration ---
const char* DB_HOST = "localhost";
const char* DB_USER = "your_username";
const char* DB_PASS = "your_password";
const char* DB_NAME = "weatherdb";
const unsigned int DB_PORT = 3306;

struct connection_info_struct {
    char* response_data;
    int response_code;
};

// Function to get a parameter value from the URL query string
const char* get_param(void *cls, enum MHD_ValueKind kind, const char *key, const char *value) {
    if (strcmp(key, (const char*)cls) == 0) {
        // Return the value itself to stop iteration
        return value;
    }
    // Return NULL to continue iterating
    return NULL;
}

int send_response(struct MHD_Connection *connection, int status_code, const char *content) {
    struct MHD_Response *response;
    int ret;

    response = MHD_create_response_from_buffer(strlen(content), (void *)content, MHD_RESPMEM_MUST_COPY);
    if (!response) {
        return MHD_NO;
    }
    MHD_add_response_header(response, "Content-Type", "application/json");

    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret;
}


int answer_to_connection(void *cls, struct MHD_Connection *connection,
                         const char *url, const char *method,
                         const char *version, const char *upload_data,
                         size_t *upload_data_size, void **con_cls) {

    if (strcmp(method, "GET") != 0 || strcmp(url, API_ENDPOINT) != 0) {
        return send_response(connection, MHD_HTTP_NOT_FOUND, "{\"error\":\"Not Found\"}");
    }

    // --- Parameter Extraction and Validation ---
    const char *lat_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "latitude");
    const char *lon_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "longitude");
    const char *year_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "year");
    const char *month_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "month");
    const char *day_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "day");
    const char *grib_file = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "grib_file");

    if (!lat_str || !lon_str || !year_str || !month_str || !day_str || !grib_file) {
        return send_response(connection, MHD_HTTP_BAD_REQUEST, "{\"error\":\"Missing required parameters.\"}");
    }

    // Validate input values
    char *endptr;
    double lat = strtod(lat_str, &endptr);
    if (*endptr != '\0' || lat < -90.0 || lat > 90.0) return send_response(connection, MHD_HTTP_BAD_REQUEST, "{\"error\":\"Invalid latitude.\"}");
    
    double lon = strtod(lon_str, &endptr);
    if (*endptr != '\0' || lon < -180.0 || lon > 180.0) return send_response(connection, MHD_HTTP_BAD_REQUEST, "{\"error\":\"Invalid longitude.\"}");

    long year = strtol(year_str, &endptr, 10);
    if (*endptr != '\0') return send_response(connection, MHD_HTTP_BAD_REQUEST, "{\"error\":\"Invalid year.\"}");
    long month = strtol(month_str, &endptr, 10);
    if (*endptr != '\0' || month < 1 || month > 12) return send_response(connection, MHD_HTTP_BAD_REQUEST, "{\"error\":\"Invalid month.\"}");
    long day = strtol(day_str, &endptr, 10);
    if (*endptr != '\0' || day < 1 || day > 31) return send_response(connection, MHD_HTTP_BAD_REQUEST, "{\"error\":\"Invalid day.\"}");

    regex_t regex;
    int reti = regcomp(&regex, "^[A-Za-z0-9_.-]+$", REG_EXTENDED);
    if (reti) { return send_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "{\"error\":\"Regex compilation failed.\"}"); }
    reti = regexec(&regex, grib_file, 0, NULL, 0);
    regfree(&regex);
    if (reti == REG_NOMATCH) {
        return send_response(connection, MHD_HTTP_BAD_REQUEST, "{\"error\":\"Invalid grib_file format.\"}");
    }
    
    // --- Database Operation ---
    MYSQL *conn = mysql_init(NULL);
    if (!conn) {
        return send_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "{\"error\":\"mysql_init failed\"}");
    }
    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0) == NULL) {
        mysql_close(conn);
        return send_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "{\"error\":\"Database connection failed\"}");
    }

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt) {
        mysql_close(conn);
        return send_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "{\"error\":\"mysql_stmt_init failed\"}");
    }
    
    const char* sql = "SELECT MAX(temperature) FROM temperature_data WHERE latitude = ? AND longitude = ? AND obs_year = ? AND obs_month = ? AND obs_day = ? AND grib_file = ?";
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return send_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "{\"error\":\"SQL prepare failed\"}");
    }

    MYSQL_BIND params[6];
    memset(params, 0, sizeof(params));
    params[0].buffer_type = MYSQL_TYPE_DOUBLE; params[0].buffer = (char*)&lat;
    params[1].buffer_type = MYSQL_TYPE_DOUBLE; params[1].buffer = (char*)&lon;
    params[2].buffer_type = MYSQL_TYPE_LONG;   params[2].buffer = (char*)&year;
    params[3].buffer_type = MYSQL_TYPE_LONG;   params[3].buffer = (char*)&month;
    params[4].buffer_type = MYSQL_TYPE_LONG;   params[4].buffer = (char*)&day;
    params[5].buffer_type = MYSQL_TYPE_STRING; params[5].buffer = (char*)grib_file; params[5].buffer_length = strlen(grib_file);

    mysql_stmt_bind_param(stmt, params);
    mysql_stmt_execute(stmt);

    double max_temp;
    my_bool is_null;
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_DOUBLE;
    result[0].buffer = (char*)&max_temp;
    result[0].is_null = &is_null;
    
    mysql_stmt_bind_result(stmt, result);

    int status_code;
    char response_buf[256];
    if (mysql_stmt_fetch(stmt) == 0) {
        if (is_null) {
            snprintf(response_buf, sizeof(response_buf), "{\"status\":\"not_found\", \"message\":\"No temperature data found.\"}");
        } else {
            snprintf(response_buf, sizeof(response_buf), "{\"status\":\"success\", \"max_temperature\":%.2f}", max_temp);
        }
        status_code = MHD_HTTP_OK;
    } else {
        snprintf(response_buf, sizeof(response_buf), "{\"status\":\"not_found\", \"message\":\"No temperature data found.\"}");
        status_code = MHD_HTTP_OK;
    }

    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return send_response(connection, status_code, response_buf);
}


int main() {
    struct MHD_Daemon *daemon;

    printf("Starting C server on http://localhost:%d\n", PORT);

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start daemon\n");
        return 1;
    }

    printf("\n--- TEST CASES (run in a separate terminal) ---\n");
    char base_url[128];
    snprintf(base_url, sizeof(base_url), "http://localhost:%d%s", PORT, API_ENDPOINT);
    printf("1. curl \"%s?latitude=34.0522&longitude=-118.2437&year=2023&month=7&day=15&grib_file=forecast.grib2\"\n", base_url);
    printf("2. curl \"%s?latitude=0&longitude=0&year=1900&month=1&day=1&grib_file=no_data.grib\"\n", base_url);
    printf("3. curl \"%s?latitude=100.0&longitude=-118.2437&year=2023&month=7&day=15&grib_file=forecast.grib2\"\n", base_url);
    printf("4. curl \"%s?latitude=34.0522&longitude=-118.2437&year=2023&month=7&grib_file=forecast.grib2\"\n", base_url);
    printf("5. curl \"%s?latitude=34.0522&longitude=-118.2437&year=2023&month=7&day=15&grib_file=../../etc/passwd\"\n", base_url);
    printf("-------------------------------------------------\n");

    printf("Server running. Press Enter to exit.\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    return 0;
}