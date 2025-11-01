
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <sqlite3.h>
#include <sys/stat.h>

class Lse68 {
private:
    static const std::string DB_PATH;
    static const size_t MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB
    
    static std::string base64_encode(const std::vector<unsigned char>& data) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::string result;
        int val = 0;
        int valb = -6;
        
        for (unsigned char c : data) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                result.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        
        if (valb > -6) {
            result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        
        while (result.size() % 4) {
            result.push_back('=');
        }
        
        return result;
    }
    
    static bool has_valid_extension(const std::string& filename) {
        std::string lower_filename = filename;
        for (char& c : lower_filename) {
            c = tolower(c);
        }
        
        return lower_filename.ends_with(".jpg") || 
               lower_filename.ends_with(".jpeg") || 
               lower_filename.ends_with(".png") || 
               lower_filename.ends_with(".gif");
    }
    
    static std::string get_filename(const std::string& path) {
        size_t pos = path.find_last_of("/\\\\");\n        if (pos != std::string::npos) {\n            return path.substr(pos + 1);\n        }\n        return path;\n    }\n\npublic:\n    static void init_database() {\n        sqlite3* db;\n        char* err_msg = nullptr;\n        \n        int rc = sqlite3_open(DB_PATH.c_str(), &db);\n        if (rc != SQLITE_OK) {\n            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;\n            return;\n        }\n        \n        const char* sql = "CREATE TABLE IF NOT EXISTS images ("\n                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"\n                         "name TEXT NOT NULL,"\n                         "image_data TEXT NOT NULL);";\n        \n        rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);\n        if (rc != SQLITE_OK) {\n            std::cerr << "SQL error: " << err_msg << std::endl;\n            sqlite3_free(err_msg);\n        }\n        \n        sqlite3_close(db);\n    }\n    \n    static bool upload_image(const std::string& image_path) {\n        // Input validation\n        if (image_path.empty()) {\n            std::cerr << "Invalid image path" << std::endl;\n            return false;\n        }\n        \n        // Check if file exists\n        struct stat buffer;\n        if (stat(image_path.c_str(), &buffer) != 0) {\n            std::cerr << "File does not exist" << std::endl;\n            return false;\n        }\n        \n        // Check file size\n        if (buffer.st_size > MAX_FILE_SIZE) {\n            std::cerr << "File size exceeds 5MB limit" << std::endl;\n            return false;\n        }\n        \n        // Validate file extension\n        if (!has_valid_extension(image_path)) {\n            std::cerr << "Invalid file type. Only JPG, PNG, and GIF allowed" << std::endl;\n            return false;\n        }\n        \n        // Read file\n        std::ifstream file(image_path, std::ios::binary);\n        if (!file) {\n            std::cerr << "Error opening file" << std::endl;\n            return false;\n        }\n        \n        std::vector<unsigned char> image_data((std::istreambuf_iterator<char>(file)),\n                                              std::istreambuf_iterator<char>());\n        file.close();\n        \n        // Convert to base64\n        std::string base64_image = base64_encode(image_data);\n        std::string filename = get_filename(image_path);\n        \n        // Insert into database\n        sqlite3* db;\n        int rc = sqlite3_open(DB_PATH.c_str(), &db);\n        if (rc != SQLITE_OK) {\n            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;\n            return false;\n        }\n        \n        sqlite3_stmt* stmt;\n        const char* sql = "INSERT INTO images (name, image_data) VALUES (?, ?)";\n        \n        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);\n        if (rc != SQLITE_OK) {\n            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;\n            sqlite3_close(db);\n            return false;\n        }\n        \n        sqlite3_bind_text(stmt, 1, filename.c_str(), -1, SQLITE_TRANSIENT);\n        sqlite3_bind_text(stmt, 2, base64_image.c_str(), -1, SQLITE_TRANSIENT);\n        \n        rc = sqlite3_step(stmt);\n        sqlite3_finalize(stmt);\n        sqlite3_close(db);\n        \n        if (rc != SQLITE_DONE) {\n            std::cerr << "Execution failed" << std::endl;\n            return false;\n        }\n        \n        std::cout << "Image uploaded successfully: " << filename << std::endl;\n        return true;\n    }\n};\n\nconst std::string Lse68::DB_PATH = "images.db";\n\nint main() {\n    // Initialize database\n    Lse68::init_database();\n    \n    // Test cases\n    std::cout << "Test Case 1: Valid image (create a test.jpg file first)" << std::endl;\n    Lse68::upload_image("test.jpg");\n    \n    std::cout << "\
Test Case 2: Empty path" << std::endl;\n    Lse68::upload_image("");\n    \n    std::cout << "\
Test Case 3: Non-existent file" << std::endl;\n    Lse68::upload_image("nonexistent.jpg");\n    \n    std::cout << "\
Test Case 4: Invalid file type" << std::endl;\n    Lse68::upload_image("test.txt");\n    \n    std::cout << "\
Test Case 5: Another valid test" << std::endl;\n    Lse68::upload_image("test2.png");
    
    return 0;
}
