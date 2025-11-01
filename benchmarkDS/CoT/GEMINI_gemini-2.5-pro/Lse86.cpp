/*
 * Dependencies:
 * 1. cpp-httplib: A header-only cross-platform HTTP/HTTPS library.
 *    Download `httplib.h` from https://github.com/yhirose/cpp-httplib
 *    and place it in the same directory or in your include path.
 * 2. MySQL C Connector library (`libmysqlclient`).
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
 * g++ -o lse86_server Lse86.cpp -lmysqlclient -pthread -std=c++17
 *
 * How to run:
 * ./lse86_server
 */
#define CPPHTTPLIB_OPENSSL_SUPPORT // Comment out if not using HTTPS
#include "httplib.h"
#include <mysql/mysql.h>
#include <iostream>
#include <string>
#include <memory>

// In a real application, use a config file or environment variables.
const char* DB_HOST = "127.0.0.1";
const char* DB_USER = "your_user";
const char* DB_PASS = "your_password";
const char* DB_NAME = "weather_db";
const unsigned int DB_PORT = 3306;

class Lse86 {
public:
    static std::string findMaxTemperature(double lat, double lon, int year, int month, int day, const std::string& grib_file) {
        MYSQL* conn = mysql_init(NULL);
        if (!conn) {
            return "{\"error\":\"mysql_init failed\"}";
        }

        if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0)) {
            std::string err = "{\"error\":\"Database connection failed: " + std::string(mysql_error(conn)) + "\"}";
            mysql_close(conn);
            return err;
        }

        // Use prepared statements to prevent SQL injection
        const char* sql = "SELECT MAX(temperature) FROM weather_data "
                          "WHERE latitude = ? AND longitude = ? AND year = ? AND "
                          "month = ? AND day = ? AND grib_file = ?";
        
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            mysql_close(conn);
            return "{\"error\":\"mysql_stmt_init failed\"}";
        }

        if (mysql_stmt_prepare(stmt, sql, strlen(sql)) != 0) {
            std::string err = "{\"error\":\"Statement preparation failed: " + std::string(mysql_stmt_error(stmt)) + "\"}";
            mysql_stmt_close(stmt);
            mysql_close(conn);
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

        if (mysql_stmt_bind_param(stmt, params) != 0) {
            std::string err = "{\"error\":\"Parameter binding failed: " + std::string(mysql_stmt_error(stmt)) + "\"}";
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return err;
        }

        if (mysql_stmt_execute(stmt) != 0) {
            std::string err = "{\"error\":\"Statement execution failed: " + std::string(mysql_stmt_error(stmt)) + "\"}";
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return err;
        }
        
        MYSQL_BIND result_bind[1];
        memset(result_bind, 0, sizeof(result_bind));
        char result_data[32]; // Buffer for the max temperature (as string)
        my_bool is_null;

        result_bind[0].buffer_type = MYSQL_TYPE_STRING;
        result_bind[0].buffer = result_data;
        result_bind[0].buffer_length = sizeof(result_data);
        result_bind[0].is_null = &is_null;

        if (mysql_stmt_bind_result(stmt, result_bind) != 0) {
            std::string err = "{\"error\":\"Result binding failed: " + std::string(mysql_stmt_error(stmt)) + "\"}";
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return err;
        }

        std::string response;
        if (mysql_stmt_fetch(stmt) == 0) { // Fetch was successful
            if (is_null) {
                response = "{\"message\":\"No temperature data found for the specified criteria.\"}";
            } else {
                response = "{\"max_temperature\":" + std::string(result_data) + "}";
            }
        } else {
            response = "{\"message\":\"No temperature data found for the specified criteria.\"}";
        }
        
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return response;
    }
};

int main() {
    httplib::Server svr;

    svr.Get("/api/temperature_for_location", [](const httplib::Request& req, httplib::Response& res) {
        // Parameter validation
        if (!req.has_param("latitude") || !req.has_param("longitude") || !req.has_param("year") ||
            !req.has_param("month") || !req.has_param("day") || !req.has_param("grib_file")) {
            res.set_content("{\"error\":\"Missing required parameters\"}", "application/json");
            res.status = 400;
            return;
        }

        try {
            double lat = std::stod(req.get_param_value("latitude"));
            double lon = std::stod(req.get_param_value("longitude"));
            int year = std::stoi(req.get_param_value("year"));
            int month = std::stoi(req.get_param_value("month"));
            int day = std::stoi(req.get_param_value("day"));
            std::string grib_file = req.get_param_value("grib_file");

            std::string response_body = Lse86::findMaxTemperature(lat, lon, year, month, day, grib_file);
            
            if(response_body.find("error") != std::string::npos) {
                res.status = 500;
            } else if (response_body.find("No temperature data") != std::string::npos) {
                res.status = 404;
            } else {
                res.status = 200;
            }
            res.set_content(response_body, "application/json");

        } catch (const std::invalid_argument& e) {
            res.set_content("{\"error\":\"Invalid data type for one or more parameters.\"}", "application/json");
            res.status = 400;
        }
    });

    int port = 8081;
    std::cout << "Server starting on port " << port << std::endl;
    std::cout << "API endpoint available at: http://localhost:8081/api/temperature_for_location" << std::endl;

    std::cout << "\n--- How to Test ---" << std::endl;
    std::cout << "Use a tool like curl to send requests to the running server." << std::endl;
    std::cout << "1. Valid request with data:" << std::endl;
    std::cout << "   curl \"http://localhost:8081/api/temperature_for_location?latitude=34.0522&longitude=-118.2437&year=2023&month=10&day=26&grib_file=gfs.t00z.pgrb2.0p25.f000\"" << std::endl;
    std::cout << "\n2. Request for data that doesn't exist:" << std::endl;
    std::cout << "   curl \"http://localhost:8081/api/temperature_for_location?latitude=0&longitude=0&year=1900&month=1&day=1&grib_file=none\"" << std::endl;
    std::cout << "\n3. Request with missing parameters:" << std::endl;
    std::cout << "   curl \"http://localhost:8081/api/temperature_for_location?latitude=34.0522&longitude=-118.2437\"" << std::endl;
    std::cout << "\n4. Request with invalid parameter type:" << std::endl;
    std::cout << "   curl \"http://localhost:8081/api/temperature_for_location?latitude=invalid&longitude=-118.2437&year=2023&month=10&day=26&grib_file=file.grib\"" << std::endl;
    std::cout << "\n5. Another valid request:" << std::endl;
    std::cout << "   curl \"http://localhost:8081/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=10&day=27&grib_file=gfs.t00z.pgrb2.0p25.f001\"" << std::endl;

    svr.listen("0.0.0.0", port);

    return 0;
}