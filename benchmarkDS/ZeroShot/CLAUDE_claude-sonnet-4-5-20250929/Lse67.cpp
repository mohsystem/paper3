
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <mysql/mysql.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

class Lse67 {
private:
    static const long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB
    static const std::vector<std::string> ALLOWED_EXTENSIONS;
    
    static bool isValidExtension(const std::string& extension) {
        return std::find(ALLOWED_EXTENSIONS.begin(), ALLOWED_EXTENSIONS.end(), extension) != ALLOWED_EXTENSIONS.end();
    }
    
    static std::string getFileExtension(const std::string& filename) {
        size_t pos = filename.find_last_of('.');
        if (pos == std::string::npos) return "";
        std::string ext = filename.substr(pos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }
    
    static bool isValidImageFile(const std::vector<unsigned char>& data) {
        if (data.size() < 4) return false;
        
        // JPEG: FF D8 FF
        if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) {
            return true;
        }
        // PNG: 89 50 4E 47
        if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47) {
            return true;
        }
        // GIF: 47 49 46 38
        if (data[0] == 0x47 && data[1] == 0x49 && data[2] == 0x46 && data[3] == 0x38) {
            return true;
        }
        return false;
    }
    
    static std::string calculateSHA256(const std::vector<unsigned char>& data) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(data.data(), data.size(), hash);
        
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    static void createTableIfNotExists(MYSQL* conn) {
        const char* sql = "CREATE TABLE IF NOT EXISTS images ("
                         "id INT AUTO_INCREMENT PRIMARY KEY, "
                         "file_name VARCHAR(255) NOT NULL, "
                         "file_size BIGINT NOT NULL, "
                         "file_hash VARCHAR(64) NOT NULL, "
                         "image_data LONGBLOB NOT NULL, "
                         "upload_date DATETIME NOT NULL)";
        
        if (mysql_query(conn, sql)) {
            std::cerr << "Error creating table: " << mysql_error(conn) << std::endl;
        }
    }
    
public:
    static bool storeImage(const std::string& imagePath, const std::string& dbHost, 
                          const std::string& dbUser, const std::string& dbPassword, 
                          const std::string& dbName) {
        MYSQL* conn = nullptr;
        MYSQL_STMT* stmt = nullptr;
        
        try {
            // Check if file exists
            std::ifstream file(imagePath, std::ios::binary | std::ios::ate);
            if (!file.is_open()) {
                std::cerr << "Error: File does not exist or cannot be opened" << std::endl;
                return false;
            }
            
            // Check file size
            std::streamsize fileSize = file.tellg();
            if (fileSize > MAX_FILE_SIZE) {
                std::cerr << "Error: File size exceeds maximum allowed size" << std::endl;
                file.close();
                return false;
            }
            
            // Validate file extension
            std::string extension = getFileExtension(imagePath);
            if (!isValidExtension(extension)) {
                std::cerr << "Error: Invalid file extension" << std::endl;
                file.close();
                return false;
            }
            
            // Read file content
            file.seekg(0, std::ios::beg);
            std::vector<unsigned char> imageData(fileSize);
            file.read(reinterpret_cast<char*>(imageData.data()), fileSize);
            file.close();
            
            // Verify it's actually an image
            if (!isValidImageFile(imageData)) {
                std::cerr << "Error: File is not a valid image" << std::endl;
                return false;
            }
            
            // Calculate file hash
            std::string fileHash = calculateSHA256(imageData);
            
            // Extract filename
            size_t pos = imagePath.find_last_of("/\\\\");\n            std::string fileName = (pos == std::string::npos) ? imagePath : imagePath.substr(pos + 1);\n            \n            // Connect to database\n            conn = mysql_init(nullptr);\n            if (!conn) {\n                std::cerr << "Error: mysql_init failed" << std::endl;\n                return false;\n            }\n            \n            if (!mysql_real_connect(conn, dbHost.c_str(), dbUser.c_str(), \n                                   dbPassword.c_str(), dbName.c_str(), 0, nullptr, 0)) {\n                std::cerr << "Error: " << mysql_error(conn) << std::endl;\n                mysql_close(conn);\n                return false;\n            }\n            \n            // Create table if not exists\n            createTableIfNotExists(conn);\n            \n            // Prepare statement\n            const char* sql = "INSERT INTO images (file_name, file_size, file_hash, image_data, upload_date) "\n                            "VALUES (?, ?, ?, ?, NOW())";\n            stmt = mysql_stmt_init(conn);\n            if (!stmt) {\n                std::cerr << "Error: mysql_stmt_init failed" << std::endl;\n                mysql_close(conn);\n                return false;\n            }\n            \n            if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {\n                std::cerr << "Error: " << mysql_stmt_error(stmt) << std::endl;\n                mysql_stmt_close(stmt);\n                mysql_close(conn);\n                return false;\n            }\n            \n            // Bind parameters\n            MYSQL_BIND bind[4];\n            memset(bind, 0, sizeof(bind));\n            \n            // file_name\n            bind[0].buffer_type = MYSQL_TYPE_STRING;\n            bind[0].buffer = const_cast<char*>(fileName.c_str());\n            bind[0].buffer_length = fileName.length();\n            \n            // file_size\n            bind[1].buffer_type = MYSQL_TYPE_LONGLONG;\n            bind[1].buffer = &fileSize;\n            \n            // file_hash\n            bind[2].buffer_type = MYSQL_TYPE_STRING;\n            bind[2].buffer = const_cast<char*>(fileHash.c_str());\n            bind[2].buffer_length = fileHash.length();\n            \n            // image_data\n            unsigned long dataLength = imageData.size();\n            bind[3].buffer_type = MYSQL_TYPE_LONG_BLOB;\n            bind[3].buffer = imageData.data();\n            bind[3].buffer_length = dataLength;\n            bind[3].length = &dataLength;\n            \n            if (mysql_stmt_bind_param(stmt, bind)) {\n                std::cerr << "Error: " << mysql_stmt_error(stmt) << std::endl;\n                mysql_stmt_close(stmt);\n                mysql_close(conn);\n                return false;\n            }\n            \n            // Execute statement\n            if (mysql_stmt_execute(stmt)) {\n                std::cerr << "Error: " << mysql_stmt_error(stmt) << std::endl;\n                mysql_stmt_close(stmt);\n                mysql_close(conn);\n                return false;\n            }\n            \n            mysql_stmt_close(stmt);\n            mysql_close(conn);\n            return true;\n            \n        } catch (const std::exception& e) {\n            std::cerr << "Error: " << e.what() << std::endl;\n            if (stmt) mysql_stmt_close(stmt);\n            if (conn) mysql_close(conn);\n            return false;\n        }\n    }\n};\n\nconst std::vector<std::string> Lse67::ALLOWED_EXTENSIONS = {"jpg", "jpeg", "png", "gif"};\n\nint main() {\n    // Database configuration - CHANGE THESE VALUES\n    std::string dbHost = "localhost";\n    std::string dbUser = "root";\n    std::string dbPassword = "password";\n    std::string dbName = "imagedb";\n    \n    // Test cases\n    std::cout << "Test Case 1: Valid JPEG image" << std::endl;\n    bool result1 = Lse67::storeImage("test1.jpg", dbHost, dbUser, dbPassword, dbName);\n    std::cout << "Result: " << (result1 ? "true" : "false") << "\
" << std::endl;\n    \n    std::cout << "Test Case 2: Valid PNG image" << std::endl;\n    bool result2 = Lse67::storeImage("test2.png", dbHost, dbUser, dbPassword, dbName);\n    std::cout << "Result: " << (result2 ? "true" : "false") << "\
" << std::endl;\n    \n    std::cout << "Test Case 3: Non-existent file" << std::endl;\n    bool result3 = Lse67::storeImage("nonexistent.jpg", dbHost, dbUser, dbPassword, dbName);\n    std::cout << "Result: " << (result3 ? "true" : "false") << "\
" << std::endl;\n    \n    std::cout << "Test Case 4: Invalid extension" << std::endl;\n    bool result4 = Lse67::storeImage("test.txt", dbHost, dbUser, dbPassword, dbName);\n    std::cout << "Result: " << (result4 ? "true" : "false") << "\
" << std::endl;\n    \n    std::cout << "Test Case 5: Valid GIF image" << std::endl;\n    bool result5 = Lse67::storeImage("test5.gif", dbHost, dbUser, dbPassword, dbName);\n    std::cout << "Result: " << (result5 ? "true" : "false") << std::endl;
    
    return 0;
}
