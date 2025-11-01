// This requires the cpp-httplib and mysql-client libraries.
// httplib.h should be in the include path. Get it from: https://github.com/yhirose/cpp-httplib
#define CPPHTTPLIB_OPENSSL_SUPPORT // For HTTPS support (optional)
#include "httplib.h" 

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <regex>
#include <stdexcept>
#include <mysql/mysql.h>

// --- Helper Functions ---
bool is_valid_grib_file(const std::string& filename) {
    if (filename.length() > 255 || filename.empty()) return false;
    // Basic validation: allows alphanumeric, underscore, hyphen, and period.
    // Disallows directory traversal characters.
    static const std::regex pattern("^[a-zA-Z0-9_.-]+$");
    return std::regex_match(filename, pattern);
}

bool is_valid_date(int year, int month, int day) {
    if (year < 1900 || year > 2100) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) return false;
    if (month == 2) {
        bool is_leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        if (is_leap && day > 29) return false;
        if (!is_leap && day > 28) return false;
    }
    return true;
}

void handle_request(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. Extract and validate parameters
        if (!req.has_param("latitude") || !req.has_param("longitude") || !req.has_param("year") ||
            !req.has_param("month") || !req.has_param("day") || !req.has_param("grib_file")) {
            res.set_content("{\"error\":\"Missing one or more required parameters.\"}", "application/json");
            res.status = 400;
            return;
        }

        double latitude = std::stod(req.get_param_value("latitude"));
        double longitude = std::stod(req.get_param_value("longitude"));
        int year = std::stoi(req.get_param_value("year"));
        int month = std::stoi(req.get_param_value("month"));
        int day = std::stoi(req.get_param_value("day"));
        std::string grib_file = req.get_param_value("grib_file");

        if (latitude < -90.0 || latitude > 90.0) throw std::invalid_argument("Invalid latitude.");
        if (longitude < -180.0 || longitude > 180.0) throw std::invalid_argument("Invalid longitude.");
        if (!is_valid_date(year, month, day)) throw std::invalid_argument("Invalid date.");
        if (!is_valid_grib_file(grib_file)) throw std::invalid_argument("Invalid grib_file format or content.");

        // 2. Get DB credentials from environment variables
        const char* db_user_env = std::getenv("DB_USER");
        const char* db_pass_env = std::getenv("DB_PASS");
        if (!db_user_env || !db_pass_env) {
            std::cerr << "Error: DB_USER and DB_PASS environment variables must be set." << std::endl;
            res.set_content("{\"error\":\"Server configuration error.\"}", "application/json");
            res.status = 500;
            return;
        }
        const char* db_host_env = std::getenv("DB_HOST");
        const char* db_port_env = std::getenv("DB_PORT");
        const char* db_name_env = std::getenv("DB_NAME");
        
        std::string db_host = db_host_env ? db_host_env : "localhost";
        int db_port = db_port_env ? std::stoi(db_port_env) : 3306;
        std::string db_name = db_name_env ? db_name_env : "weather_db";
        
        // 3. Connect to database
        MYSQL* conn = mysql_init(NULL);
        if (!conn) {
             throw std::runtime_error("mysql_init failed");
        }
        if (!mysql_real_connect(conn, db_host.c_str(), db_user_env, db_pass_env, db_name.c_str(), db_port, NULL, 0)) {
            std::string err_msg = "Failed to connect to database: " + std::string(mysql_error(conn));
            mysql_close(conn);
            throw std::runtime_error(err_msg);
        }

        // 4. Use prepared statements
        const char* sql = "SELECT MAX(temperature) FROM weather_data WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? AND day = ? AND grib_file = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) { mysql_close(conn); throw std::runtime_error("mysql_stmt_init failed"); }

        if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
            std::string err_msg = "mysql_stmt_prepare failed: " + std::string(mysql_stmt_error(stmt));
            mysql_stmt_close(stmt); mysql_close(conn); throw std::runtime_error(err_msg);
        }

        MYSQL_BIND params[6];
        memset(params, 0, sizeof(params));
        params[0] = { .buffer_type = MYSQL_TYPE_DOUBLE, .buffer = &latitude };
        params[1] = { .buffer_type = MYSQL_TYPE_DOUBLE, .buffer = &longitude };
        params[2] = { .buffer_type = MYSQL_TYPE_LONG, .buffer = &year };
        params[3] = { .buffer_type = MYSQL_TYPE_LONG, .buffer = &month };
        params[4] = { .buffer_type = MYSQL_TYPE_LONG, .buffer = &day };
        params[5] = { .buffer_type = MYSQL_TYPE_STRING, .buffer = (char*)grib_file.c_str(), .buffer_length = (unsigned long)grib_file.length() };

        if (mysql_stmt_bind_param(stmt, params)) {
             std::string err_msg = "mysql_stmt_bind_param failed: " + std::string(mysql_stmt_error(stmt));
             mysql_stmt_close(stmt); mysql_close(conn); throw std::runtime_error(err_msg);
        }
        if (mysql_stmt_execute(stmt)) {
             std::string err_msg = "mysql_stmt_execute failed: " + std::string(mysql_stmt_error(stmt));
             mysql_stmt_close(stmt); mysql_close(conn); throw std::runtime_error(err_msg);
        }
        
        // 5. Fetch result
        double max_temp;
        my_bool is_null;
        MYSQL_BIND result_bind[1];
        memset(result_bind, 0, sizeof(result_bind));
        result_bind[0] = { .buffer_type = MYSQL_TYPE_DOUBLE, .buffer = &max_temp, .is_null = &is_null };

        mysql_stmt_bind_result(stmt, result_bind);
        int fetch_status = mysql_stmt_fetch(stmt);

        if (fetch_status == 0) { // Success
            if (is_null) {
                res.set_content("{\"message\":\"No temperature data found for the given parameters.\"}", "application/json");
                res.status = 404;
            } else {
                char response_body[64];
                snprintf(response_body, sizeof(response_body), "{\"max_temperature\": %.2f}", max_temp);
                res.set_content(response_body, "application/json");
                res.status = 200;
            }
        } else if (fetch_status == MYSQL_NO_DATA) {
            res.set_content("{\"message\":\"No temperature data found for the given parameters.\"}", "application/json");
            res.status = 404;
        } else { // Error
             throw std::runtime_error("mysql_stmt_fetch failed: " + std::string(mysql_stmt_error(stmt)));
        }

        mysql_stmt_close(stmt);
        mysql_close(conn);
    } catch (const std::invalid_argument& e) {
        res.set_content("{\"error\":\"Invalid parameter: " + std::string(e.what()) + "\"}", "application/json");
        res.status = 400;
    } catch (const std::exception& e) {
        std::cerr << "Internal Error: " << e.what() << std::endl;
        res.set_content("{\"error\":\"An internal server error occurred.\"}", "application/json");
        res.status = 500;
    }
}

int main() {
    httplib::Server svr;
    svr.Get("/api/temperature_for_location", handle_request);
    int port = 8080;
    std::cout << "Server starting on port " << port << "..." << std::endl;
    std::cout << "Use 'curl' to test. Examples:" << std::endl;
    std::cout << "1. Success: curl \"http://localhost:8080/api/temperature_for_location?latitude=34.0522&longitude=-118.2437&year=2023&month=10&day=26&grib_file=gfs.t00z.pgrb2.0p25.f000\"" << std::endl;
    std::cout << "2. Invalid Latitude: curl \"http://localhost:8080/api/temperature_for_location?latitude=999&longitude=-118.2437&year=2023&month=10&day=26&grib_file=file.grib\"" << std::endl;
    std::cout << "3. Invalid Date: curl \"http://localhost:8080/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=13&day=27&grib_file=file.grib\"" << std::endl;
    std::cout << "4. Invalid Grib File: curl \"http://localhost:8080/api/temperature_for_location?latitude=51.5074&longitude=-0.1278&year=2024&month=1&day=1&grib_file=../etc/passwd\"" << std::endl;
    std::cout << "5. No Data Found: curl \"http://localhost:8080/api/temperature_for_location?latitude=1.0&longitude=1.0&year=2025&month=1&day=1&grib_file=no_data.grb\"" << std::endl;
    /* To compile and run:
     * 1. Install MySQL C Connector (e.g., sudo apt-get install libmysqlclient-dev)
     * 2. Download httplib.h and place it in your include path.
     * 3. Compile: g++ -std=c++17 your_file.cpp -o server `mysql_config --cflags --libs` -pthread -lssl -lcrypto
     * Set environment variables for DB connection: export DB_USER="user" DB_PASS="pass"
     * 4. Run: ./server */
    svr.listen("0.0.0.0", port);
    return 0;
}