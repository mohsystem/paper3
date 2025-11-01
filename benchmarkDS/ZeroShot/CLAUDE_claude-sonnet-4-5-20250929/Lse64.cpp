
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <uuid/uuid.h>

#define UPLOAD_DIR "uploads/"
#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASS "password"
#define DB_NAME "pdfdb"

class Lse64 {
private:
    static bool endsWith(const std::string& str, const std::string& suffix) {
        if (str.length() < suffix.length()) return false;
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }
    
    static std::string generateUUID() {
        uuid_t uuid;
        char uuid_str[37];
        uuid_generate(uuid);
        uuid_unparse(uuid, uuid_str);
        return std::string(uuid_str);
    }
    
    static void createDirectory(const char* path) {
        mkdir(path, 0755);
    }

public:
    static std::string upload_pdf(const char* fileContent, size_t contentSize, const std::string& originalFilename) {
        // Validate file extension
        std::string lowerFilename = originalFilename;
        for (char& c : lowerFilename) c = tolower(c);
        
        if (!endsWith(lowerFilename, ".pdf")) {
            return "Error: Only PDF files are allowed";
        }
        
        // Create uploads directory
        createDirectory(UPLOAD_DIR);
        
        // Generate unique filename
        std::string uniqueFilename = generateUUID() + "_" + originalFilename;
        std::string filepath = std::string(UPLOAD_DIR) + uniqueFilename;
        
        // Save file
        std::ofstream outFile(filepath, std::ios::binary);
        if (!outFile) {
            return "Error: Could not create file";
        }
        
        outFile.write(fileContent, contentSize);
        outFile.close();
        
        // Save to database
        if (!saveFilepathToDatabase(filepath, originalFilename)) {
            return "Error: Database operation failed";
        }
        
        return "Success: " + filepath;
    }
    
    static bool saveFilepathToDatabase(const std::string& filepath, const std::string& originalFilename) {
        MYSQL* conn = mysql_init(NULL);
        
        if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
            mysql_close(conn);
            return false;
        }
        
        std::string query = "INSERT INTO pdf_files (filepath, original_name, upload_date) VALUES (?, ?, NOW())";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)filepath.c_str();
        bind[0].buffer_length = filepath.length();
        
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)originalFilename.c_str();
        bind[1].buffer_length = originalFilename.length();
        
        mysql_stmt_bind_param(stmt, bind);
        mysql_stmt_execute(stmt);
        
        mysql_stmt_close(stmt);
        mysql_close(conn);
        
        return true;
    }
    
    static char* download_pdf(const std::string& filepath, size_t& fileSize) {
        // Validate filepath
        if (filepath.find("..") != std::string::npos || 
            filepath.substr(0, strlen(UPLOAD_DIR)) != UPLOAD_DIR) {
            fileSize = 0;
            return nullptr;
        }
        
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);
        if (!file) {
            fileSize = 0;
            return nullptr;
        }
        
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        char* buffer = new char[fileSize];
        file.read(buffer, fileSize);
        file.close();
        
        return buffer;
    }
};

int main() {
    // Test case 1: Valid PDF upload
    std::cout << "Test 1: Valid PDF upload" << std::endl;
    const char* content1 = "PDF content test 1";
    std::string result1 = Lse64::upload_pdf(content1, strlen(content1), "test1.pdf");
    std::cout << result1 << std::endl;
    
    // Test case 2: Valid PDF upload
    std::cout << "\\nTest 2: Valid PDF upload" << std::endl;
    const char* content2 = "PDF content test 2";
    std::string result2 = Lse64::upload_pdf(content2, strlen(content2), "document.pdf");
    std::cout << result2 << std::endl;
    
    // Test case 3: Invalid file extension
    std::cout << "\\nTest 3: Invalid file extension" << std::endl;
    const char* content3 = "Not a PDF";
    std::string result3 = Lse64::upload_pdf(content3, strlen(content3), "malicious.exe");
    std::cout << result3 << std::endl;
    
    // Test case 4: Download valid file
    std::cout << "\\nTest 4: Download valid file" << std::endl;
    size_t fileSize;
    char* downloadResult = Lse64::download_pdf("uploads/test.pdf", fileSize);
    std::cout << (downloadResult != nullptr ? "Success" : "Failed") << std::endl;
    if (downloadResult) delete[] downloadResult;
    
    // Test case 5: Directory traversal attempt
    std::cout << "\\nTest 5: Directory traversal attempt" << std::endl;
    char* traversalResult = Lse64::download_pdf("../../../etc/passwd", fileSize);
    std::cout << (traversalResult == nullptr ? "Blocked (Success)" : "Failed") << std::endl;
    if (traversalResult) delete[] traversalResult;
    
    return 0;
}
