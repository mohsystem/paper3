
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <mysql/mysql.h>
#include <cstring>
#include <sys/stat.h>

const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
const std::vector<std::string> ALLOWED_EXTENSIONS = {".jpg", ".jpeg", ".png", ".gif", ".bmp"};

class Lse67 {
public:
    static bool storeImageInDatabase(const std::string& imagePath, 
                                     const std::string& dbHost,
                                     const std::string& dbUser, 
                                     const std::string& dbPassword,
                                     const std::string& dbName) {
        // Validate input
        if (imagePath.empty()) {
            std::cout << "Invalid image path" << std::endl;
            return false;
        }
        
        // Check file existence and size
        struct stat fileStat;
        if (stat(imagePath.c_str(), &fileStat) != 0) {
            std::cout << "File does not exist or cannot be accessed" << std::endl;
            return false;
        }
        
        // Validate file size
        if (fileStat.st_size > MAX_FILE_SIZE) {
            std::cout << "File size exceeds maximum allowed size" << std::endl;
            return false;
        }
        
        // Validate file extension
        size_t dotPos = imagePath.find_last_of('.');
        if (dotPos == std::string::npos) {
            std::cout << "No file extension found" << std::endl;
            return false;
        }
        
        std::string extension = imagePath.substr(dotPos);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
        
        bool validExtension = false;
        for (const auto& ext : ALLOWED_EXTENSIONS) {
            if (extension == ext) {
                validExtension = true;
                break;
            }
        }
        
        if (!validExtension) {
            std::cout << "Invalid file extension" << std::endl;
            return false;
        }
        
        // Read file
        std::ifstream file(imagePath, std::ios::binary);
        if (!file) {
            std::cout << "Cannot open file" << std::endl;
            return false;
        }
        
        std::vector<char> imageData((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
        file.close();
        
        // Initialize MySQL
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) {
            std::cout << "MySQL initialization failed" << std::endl;
            return false;
        }
        
        // Connect to database
        if (!mysql_real_connect(conn, dbHost.c_str(), dbUser.c_str(), 
                               dbPassword.c_str(), dbName.c_str(), 0, nullptr, 0)) {
            std::cout << "Database connection failed: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return false;
        }
        
        // Prepare statement
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            std::cout << "Statement initialization failed" << std::endl;
            mysql_close(conn);
            return false;
        }
        
        const char* query = "INSERT INTO images (filename, image_data, upload_date) VALUES (?, ?, NOW())";
        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            std::cout << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        // Bind parameters
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));
        
        // Filename parameter
        std::string filename = imagePath.substr(imagePath.find_last_of("/\\\\") + 1);\n        bind[0].buffer_type = MYSQL_TYPE_STRING;\n        bind[0].buffer = (char*)filename.c_str();\n        bind[0].buffer_length = filename.length();\n        \n        // Image data parameter\n        bind[1].buffer_type = MYSQL_TYPE_BLOB;\n        bind[1].buffer = imageData.data();\n        bind[1].buffer_length = imageData.size();\n        \n        if (mysql_stmt_bind_param(stmt, bind)) {\n            std::cout << "Parameter binding failed: " << mysql_stmt_error(stmt) << std::endl;\n            mysql_stmt_close(stmt);\n            mysql_close(conn);\n            return false;\n        }\n        \n        // Execute statement\n        bool success = false;\n        if (mysql_stmt_execute(stmt) == 0) {\n            success = true;\n        } else {\n            std::cout << "Execution failed: " << mysql_stmt_error(stmt) << std::endl;\n        }\n        \n        // Clean up\n        mysql_stmt_close(stmt);\n        mysql_close(conn);\n        \n        return success;\n    }\n};\n\nint main() {\n    std::string dbHost = "localhost";\n    std::string dbUser = "root";\n    std::string dbPassword = "password";\n    std::string dbName = "imagedb";\n    \n    // Test cases\n    std::cout << "Test 1 (valid image): " \n              << Lse67::storeImageInDatabase("test1.jpg", dbHost, dbUser, dbPassword, dbName) \n              << std::endl;\n    \n    std::cout << "Test 2 (valid image): " \n              << Lse67::storeImageInDatabase("test2.png", dbHost, dbUser, dbPassword, dbName) \n              << std::endl;\n    \n    std::cout << "Test 3 (empty path): " \n              << Lse67::storeImageInDatabase("", dbHost, dbUser, dbPassword, dbName) \n              << std::endl;\n    \n    std::cout << "Test 4 (non-existent file): " \n              << Lse67::storeImageInDatabase("nonexistent.jpg", dbHost, dbUser, dbPassword, dbName) \n              << std::endl;\n    \n    std::cout << "Test 5 (invalid extension): " \n              << Lse67::storeImageInDatabase("test.txt", dbHost, dbUser, dbPassword, dbName) 
              << std::endl;
    
    return 0;
}
