#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <mysql/mysql.h>

// This example assumes mongoose.h is in the include path
// and mongoose.c is compiled along with this file.
// Get them from: https://github.com/cesanta/mongoose
#include "mongoose.h"

#define MAX_GRIB_FILE_LENGTH 255
#define MAX_PARAM_LENGTH 64

// --- Validation Functions ---
static bool is_valid_grib_file(const char* filename) {
    if (filename == NULL || strlen(filename) > MAX_GRIB_FILE_LENGTH || strlen(filename) == 0) {
        return false;
    }
    for (size_t i = 0; i < strlen(filename); i++) {
        if (!isalnum((unsigned char)filename[i]) && filename[i] != '_' && filename[i] != '.' && filename[i] != '-') {
            return false;
        }
    }
    if (strstr(filename, "..") != NULL) {
        return false;
    }
    return true;
}

static bool is_valid_date(int year, int month, int day) {
    if (year < 1900 || year > 2100) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) return false;
    if (month == 2) {
        bool is_leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        if ((is_leap && day > 29) || (!is_leap && day > 28)) return false;
    }
    return true;
}

static void send_error(struct mg_connection *c, int status, const char *msg) {
    mg_http_reply(c, status, "Content-Type: application/json\r\n", "{\"error\":\"%s\"}\n", msg);
}

// --- Mongoose Event Handler ---
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_http_match_uri(hm, "/api/temperature_for_location")) {
            char lat_str[MAX_PARAM_LENGTH], lon_str[MAX_PARAM_LENGTH], year_str[MAX_PARAM_LENGTH], 
                 month_str[MAX_PARAM_LENGTH], day_str[MAX_PARAM_LENGTH], grib_file[MAX_GRIB_FILE_LENGTH + 1];
            double latitude, longitude;
            int year, month, day;

            if (mg_http_get_var(&hm->query, "latitude", lat_str, sizeof(lat_str)) <= 0 ||
                mg_http_get_var(&hm->query, "longitude", lon_str, sizeof(lon_str)) <= 0 ||
                mg_http_get_var(&hm->query, "year", year_str, sizeof(year_str)) <= 0 ||
                mg_http_get_var(&hm->query, "month", month_str, sizeof(month_str)) <= 0 ||
                mg_http_get_var(&hm->query, "day", day_str, sizeof(day_str)) <= 0 ||
                mg_http_get_var(&hm->query, "grib_file", grib_file, sizeof(grib_file)) <= 0) {
                return send_error(c, 400, "Missing required parameters.");
            }

            char *endptr;
            latitude = strtod(lat_str, &endptr);
            if (*endptr != '\0' || latitude < -90.0 || latitude > 90.0) return send_error(c, 400, "Invalid latitude.");
            longitude = strtod(lon_str, &endptr);
            if (*endptr != '\0' || longitude < -180.0 || longitude > 180.0) return send_error(c, 400, "Invalid longitude.");
            year = strtol(year_str, &endptr, 10); if (*endptr != '\0') return send_error(c, 400, "Invalid year format.");
            month = strtol(month_str, &endptr, 10); if (*endptr != '\0') return send_error(c, 400, "Invalid month format.");
            day = strtol(day_str, &endptr, 10); if (*endptr != '\0') return send_error(c, 400, "Invalid day format.");
            if (!is_valid_date(year, month, day)) return send_error(c, 400, "Invalid date.");
            if (!is_valid_grib_file(grib_file)) return send_error(c, 400, "Invalid grib_file.");
            
            const char* db_user = getenv("DB_USER"), *db_pass = getenv("DB_PASS");
            if (db_user == NULL || db_pass == NULL) {
                 fprintf(stderr, "Error: DB_USER and DB_PASS must be set.\n");
                 return send_error(c, 500, "Server configuration error.");
            }
            const char* db_host = getenv("DB_HOST") ? getenv("DB_HOST") : "localhost";
            const char* db_port_str = getenv("DB_PORT") ? getenv("DB_PORT") : "3306";
            const char* db_name = getenv("DB_NAME") ? getenv("DB_NAME") : "weather_db";

            MYSQL *conn = mysql_init(NULL);
            if (!conn) return send_error(c, 500, "Internal server error.");
            if (!mysql_real_connect(conn, db_host, db_user, db_pass, db_name, atoi(db_port_str), NULL, 0)) {
                fprintf(stderr, "DB Connect Error: %s\n", mysql_error(conn));
                mysql_close(conn);
                return send_error(c, 500, "Internal server error.");
            }
            
            const char *sql = "SELECT MAX(temperature) FROM weather_data WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? AND day = ? AND grib_file = ?";
            MYSQL_STMT *stmt = mysql_stmt_init(conn);
            mysql_stmt_prepare(stmt, sql, strlen(sql));

            MYSQL_BIND params[6];
            memset(params, 0, sizeof(params));
            params[0].buffer_type = MYSQL_TYPE_DOUBLE; params[0].buffer = &latitude;
            params[1].buffer_type = MYSQL_TYPE_DOUBLE; params[1].buffer = &longitude;
            params[2].buffer_type = MYSQL_TYPE_LONG;   params[2].buffer = &year;
            params[3].buffer_type = MYSQL_TYPE_LONG;   params[3].buffer = &month;
            params[4].buffer_type = MYSQL_TYPE_LONG;   params[4].buffer = &day;
            params[5].buffer_type = MYSQL_TYPE_STRING; params[5].buffer = grib_file; params[5].buffer_length = strlen(grib_file);

            mysql_stmt_bind_param(stmt, params);
            mysql_stmt_execute(stmt);

            double max_temp; my_bool is_null;
            MYSQL_BIND result_bind[1];
            memset(result_bind, 0, sizeof(result_bind));
            result_bind[0].buffer_type = MYSQL_TYPE_DOUBLE; result_bind[0].buffer = &max_temp; result_bind[0].is_null = &is_null;

            mysql_stmt_bind_result(stmt, result_bind);
            int fetch_status = mysql_stmt_fetch(stmt);

            if (fetch_status == 0 && !is_null) {
                mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"max_temperature\": %.2f}\n", max_temp);
            } else {
                mg_http_reply(c, 404, "Content-Type: application/json\r\n", "{\"message\":\"No data found.\"}\n");
            }
            
            mysql_stmt_close(stmt);
            mysql_close(conn);
        } else {
            mg_http_reply(c, 404, "", "{\"error\":\"Not Found\"}\n");
        }
    }
    (void) fn_data;
}


int main(void) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    const char *port = "8080";
    char addr[64];
    snprintf(addr, sizeof(addr), "http://0.0.0.0:%s", port);
    
    printf("Starting server on port %s\n", port);
    printf("Use 'curl' to test. Examples:\n");
    printf("1. Success: curl \"http://localhost:8080/api/temperature_for_location?latitude=34.0522&longitude=-118.2437&year=2023&month=10&day=26&grib_file=gfs.t00z.pgrb2.0p25.f000\"\n");
    printf("2. Invalid Latitude: curl \"http://localhost:8080/api/temperature_for_location?latitude=999&longitude=-118.2437&year=2023&month=10&day=26&grib_file=file.grib\"\n");
    printf("3. Invalid Date: curl \"http://localhost:8080/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=13&day=27&grib_file=file.grib\"\n");
    printf("4. Invalid Grib File: curl \"http://localhost:8080/api/temperature_for_location?latitude=51.5074&longitude=-0.1278&year=2024&month=1&day=1&grib_file=../etc/passwd\"\n");
    printf("5. No Data Found: curl \"http://localhost:8080/api/temperature_for_location?latitude=1.0&longitude=1.0&year=2025&month=1&day=1&grib_file=no_data.grb\"\n");
    /* To compile and run:
     * 1. Install MySQL C Connector (e.g., sudo apt-get install libmysqlclient-dev)
     * 2. Download mongoose.c and mongoose.h
     * 3. Compile: gcc -o server your_file.c mongoose.c `mysql_config --cflags --libs`
     * Set environment variables for DB connection: export DB_USER="user" DB_PASS="pass"
     * 4. Run: ./server
    */
    
    mg_http_listen(&mgr, addr, fn, NULL);
    for (;;) mg_mgr_poll(&mgr, 1000);
    mg_mgr_free(&mgr);
    return 0;
}