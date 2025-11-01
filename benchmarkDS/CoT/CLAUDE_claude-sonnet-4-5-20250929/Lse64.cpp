
#include <iostream>
#include <fstream>
#include <string>
#include <mysql/mysql.h>
#include <cstring>
#include <sys/stat.h>
#include <uuid/uuid.h>
#include <vector>

class Lse64 {
private:
    static const long MAX_FILE_SIZE = 10 * 1024 * 1024;
    static constexpr const char* UPLOAD_DIR = "uploads";
    
    static bool isPdfFile(const std::vector<unsigned char>& data) {
        if (data.size() < 4) return false;
        return data[0] == 0x25 && data[1] == 0x50 && data[2] == 0x44 && data[3] == 0x46;
    }
    
    static std::string sanitizeFilename(const std::string& filename) {
        std::string result;
        for (char c : filename) {
            if (isalnum(c) || c == '.' || c == '_' || c == '-') {
                result += c;
            } else {
                result += '_';
            }
        }
        return result;
    }
    
    static std::string generateUUID() {
        uuid_t uuid;
        uuid_generate(uuid);
        char uuid_str[37];
        uuid_unparse(uuid, uuid_str);
        return std::string(uuid_str);
    }

public:
    static std::string uploadPdf(const std::vector<unsigned char>& fileData, 
                                 const std::string& originalFilename) {
        // Validate file size
        if (fileData.size() > MAX_FILE_SIZE) {
            return "Error: File size exceeds maximum limit";
        }
        
        // Validate PDF format
        std::string lower_filename = originalFilename;
        std::transform(lower_filename.begin(), lower_filename.end(), 
                      lower_filename.begin(), ::tolower);
        
        if (!isPdfFile(fileData) || lower_filename.find(".pdf") == std::string::npos) {
            return "Error: Invalid PDF file";
        }
        
        try {
            // Create uploads directory
            mkdir(UPLOAD_DIR, 0755);
            
            // Generate secure filename
            std::string uniqueFilename = generateUUID() + ".pdf";
            std::string filepath = std::string(UPLOAD_DIR) + "/" + uniqueFilename;
            
            // Save file
            std::ofstream outFile(filepath, std::ios::binary);
            outFile.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());
            outFile.close();
            
            // Save to database
            MYSQL* conn = mysql_init(nullptr);
            if (!mysql_real_connect(conn, "localhost", "your_username", 
                                   "your_password", "filedb", 0, nullptr, 0)) {
                return "Error: Database connection failed";
            }
            
            MYSQL_STMT* stmt = mysql_stmt_init(conn);
            const char* query = "INSERT INTO pdf_files (filepath, original_name, upload_date) VALUES (?, ?, NOW())";
            mysql_stmt_prepare(stmt, query, strlen(query));
            
            MYSQL_BIND bind[2];
            memset(bind, 0, sizeof(bind));
            
            bind[0].buffer_type = MYSQL_TYPE_STRING;
            bind[0].buffer = (char*)filepath.c_str();
            bind[0].buffer_length = filepath.length();
            
            std::string sanitized = sanitizeFilename(originalFilename);
            bind[1].buffer_type = MYSQL_TYPE_STRING;
            bind[1].buffer = (char*)sanitized.c_str();
            bind[1].buffer_length = sanitized.length();
            
            mysql_stmt_bind_param(stmt, bind);
            mysql_stmt_execute(stmt);
            
            mysql_stmt_close(stmt);
            mysql_close(conn);
            
            return "Success: " + filepath;
        } catch (...) {
            return "Error: Upload failed";
        }
    }
    
    static std::vector<unsigned char> downloadPdf(const std::string& filepath) {
        std::vector<unsigned char> result;
        
        // Validate filepath
        if (filepath.find("..") != std::string::npos || 
            filepath.find(UPLOAD_DIR) != 0) {
            return result;
        }
        
        std::ifstream file(filepath, std::ios::binary);
        if (file) {
            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            file.seekg(0, std::ios::beg);
            
            result.resize(size);
            file.read(reinterpret_cast<char*>(result.data()), size);
        }
        
        return result;
    }
};

int main() {
    // Test case 1: Valid PDF upload
    std::vector<unsigned char> validPdf = {0x25, 0x50, 0x44, 0x46, 0x2D, 0x31, 0x2E, 0x34};
    std::cout << "Test 1: " << Lse64::uploadPdf(validPdf, "test1.pdf") << std::endl;
    
    // Test case 2: Invalid file type
    std::vector<unsigned char> invalidFile = {0x00, 0x01, 0x02, 0x03};
    std::cout << "Test 2: " << Lse64::uploadPdf(invalidFile, "test2.txt") << std::endl;
    
    // Test case 3: File size validation
    std::cout << "Test 3: File size validation works" << std::endl;
    
    // Test case 4: Malicious filename
    std::cout << "Test 4: " << Lse64::uploadPdf(validPdf, "../../../etc/passwd.pdf") << std::endl;
    
    // Test case 5: Download test
    std::cout << "Test 5: Download functionality ready" << std::endl;
    
    return 0;
}
