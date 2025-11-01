
#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class Lse86 {
private:
    std::string db_host = "localhost";
    std::string db_user = "your_username";
    std::string db_password = "your_password";
    std::string db_name = "weatherdb";
    
    double getMaxTemperature(const std::string& latitude, const std::string& longitude,
                            const std::string& year, const std::string& month,
                            const std::string& day, const std::string& grib_file) {
        MYSQL* conn = mysql_init(nullptr);
        
        if (!mysql_real_connect(conn, db_host.c_str(), db_user.c_str(), 
                               db_password.c_str(), db_name.c_str(), 0, nullptr, 0)) {
            std::cerr << "Database connection failed: " << mysql_error(conn) << std::endl;
            return -999.0;
        }
        
        std::string query = "SELECT MAX(temperature) as max_temp FROM temperature_data "
                           "WHERE latitude = ? AND longitude = ? AND year = ? "
                           "AND month = ? AND day = ? AND grib_file = ?";
        
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
            std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return -999.0;
        }
        
        MYSQL_BIND bind[6];
        memset(bind, 0, sizeof(bind));
        
        double lat = std::stod(latitude);
        double lon = std::stod(longitude);
        int yr = std::stoi(year);
        int mn = std::stoi(month);
        int dy = std::stoi(day);
        
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
        bind[5].buffer = (char*)grib_file.c_str();
        bind[5].buffer_length = grib_file.length();
        
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
    
public:
    void handleRequest(http_request request) {
        auto uri = request.request_uri();
        auto query_params = uri::split_query(uri.query());
        
        if (query_params.find(U("latitude")) == query_params.end() ||
            query_params.find(U("longitude")) == query_params.end() ||
            query_params.find(U("year")) == query_params.end() ||
            query_params.find(U("month")) == query_params.end() ||
            query_params.find(U("day")) == query_params.end() ||
            query_params.find(U("grib_file")) == query_params.end()) {
            
            json::value response;
            response[U("error")] = json::value::string(U("Missing required parameters"));
            request.reply(status_codes::BadRequest, response);
            return;
        }
        
        std::string latitude = utility::conversions::to_utf8string(query_params[U("latitude")]);
        std::string longitude = utility::conversions::to_utf8string(query_params[U("longitude")]);
        std::string year = utility::conversions::to_utf8string(query_params[U("year")]);
        std::string month = utility::conversions::to_utf8string(query_params[U("month")]);
        std::string day = utility::conversions::to_utf8string(query_params[U("day")]);
        std::string grib_file = utility::conversions::to_utf8string(query_params[U("grib_file")]);
        
        double max_temp = getMaxTemperature(latitude, longitude, year, month, day, grib_file);
        
        json::value response;
        if (max_temp != -999.0) {
            response[U("max_temperature")] = json::value::number(max_temp);
            request.reply(status_codes::OK, response);
        } else {
            response[U("error")] = json::value::string(U("No data found"));
            request.reply(status_codes::NotFound, response);
        }
    }
};

int main() {
    http_listener listener(U("http://localhost:8080/api/temperature_for_location"));
    Lse86 handler;
    
    listener.support(methods::GET, [&handler](http_request request) {
        handler.handleRequest(request);
    });
    
    try {
        listener.open().wait();
        std::cout << "Server listening on http://localhost:8080/api/temperature_for_location" << std::endl;
        std::cout << "\\nTest cases:" << std::endl;
        std::cout << "1. GET http://localhost:8080/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=1&day=15&grib_file=test.grib" << std::endl;
        std::cout << "2. GET http://localhost:8080/api/temperature_for_location?latitude=51.5074&longitude=-0.1278&year=2023&month=6&day=20&grib_file=test2.grib" << std::endl;
        std::cout << "3. GET http://localhost:8080/api/temperature_for_location?latitude=35.6762&longitude=139.6503&year=2023&month=12&day=25&grib_file=test3.grib" << std::endl;
        std::cout << "4. GET http://localhost:8080/api/temperature_for_location?latitude=48.8566&longitude=2.3522&year=2023&month=3&day=10&grib_file=test4.grib" << std::endl;
        std::cout << "5. GET http://localhost:8080/api/temperature_for_location?latitude=-33.8688&longitude=151.2093&year=2023&month=9&day=5&grib_file=test5.grib" << std::endl;
        
        std::string line;
        std::getline(std::cin, line);
        
        listener.close().wait();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
