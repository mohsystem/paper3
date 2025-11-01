
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <cstring>

class Lse68 {
private:
    static std::string base64_encode(const std::vector<unsigned char>& data) {
        const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
        
        std::string encoded;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        
        for (size_t n = 0; n < data.size(); n++) {
            char_array_3[i++] = data[n];
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for(i = 0; i < 4; i++)
                    encoded += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (j = 0; j < i + 1; j++)
                encoded += base64_chars[char_array_4[j]];
            
            while(i++ < 3)
                encoded += '=';
        }
        
        return encoded;
    }
    
    static std::string getFileName(const std::string& path) {
        size_t pos = path.find_last_of("/\\\\");\n        if (pos != std::string::npos)\n            return path.substr(pos + 1);\n        return path;\n    }\n\npublic:\n    static void initDatabase() {\n        sqlite3* db;\n        char* errMsg = 0;\n        \n        int rc = sqlite3_open("images.db", &db);\n        if (rc) {\n            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;\n            return;\n        }\n        \n        const char* sql = "CREATE TABLE IF NOT EXISTS images ("\n                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"\n                         "name TEXT NOT NULL,"\n                         "image_data TEXT NOT NULL);";\n        \n        rc = sqlite3_exec(db, sql, 0, 0, &errMsg);\n        if (rc != SQLITE_OK) {\n            std::cerr << "SQL error: " << errMsg << std::endl;\n            sqlite3_free(errMsg);\n        }\n        \n        sqlite3_close(db);\n    }\n    \n    static bool upload_image(const std::string& imagePath) {\n        std::ifstream file(imagePath, std::ios::binary);\n        if (!file.is_open()) {\n            return false;\n        }\n        \n        std::vector<unsigned char> imageData((std::istreambuf_iterator<char>(file)),\n                                              std::istreambuf_iterator<char>());\n        file.close();\n        \n        std::string base64Image = base64_encode(imageData);\n        std::string imageName = getFileName(imagePath);\n        \n        sqlite3* db;\n        int rc = sqlite3_open("images.db", &db);\n        if (rc) {\n            return false;\n        }\n        \n        const char* sql = "INSERT INTO images (name, image_data) VALUES (?, ?);";\n        sqlite3_stmt* stmt;\n        \n        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);\n        if (rc != SQLITE_OK) {\n            sqlite3_close(db);\n            return false;\n        }\n        \n        sqlite3_bind_text(stmt, 1, imageName.c_str(), -1, SQLITE_TRANSIENT);\n        sqlite3_bind_text(stmt, 2, base64Image.c_str(), -1, SQLITE_TRANSIENT);\n        \n        rc = sqlite3_step(stmt);\n        sqlite3_finalize(stmt);\n        sqlite3_close(db);\n        \n        return rc == SQLITE_DONE;\n    }\n};\n\nint main() {\n    Lse68::initDatabase();\n    \n    // Test case 1: Upload a valid image\n    std::cout << "Test 1: " << Lse68::upload_image("test_image1.jpg") << std::endl;\n    \n    // Test case 2: Upload another valid image\n    std::cout << "Test 2: " << Lse68::upload_image("test_image2.png") << std::endl;\n    \n    // Test case 3: Upload non-existent image\n    std::cout << "Test 3: " << Lse68::upload_image("non_existent.jpg") << std::endl;\n    \n    // Test case 4: Upload with different path\n    std::cout << "Test 4: " << Lse68::upload_image("images/photo.jpg") << std::endl;\n    \n    // Test case 5: Upload another image\n    std::cout << "Test 5: " << Lse68::upload_image("sample.bmp") << std::endl;
    
    return 0;
}
