/*
NOTE:
1. This code requires external libraries that are NOT part of the C++ standard.
   - cpp-httplib: A header-only HTTP server library. Download `httplib.h` from its GitHub repository.
   - MySQL Connector/C++: The official MySQL C++ driver.

2. Installation (example on Debian/Ubuntu):
   sudo apt-get install libmysqlcppconn-dev
   wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h

3. Compilation:
   Place `httplib.h` in the same directory or in your include path.
   g++ -o Lse86_server Lse86.cpp -lmysqlcppconn -pthread -std=c++17

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
#include "httplib.h"
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>

// MySQL Connector/C++ includes
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

// --- DATABASE CONFIGURATION ---
const std::string DB_HOST = "tcp://127.0.0.1:3306";
const std::string DB_USER = "weather_user";
const std::string DB_PASS = "password";
const std::string DB_NAME = "weatherdb";

// Function to handle the database query
std::string getMaxTemperature(double lat, double lon, int year, int month, int day, const std::string& gribFile) {
    try {
        sql::Driver* driver = get_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect(DB_HOST, DB_USER, DB_PASS));
        con->setSchema(DB_NAME);

        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
            "SELECT MAX(temperature) AS max_temp FROM weather_data "
            "WHERE latitude = ? AND longitude = ? AND year = ? AND month = ? AND day = ? AND grib_file = ?"
        ));

        pstmt->setDouble(1, lat);
        pstmt->setDouble(2, lon);
        pstmt->setInt(3, year);
        pstmt->setInt(4, month);
        pstmt->setInt(5, day);
        pstmt->setString(6, gribFile);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {
            if (res->isNull("max_temp")) {
                 return "{\"message\": \"Temperature data not found for the given criteria.\"}";
            }
            double maxTemp = res->getDouble("max_temp");
            // Format to 2 decimal places
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "{\"max_temperature\": %.2f}", maxTemp);
            return std::string(buffer);
        } else {
            return "{\"message\": \"Temperature data not found for the given criteria.\"}";
        }
    } catch (sql::SQLException &e) {
        std::cerr << "SQL Error: " << e.what() << " (MySQL error code: " << e.getErrorCode() << ")" << std::endl;
        return "{\"error\": \"Database query failed.\"}";
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return "{\"error\": \"An internal error occurred.\"}";
    }
}


int main(void) {
    httplib::Server svr;

    svr.Get("/api/temperature_for_location", [](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("latitude") && req.has_param("longitude") &&
            req.has_param("year") && req.has_param("month") &&
            req.has_param("day") && req.has_param("grib_file")) {
            
            try {
                double lat = std::stod(req.get_param_value("latitude"));
                double lon = std::stod(req.get_param_value("longitude"));
                int year = std::stoi(req.get_param_value("year"));
                int month = std::stoi(req.get_param_value("month"));
                int day = std::stoi(req.get_param_value("day"));
                std::string grib_file = req.get_param_value("grib_file");
                
                std::string json_response = getMaxTemperature(lat, lon, year, month, day, grib_file);

                if (json_response.find("not found") != std::string::npos) {
                    res.status = 404;
                } else if (json_response.find("error") != std::string::npos) {
                    res.status = 500;
                } else {
                    res.status = 200;
                }
                res.set_content(json_response, "application/json");

            } catch (const std::invalid_argument& e) {
                res.status = 400;
                res.set_content("{\"error\": \"Bad Request: Invalid data type for a parameter.\"}", "application/json");
            } catch (...) {
                res.status = 500;
                res.set_content("{\"error\": \"Internal Server Error\"}", "application/json");
            }
        } else {
            res.status = 400;
            res.set_content("{\"error\": \"Bad Request: Missing required parameters.\"}", "application/json");
        }
    });

    int port = 8080;
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "--- 5 Test Cases (run in a separate terminal) ---" << std::endl;
    std::cout << "1. curl \"http://localhost:" << port << "/api/temperature_for_location?latitude=34.05&longitude=-118.24&year=2023&month=7&day=15&grib_file=gfs.t00z.pgrb2.0p25.f000\"" << std::endl;
    std::cout << "2. curl \"http://localhost:" << port << "/api/temperature_for_location?latitude=40.71&longitude=-74.00&year=2023&month=8&day=20&grib_file=gfs.t06z.pgrb2.0p25.f001\"" << std::endl;
    std::cout << "3. curl \"http://localhost:" << port << "/api/temperature_for_location?latitude=48.85&longitude=2.35&year=2024&month=1&day=10&grib_file=era5.12z.an.sfc\"" << std::endl;
    std::cout << "4. curl \"http://localhost:" << port << "/api/temperature_for_location?latitude=99.99&longitude=99.99&year=2025&month=1&day=1&grib_file=none\" # Should return not found" << std::endl;
    std::cout << "5. curl \"http://localhost:" << port << "/api/temperature_for_location?latitude=40.71\" # Should return bad request" << std::endl;
    
    svr.listen("0.0.0.0", port);

    return 0;
}