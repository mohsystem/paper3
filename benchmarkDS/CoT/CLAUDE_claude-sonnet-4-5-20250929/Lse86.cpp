
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <mysql/mysql.h>
#include <microhttpd.h>
#include <cstring>
#include <regex>
#include <cstdlib>

class Lse86 {
private:
    static const char* DB_HOST;
    static const char* DB_USER;
    static const char* DB_PASSWORD;
    static const char* DB_NAME;
    
    static bool validateDouble(const char* value, double min, double max, double& result) {
        if (!value) return false;
        try {
            result = std::stod(value);
            return result >= min && result <= max;
        } catch (...) {
            return false;
        }
    }
    
    static bool validateInt(const char* value, int min, int max, int& result) {
        if (!value) return false;
        try {
            result = std::stoi(value);
            return result >= min && result <= max;
        } catch (...) {
            return false;
        }
    }
    
    static std::string validateString(const char* value, size_t maxLength) {
        if (!value) return "";
        std::string str(value);
        if (str.length() > maxLength) return "";
        std::regex pattern("[^a-zA-Z0-9._-]");
        return std::regex_replace(str, pattern, "");
    }

public:
    static double getMaxTemperature(double latitude, double longitude, int year, 
                                   int month, int day, const std::string& gribFile) {
        MYSQL* conn = mysql_init(NULL);
        
        if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, 0, NULL, 0)) {
            mysql_close(conn);
            return -999.0;
        }
        
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        const char* query = "SELECT MAX(temperature) as max_temp FROM temperature_data "
                          "WHERE latitude = ? AND longitude = ? AND year = ? "
                          "AND month = ? AND day = ? AND grib_file = ?";
        
        mysql_stmt_prepare(stmt, query, strlen(query));
        
        MYSQL_BIND bind[6];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[0].buffer = (char*)&latitude;
        
        bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[1].buffer = (char*)&longitude;
        
        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = (char*)&year;
        
        bind[3].buffer_type = MYSQL_TYPE_LONG;
        bind[3].buffer = (char*)&month;
        
        bind[4].buffer_type = MYSQL_TYPE_LONG;
        bind[4].buffer = (char*)&day;
        
        bind[5].buffer_type = MYSQL_TYPE_STRING;
        bind[5].buffer = (char*)gribFile.c_str();
        bind[5].buffer_length = gribFile.length();
        
        mysql_stmt_bind_param(stmt, bind);
        mysql_stmt_execute(stmt);
        
        MYSQL_BIND result_bind;
        double maxTemp;
        my_bool is_null;
        
        memset(&result_bind, 0, sizeof(result_bind));
        result_bind.buffer_type = MYSQL_TYPE_DOUBLE;
        result_bind.buffer = (char*)&maxTemp;
        result_bind.is_null = &is_null;
        
        mysql_stmt_bind_result(stmt, &result_bind);
        
        double resultValue = -999.0;
        if (mysql_stmt_fetch(stmt) == 0 && !is_null) {
            resultValue = maxTemp;
        }
        
        mysql_stmt_close(stmt);
        mysql_close(conn);
        
        return resultValue;
    }
    
    static int handleRequest(void* cls, struct MHD_Connection* connection,
                           const char* url, const char* method,
                           const char* version, const char* upload_data,
                           size_t* upload_data_size, void** con_cls) {
        if (strcmp(method, "GET") != 0) {
            return MHD_NO;
        }
        
        if (strcmp(url, "/api/temperature_for_location") != 0) {
            return MHD_NO;
        }
        
        double latitude, longitude;
        int year, month, day;
        
        const char* lat_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "latitude");
        const char* lon_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "longitude");
        const char* year_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "year");
        const char* month_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "month");
        const char* day_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "day");
        const char* grib_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "grib_file");
        
        if (!validateDouble(lat_str, -90, 90, latitude) ||
            !validateDouble(lon_str, -180, 180, longitude) ||
            !validateInt(year_str, 1900, 2100, year) ||
            !validateInt(month_str, 1, 12, month) ||
            !validateInt(day_str, 1, 31, day)) {
            
            const char* error = "{\\"error\\": \\"Invalid parameters\\"}";
            struct MHD_Response* response = MHD_create_response_from_buffer(
                strlen(error), (void*)error, MHD_RESPMEM_PERSISTENT);
            int ret = MHD_queue_response(connection, 400, response);
            MHD_destroy_response(response);
            return ret;
        }
        
        std::string gribFile = validateString(grib_str, 100);
        if (gribFile.empty()) {
            const char* error = "{\\"error\\": \\"Invalid grib_file\\"}";
            struct MHD_Response* response = MHD_create_response_from_buffer(
                strlen(error), (void*)error, MHD_RESPMEM_PERSISTENT);
            int ret = MHD_queue_response(connection, 400, response);
            MHD_destroy_response(response);
            return ret;
        }
        
        double maxTemp = getMaxTemperature(latitude, longitude, year, month, day, gribFile);
        
        std::string responseStr;
        if (maxTemp != -999.0) {
            responseStr = "{\\"max_temperature\\": " + std::to_string(maxTemp) + "}";
        } else {
            responseStr = "{\\"error\\": \\"No data found\\"}";
        }
        
        struct MHD_Response* response = MHD_create_response_from_buffer(
            responseStr.length(), (void*)responseStr.c_str(), MHD_RESPMEM_MUST_COPY);
        int ret = MHD_queue_response(connection, maxTemp != -999.0 ? 200 : 404, response);
        MHD_destroy_response(response);
        
        return ret;
    }
    
    static void runTestCases() {
        std::cout << "\\n=== Test Cases ===" << std::endl;
        
        double result1 = getMaxTemperature(40.7128, -74.0060, 2023, 6, 15, "test.grb");
        std::cout << "Test 1 - NYC coordinates: " << result1 << std::endl;
        
        double result2 = getMaxTemperature(51.5074, -0.1278, 2023, 7, 20, "test.grb");
        std::cout << "Test 2 - London coordinates: " << result2 << std::endl;
        
        double result3 = getMaxTemperature(35.6762, 139.6503, 2023, 8, 10, "test.grb");
        std::cout << "Test 3 - Tokyo coordinates: " << result3 << std::endl;
        
        double result4 = getMaxTemperature(-33.8688, 151.2093, 2023, 12, 25, "test.grb");
        std::cout << "Test 4 - Sydney coordinates: " << result4 << std::endl;
        
        double result5 = getMaxTemperature(48.8566, 2.3522, 2023, 5, 1, "test.grb");
        std::cout << "Test 5 - Paris coordinates: " << result5 << std::endl;
    }
};

const char* Lse86::DB_HOST = "localhost";
const char* Lse86::DB_USER = "root";
const char* Lse86::DB_PASSWORD = "password";
const char* Lse86::DB_NAME = "weather_db";

int main() {
    Lse86::runTestCases();
    
    struct MHD_Daemon* daemon = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY, 8080, NULL, NULL,
        &Lse86::handleRequest, NULL, MHD_OPTION_END);
    
    if (daemon == NULL) {
        return 1;
    }
    
    std::cout << "Server started on port 8080" << std::endl;
    getchar();
    
    MHD_stop_daemon(daemon);
    return 0;
}
