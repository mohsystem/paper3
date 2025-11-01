/*
 * INSTRUCTIONS:
 * 1. This code requires the 'cpp-httplib' and MySQL C Connector libraries.
 * 2. 'cpp-httplib' is a header-only library. Download 'httplib.h' from its GitHub repository
 *    and place it in the same directory or in your include path.
 * 3. Install the MySQL C Connector development library.
 *    On Debian/Ubuntu: sudo apt-get install libmysqlclient-dev
 *    On RedHat/CentOS: sudo yum install mysql-devel
 * 4. Compile the code with:
 *    g++ Lse86.cpp -o Lse86 -I/path/to/mysql/include -L/path/to/mysql/lib -lmysqlclient -lpthread -std=c++17
 *    (Adjust paths if MySQL is installed in a non-standard location)
 * 5. Set database credentials before running.
*/
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <mysql/mysql.h>
#define CPPHTTPLIB_OPENSSL_SUPPORT // Uncomment for HTTPS support if needed
#include "httplib.h" // Assumes httplib.h is in the include path or same directory

// --- Database Configuration ---
// Note: In production, use a more secure method for credentials.
const char* DB_HOST = "localhost";
const char* DB_USER = "your_username";
const char* DB_PASS = "your_password";
const char* DB_NAME = "weatherdb";
const unsigned int DB_PORT = 3306;

// --- Security ---
const std::regex GRIB_FILE_REGEX("^[\\w.-]+$");

// A RAII wrapper for MYSQL connection
struct MySQLConnection {
    MYSQL* conn;
    MySQLConnection() : conn(mysql_init(NULL)) {}
    ~MySQLConnection() {
        if (conn) mysql_close(conn);
    }
};

// A RAII wrapper for MYSQL_STMT
struct MySQLStatement {
    MYSQL_STMT* stmt;
    MySQLStatement(MYSQL* conn) : stmt(mysql_stmt_init(conn)) {}
    ~MySQLStatement() {
        if (stmt) mysql_stmt_close(stmt);
    }
};

// Function to fetch max temperature
std::string getMaxTemperature(double lat, double lon, int year, int month, int day, const std::string& grib_file) {
    MySQLConnection db_conn;
    if (!db_conn.conn) {
        return "{\"status\":\"error\", \"message\":\"mysql_init failed\"}";
    }

    if (mysql_real_connect(db_conn.conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0) == NULL) {
        std::string err = "{\"status\":\"error\", \"message\":\"Database connection failed: ";
        err += mysql_error(db_conn.conn);
        err += "\"}";
        return err;
    }

    MySQLStatement db_stmt(db_conn.conn);
    if (!db_stmt.stmt) {
        return "{\"status\":\"error\", \"message\":\"mysql_stmt_init failed\"}";
    }
    
    std::string sql = "SELECT MAX(temperature) FROM temperature_data "
                      "WHERE latitude = ? AND longitude = ? AND obs_year = ? AND obs_month = ? "
                      "AND obs_day = ? AND grib_file = ?";

    if (mysql_stmt_prepare(db_stmt.stmt, sql.c_str(), sql.length())) {
        std::string err = "{\"status\":\"error\", \"message\":\"SQL prepare failed: ";
        err += mysql_stmt_error(db_stmt.stmt);
        err += "\"}";
        return err;
    }
    
    MYSQL_BIND params[6];
    memset(params, 0, sizeof(params));

    params[0].buffer_type = MYSQL_TYPE_DOUBLE;
    params[0].buffer = (char*)&lat;
    params[1].buffer_type = MYSQL_TYPE_DOUBLE;
    params[1].buffer = (char*)&lon;
    params[2].buffer_type = MYSQL_TYPE_LONG;
    params[2].buffer = (char*)&year;
    params[3].buffer_type = MYSQL_TYPE_LONG;
    params[3].buffer = (char*)&month;
    params[4].buffer_type = MYSQL_TYPE_LONG;
    params[4].buffer = (char*)&day;
    params[5].buffer_type = MYSQL_TYPE_STRING;
    params[5].buffer = (char*)grib_file.c_str();
    params[5].buffer_length = grib_file.length();

    if (mysql_stmt_bind_param(db_stmt.stmt, params)) {
        return "{\"status\":\"error\", \"message\":\"SQL bind param failed\"}";
    }

    if (mysql_stmt_execute(db_stmt.stmt)) {
        return "{\"status\":\"error\", \"message\":\"SQL execute failed\"}";
    }

    double max_temp;
    my_bool is_null;
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_DOUBLE;
    result[0].buffer = (char*)&max_temp;
    result[0].is_null = &is_null;

    if (mysql_stmt_bind_result(db_stmt.stmt, result)) {
        return "{\"status\":\"error\", \"message\":\"SQL bind result failed\"}";
    }

    std::string response;
    if (mysql_stmt_fetch(db_stmt.stmt) == 0) { // Success
        if (is_null) {
            response = "{\"status\":\"not_found\", \"message\":\"No temperature data found for the specified criteria.\"}";
        } else {
            char buffer[100];
            snprintf(buffer, sizeof(buffer), "{\"status\":\"success\", \"max_temperature\":%.2f}", max_temp);
            response = buffer;
        }
    } else {
        response = "{\"status\":\"not_found\", \"message\":\"No temperature data found for the specified criteria.\"}";
    }

    return response;
}


int main() {
    // The main class is represented by the server instance.
    httplib::Server Lse86;

    Lse86.Get("/api/temperature_for_location", [](const httplib::Request& req, httplib::Response& res) {
        try {
            // Parameter validation
            if (!req.has_param("latitude") || !req.has_param("longitude") || !req.has_param("year") ||
                !req.has_param("month") || !req.has_param("day") || !req.has_param("grib_file")) {
                res.status = 400;
                res.set_content("{\"status\":\"error\", \"message\":\"Bad Request: Missing parameters.\"}", "application/json");
                return;
            }

            double lat = std::stod(req.get_param_value("latitude"));
            if (lat < -90.0 || lat > 90.0) throw std::out_of_range("Invalid latitude");

            double lon = std::stod(req.get_param_value("longitude"));
            if (lon < -180.0 || lon > 180.0) throw std::out_of_range("Invalid longitude");

            int year = std::stoi(req.get_param_value("year"));
            int month = std::stoi(req.get_param_value("month"));
            if (month < 1 || month > 12) throw std::out_of_range("Invalid month");

            int day = std::stoi(req.get_param_value("day"));
             if (day < 1 || day > 31) throw std::out_of_range("Invalid day");

            std::string grib_file = req.get_param_value("grib_file");
            if (!std::regex_match(grib_file, GRIB_FILE_REGEX)) {
                 throw std::runtime_error("Invalid grib_file format.");
            }

            std::string db_response = getMaxTemperature(lat, lon, year, month, day, grib_file);
            res.set_content(db_response, "application/json");

        } catch (const std::invalid_argument& e) {
            res.status = 400;
            res.set_content("{\"status\":\"error\", \"message\":\"Bad Request: Invalid number format for a parameter.\"}", "application/json");
        } catch (const std::out_of_range& e) {
            res.status = 400;
            res.set_content(std::string("{\"status\":\"error\", \"message\":\"Bad Request: ") + e.what() + "\"}", "application/json");
        } catch (const std::runtime_error& e) {
            res.status = 400;
            res.set_content(std::string("{\"status\":\"error\", \"message\":\"Bad Request: ") + e.what() + "\"}", "application/json");
        } catch (...) {
            res.status = 500;
            res.set_content("{\"status\":\"error\", \"message\":\"An unexpected internal server error occurred.\"}", "application/json");
        }
    });
    
    int port = 8081;
    std::cout << "Starting C++ server on http://localhost:" << port << std::endl;
    
    std::cout << "\n--- TEST CASES (run in a separate terminal) ---" << std::endl;
    std::string base_url = "http://localhost:" + std::to_string(port) + "/api/temperature_for_location";
    std::cout << "1. curl \"" << base_url << "?latitude=34.0522&longitude=-118.2437&year=2023&month=7&day=15&grib_file=forecast.grib2\"" << std::endl;
    std::cout << "2. curl \"" << base_url << "?latitude=0&longitude=0&year=1900&month=1&day=1&grib_file=no_data.grib\"" << std::endl;
    std::cout << "3. curl \"" << base_url << "?latitude=100.0&longitude=-118.2437&year=2023&month=7&day=15&grib_file=forecast.grib2\"" << std::endl;
    std::cout << "4. curl \"" << base_url << "?latitude=34.0522&longitude=-118.2437&year=2023&month=7&grib_file=forecast.grib2\"" << std::endl;
    std::cout << "5. curl \"" << base_url << "?latitude=34.0522&longitude=-118.2437&year=2023&month=7&day=15&grib_file=../../etc/passwd\"" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;

    Lse86.listen("0.0.0.0", port);

    return 0;
}