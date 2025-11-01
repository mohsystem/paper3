
#include <iostream>
#include <fstream>
#include <mysql/mysql.h>
#include <string>
#include <cstring>

bool storeImageInDatabase(const std::string& imagePath) {
    MYSQL* conn;
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[2];
    
    const char* host = "localhost";
    const char* user = "root";
    const char* password = "password";
    const char* database = "imagedb";
    
    // Initialize MySQL connection
    conn = mysql_init(NULL);
    
    if (!mysql_real_connect(conn, host, user, password, database, 0, NULL, 0)) {
        std::cerr << "Connection failed: " << mysql_error(conn) << std::endl;
        return false;
    }
    
    // Read image file
    std::ifstream file(imagePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Cannot open file" << std::endl;
        mysql_close(conn);
        return false;
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    char* buffer = new char[size];
    if (!file.read(buffer, size)) {
        std::cerr << "Cannot read file" << std::endl;
        delete[] buffer;
        mysql_close(conn);
        return false;
    }
    
    // Get filename
    size_t pos = imagePath.find_last_of("/\\\\");\n    std::string filename = (pos != std::string::npos) ? imagePath.substr(pos + 1) : imagePath;\n    \n    // Prepare statement\n    const char* query = "INSERT INTO images (image_name, image_data) VALUES (?, ?)";\n    stmt = mysql_stmt_init(conn);\n    \n    if (mysql_stmt_prepare(stmt, query, strlen(query))) {\n        std::cerr << "Prepare failed: " << mysql_stmt_error(stmt) << std::endl;\n        delete[] buffer;\n        mysql_close(conn);\n        return false;\n    }\n    \n    // Bind parameters\n    memset(bind, 0, sizeof(bind));\n    \n    bind[0].buffer_type = MYSQL_TYPE_STRING;\n    bind[0].buffer = (char*)filename.c_str();\n    bind[0].buffer_length = filename.length();\n    \n    bind[1].buffer_type = MYSQL_TYPE_LONG_BLOB;\n    bind[1].buffer = buffer;\n    bind[1].buffer_length = size;\n    \n    mysql_stmt_bind_param(stmt, bind);\n    \n    // Execute\n    bool result = (mysql_stmt_execute(stmt) == 0);\n    \n    // Cleanup\n    delete[] buffer;\n    mysql_stmt_close(stmt);\n    mysql_close(conn);\n    \n    return result;\n}\n\nint main() {\n    // Test cases - Note: These require actual image files and MySQL setup\n    std::cout << "Test Case 1: " << storeImageInDatabase("test1.jpg") << std::endl;\n    std::cout << "Test Case 2: " << storeImageInDatabase("test2.png") << std::endl;\n    std::cout << "Test Case 3: " << storeImageInDatabase("test3.gif") << std::endl;\n    std::cout << "Test Case 4: " << storeImageInDatabase("test4.bmp") << std::endl;\n    std::cout << "Test Case 5: " << storeImageInDatabase("test5.jpeg") << std::endl;
    
    return 0;
}
