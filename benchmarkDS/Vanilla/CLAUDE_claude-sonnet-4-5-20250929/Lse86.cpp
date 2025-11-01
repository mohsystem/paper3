
#include <iostream>
#include <string>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <mysql/mysql.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class Lse86 {
private:
    http_listener listener;
    const char* db_host = "localhost";
    const char* db_user = "root";
    const char* db_password = "password";
    const char* db_name = "weather_db";

    double getMaxTemperature(const std::string& latitude, const std::string& longitude,
                            const std::string& year, const std::string& month,
                            const std::string& day, const std::string& grib_file) {
        MYSQL* conn = mysql_init(NULL);
        double max_temp = -999.0;
        
        if (conn == NULL) {
            return max_temp;
        }
        
        if (mysql_real_connect(conn, db_host, db_user, db_password, db_name, 0, NULL, 0) == NULL) {
            mysql_close(conn);
            return max_temp;
        }
        
        std::string query = "SELECT MAX(temperature) as max_temp FROM temperature_data "
                           "WHERE latitude = '" + latitude + "' AND longitude = '" + longitude + 
                           "' AND year = '" + year + "' AND month = '" + month + 
                           "' AND day = '" + day + "' AND grib_file = '" + grib_file + "'";
        
        if (mysql_query(conn, query.c_str()) == 0) {
            MYSQL_RES* result = mysql_store_result(conn);
            if (result != NULL) {
                MYSQL_ROW row = mysql_fetch_row(result);
                if (row != NULL && row[0] != NULL) {
                    max_temp = std::stod(row[0]);
                }
                mysql_free_result(result);
            }
        }
        
        mysql_close(conn);
        return max_temp;
    }

    void handleGet(http_request request) {
        auto query = uri::split_query(request.request_uri().query());
        
        std::string latitude = query["latitude"];
        std::string longitude = query["longitude"];
        std::string year = query["year"];
        std::string month = query["month"];
        std::string day = query["day"];
        std::string grib_file = query["grib_file"];
        
        double max_temp = getMaxTemperature(latitude, longitude, year, month, day, grib_file);
        
        json::value response;
        if (max_temp != -999.0) {
            response["max_temperature"] = json::value::number(max_temp);
        } else {
            response["error"] = json::value::string("No data found");
        }
        
        request.reply(status_codes::OK, response);
    }

public:
    Lse86(const std::string& url) : listener(url) {
        listener.support(methods::GET, std::bind(&Lse86::handleGet, this, std::placeholders::_1));
    }
    
    void start() {
        listener.open().wait();
        std::cout << "Listening on /api/temperature_for_location" << std::endl;
    }
    
    void stop() {
        listener.close().wait();
    }
};

int main() {
    Lse86 server("http://localhost:8080/api/temperature_for_location");
    server.start();
    
    std::cout << "Test cases (use curl or browser):" << std::endl;
    std::cout << "1. http://localhost:8080/api/temperature_for_location?latitude=40.7&longitude=-74.0&year=2023&month=6&day=15&grib_file=data.grib" << std::endl;
    std::cout << "2. http://localhost:8080/api/temperature_for_location?latitude=51.5&longitude=-0.1&year=2023&month=7&day=20&grib_file=data.grib" << std::endl;
    std::cout << "3. http://localhost:8080/api/temperature_for_location?latitude=35.6&longitude=139.6&year=2023&month=8&day=10&grib_file=data.grib" << std::endl;
    std::cout << "4. http://localhost:8080/api/temperature_for_location?latitude=48.8&longitude=2.3&year=2023&month=9&day=25&grib_file=data.grib" << std::endl;
    std::cout << "5. http://localhost:8080/api/temperature_for_location?latitude=-33.8&longitude=151.2&year=2023&month=10&day=5&grib_file=data.grib" << std::endl;
    
    std::cout << "Press Enter to stop server..." << std::endl;
    std::cin.get();
    
    server.stop();
    return 0;
}
