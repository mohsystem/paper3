
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <regex>
#include <cstring>
#include <cstdlib>
#include <mysql/mysql.h>
#include <microhttpd.h>
#include <limits>

// Database connection parameters - should be loaded from secure config
const char* DB_HOST = "localhost";
const char* DB_USER = "weatherapp";
const char* DB_NAME = "weather";

// Validate and parse double with range checking
bool validate_double(const std::string& value, double min_val, double max_val, 
                    const std::string& field_name, double& result) {
    if (value.empty()) {
        return false;
    }
    
    try {
        size_t pos;
        result = std::stod(value, &pos);
        
        // Check that entire string was parsed
        if (pos != value.length()) {
            return false;
        }
        
        // Range validation
        if (result < min_val || result > max_val) {
            return false;
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

// Validate and parse integer with range checking
bool validate_int(const std::string& value, int min_val, int max_val, 
                 const std::string& field_name, int& result) {
    if (value.empty()) {
        return false;
    }
    
    try {
        size_t pos;
        long long parsed = std::stoll(value, &pos);
        
        // Check that entire string was parsed and value fits in int
        if (pos != value.length() || parsed < std::numeric_limits<int>::min() || 
            parsed > std::numeric_limits<int>::max()) {
            return false;
        }
        
        result = static_cast<int>(parsed);
        
        // Range validation
        if (result < min_val || result > max_val) {
            return false;
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

// Validate grib_file parameter to prevent path traversal
bool validate_grib_file(const std::string& grib_file, std::string& result) {
    if (grib_file.empty()) {
        return false;
    }
    
    // Length check to prevent excessive memory usage
    if (grib_file.length() > 100) {
        return false;
    }
    
    // Sanitize filename - only allow alphanumeric, underscore, dash, and dot
    std::regex valid_pattern("^[a-zA-Z0-9_\\\\-\\\\.]+$");
    if (!std::regex_match(grib_file, valid_pattern)) {
        return false;
    }
    
    // Block path traversal attempts
    if (grib_file.find("..") != std::string::npos || 
        grib_file.find("/") != std::string::npos || 
        grib_file.find("\\\\") != std::string::npos) {\n        return false;\n    }\n    \n    result = grib_file;\n    return true;\n}\n\n// Query database with parameterized query to prevent SQL injection\nbool query_max_temperature(double latitude, double longitude, int year, int month, \n                          int day, const std::string& grib_file, double& max_temp) {\n    MYSQL* conn = nullptr;\n    MYSQL_STMT* stmt = nullptr;\n    MYSQL_BIND bind_params[6];\n    MYSQL_BIND bind_result[1];\n    bool success = false;\n    \n    // Initialize MySQL connection\n    conn = mysql_init(nullptr);\n    if (!conn) {\n        return false;\n    }\n    \n    // Get password from environment variable\n    const char* db_password = std::getenv("DB_PASSWORD");\n    if (!db_password) {\n        db_password = "";\n    }\n    \n    // Connect with SSL enabled for production\n    if (!mysql_real_connect(conn, DB_HOST, DB_USER, db_password, DB_NAME, 0, nullptr, CLIENT_SSL)) {\n        mysql_close(conn);\n        return false;\n    }\n    \n    // Prepare parameterized statement to prevent SQL injection\n    const char* query = "SELECT MAX(temperature) as max_temp FROM temperature_data "\n                       "WHERE latitude = ? AND longitude = ? AND year = ? "\n                       "AND month = ? AND day = ? AND grib_file = ? LIMIT 1";\n    \n    stmt = mysql_stmt_init(conn);\n    if (!stmt) {\n        mysql_close(conn);\n        return false;\n    }\n    \n    if (mysql_stmt_prepare(stmt, query, strlen(query))) {\n        mysql_stmt_close(stmt);\n        mysql_close(conn);\n        return false;\n    }\n    \n    // Initialize bind parameters - ensures type safety\n    memset(bind_params, 0, sizeof(bind_params));\n    \n    // Bind latitude (double)\n    bind_params[0].buffer_type = MYSQL_TYPE_DOUBLE;\n    bind_params[0].buffer = &latitude;\n    \n    // Bind longitude (double)\n    bind_params[1].buffer_type = MYSQL_TYPE_DOUBLE;\n    bind_params[1].buffer = &longitude;\n    \n    // Bind year (int)\n    bind_params[2].buffer_type = MYSQL_TYPE_LONG;\n    bind_params[2].buffer = &year;\n    \n    // Bind month (int)\n    bind_params[3].buffer_type = MYSQL_TYPE_LONG;\n    bind_params[3].buffer = &month;\n    \n    // Bind day (int)\n    bind_params[4].buffer_type = MYSQL_TYPE_LONG;\n    bind_params[4].buffer = &day;\n    \n    // Bind grib_file (string) - length is set per call\n    unsigned long grib_file_length = grib_file.length();\n    bind_params[5].buffer_type = MYSQL_TYPE_STRING;\n    bind_params[5].buffer = const_cast<char*>(grib_file.c_str());\n    bind_params[5].buffer_length = grib_file.length();\n    bind_params[5].length = &grib_file_length;\n    \n    if (mysql_stmt_bind_param(stmt, bind_params)) {\n        mysql_stmt_close(stmt);\n        mysql_close(conn);\n        return false;\n    }\n    \n    if (mysql_stmt_execute(stmt)) {\n        mysql_stmt_close(stmt);\n        mysql_close(conn);\n        return false;\n    }\n    \n    // Bind result\n    memset(bind_result, 0, sizeof(bind_result));\n    double result_temp = 0.0;\n    my_bool is_null = 0;\n    \n    bind_result[0].buffer_type = MYSQL_TYPE_DOUBLE;\n    bind_result[0].buffer = &result_temp;\n    bind_result[0].is_null = &is_null;\n    \n    if (mysql_stmt_bind_result(stmt, bind_result)) {\n        mysql_stmt_close(stmt);\n        mysql_close(conn);\n        return false;\n    }\n    \n    // Fetch result\n    if (mysql_stmt_fetch(stmt) == 0 && !is_null) {\n        max_temp = result_temp;\n        success = true;\n    }\n    \n    // Cleanup - close resources in reverse order\n    mysql_stmt_close(stmt);\n    mysql_close(conn);\n    \n    return success;\n}\n\n// HTTP request handler\nstatic int handle_request(void* cls, struct MHD_Connection* connection,\n                         const char* url, const char* method,\n                         const char* version, const char* upload_data,\n                         size_t* upload_data_size, void** con_cls) {\n    // Only accept GET requests\n    if (strcmp(method, "GET") != 0) {\n        const char* error = "{\\"error\\": \\"Method not allowed\\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(
            strlen(error), (void*)error, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, 405, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    // Only handle specific endpoint
    if (strcmp(url, "/api/temperature_for_location") != 0) {
        const char* error = "{\\"error\\": \\"Endpoint not found\\"}";
        struct MHD_Response* response = MHD_create_response_from_buffer(
            strlen(error), (void*)error, MHD_RESPMEM_PERSISTENT);
        int ret = MHD_queue_response(connection, 404, response);
        MHD_destroy_response(response);
        return ret;
    }
    
    // Extract query parameters - treat all as untrusted
    const char* lat_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "latitude");
    const char* lon_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "longitude");
    const char* year_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "year");
    const char* month_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "month");
    const char* day_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "day");
    const char* grib_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "grib_file");
    
    // Validate all parameters
    double latitude, longitude;
    int year, month, day;
    std::string grib_file;
    
    if (!lat_str || !validate_double(lat_str, -90.0, 90.0, "latitude", latitude) ||
        !lon_str || !validate_double(lon_str, -180.0, 180.0, "longitude", longitude) ||
        !year_str || !validate_int(year_str, 1900, 2100, "year", year) ||
        !month_str || !validate_int(month_str, 1, 12, "month", month) ||
        !day_str || !validate_int(day_str, 1, 31, "day", day) ||
        !grib_str || !validate_grib_file(grib_str, grib_file)) {
        
        // Generic error - don't leak validation details\n        const char* error = "{\\"error\\": \\"Invalid request parameters\\"}";\n        struct MHD_Response* response = MHD_create_response_from_buffer(\n            strlen(error), (void*)error, MHD_RESPMEM_PERSISTENT);\n        int ret = MHD_queue_response(connection, 400, response);\n        MHD_destroy_response(response);\n        return ret;\n    }\n    \n    // Query database\n    double max_temp;\n    if (query_max_temperature(latitude, longitude, year, month, day, grib_file, max_temp)) {\n        // Success - format response\n        char response_buffer[256];\n        snprintf(response_buffer, sizeof(response_buffer), \n                "{\\"max_temperature\\": %.2f}", max_temp);\n        \n        struct MHD_Response* response = MHD_create_response_from_buffer(\n            strlen(response_buffer), (void*)response_buffer, MHD_RESPMEM_MUST_COPY);\n        MHD_add_response_header(response, "Content-Type", "application/json");\n        int ret = MHD_queue_response(connection, 200, response);\n        MHD_destroy_response(response);\n        return ret;\n    } else {\n        const char* error = "{\\"error\\": \\"No data found\\"}";\n        struct MHD_Response* response = MHD_create_response_from_buffer(\n            strlen(error), (void*)error, MHD_RESPMEM_PERSISTENT);\n        int ret = MHD_queue_response(connection, 404, response);\n        MHD_destroy_response(response);\n        return ret;\n    }\n}\n\nint main() {\n    std::cout << "Starting web server on port 8000..." << std::endl;\n    std::cout << "Test with:" << std::endl;\n    std::cout << "curl 'http://localhost:8000/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=6&day=15&grib_file=gfs_20230615.grb'" << std::endl;\n    \n    // Start HTTP server\n    struct MHD_Daemon* daemon = MHD_start_daemon(\n        MHD_USE_SELECT_INTERNALLY, 8000, nullptr, nullptr,\n        &handle_request, nullptr, MHD_OPTION_END);\n    \n    if (!daemon) {\n        std::cerr << "Failed to start server" << std::endl;\n        return 1;\n    }\n    \n    // Keep server running\n    std::cout << "Press Enter to stop server..." << std::endl;
    std::cin.get();
    
    MHD_stop_daemon(daemon);
    return 0;
}
