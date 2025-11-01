
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <mysql/mysql.h>
#include <sys/stat.h>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

const string UPLOAD_DIR = "uploads/";
const string DB_HOST = "localhost";
const string DB_USER = "root";
const string DB_PASS = "password";
const string DB_NAME = "pdf_db";

class Lse64 {
private:
    static bool createDirectory(const string& path) {
        struct stat info;
        if (stat(path.c_str(), &info) != 0) {
            return mkdir(path.c_str(), 0777) == 0;
        }
        return true;
    }
    
    static bool copyFile(const string& source, const string& destination) {
        try {
            fs::copy_file(source, destination, fs::copy_options::overwrite_existing);
            return true;
        } catch (exception& e) {
            cerr << "Error copying file: " << e.what() << endl;
            return false;
        }
    }
    
    static void saveToDatabase(const string& filepath, const string& filename) {
        MYSQL* conn = mysql_init(NULL);
        
        if (!mysql_real_connect(conn, DB_HOST.c_str(), DB_USER.c_str(), 
                               DB_PASS.c_str(), DB_NAME.c_str(), 0, NULL, 0)) {
            cerr << "Database connection failed: " << mysql_error(conn) << endl;
            return;
        }
        
        string query = "INSERT INTO pdf_files (filepath, filename, upload_date) VALUES ('" + 
                      filepath + "', '" + filename + "', NOW())";
        
        if (mysql_query(conn, query.c_str())) {
            cerr << "Insert failed: " << mysql_error(conn) << endl;
        }
        
        mysql_close(conn);
    }

public:
    static string uploadPdf(const string& sourceFilePath, const string& fileName) {
        try {
            createDirectory(UPLOAD_DIR);
            
            string uniqueFileName = to_string(time(NULL)) + "_" + fileName;
            string destinationPath = UPLOAD_DIR + uniqueFileName;
            
            if (!copyFile(sourceFilePath, destinationPath)) {
                return "";
            }
            
            saveToDatabase(destinationPath, fileName);
            
            return destinationPath;
        } catch (exception& e) {
            cerr << "Error uploading PDF: " << e.what() << endl;
            return "";
        }
    }
    
    static bool downloadPdf(int fileId, const string& downloadPath) {
        MYSQL* conn = mysql_init(NULL);
        
        if (!mysql_real_connect(conn, DB_HOST.c_str(), DB_USER.c_str(), 
                               DB_PASS.c_str(), DB_NAME.c_str(), 0, NULL, 0)) {
            cerr << "Database connection failed: " << mysql_error(conn) << endl;
            return false;
        }
        
        string query = "SELECT filepath FROM pdf_files WHERE id = " + to_string(fileId);
        
        if (mysql_query(conn, query.c_str())) {
            cerr << "Query failed: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return false;
        }
        
        MYSQL_RES* result = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(result);
        
        if (row) {
            string filepath = row[0];
            bool success = copyFile(filepath, downloadPath);
            mysql_free_result(result);
            mysql_close(conn);
            return success;
        }
        
        mysql_free_result(result);
        mysql_close(conn);
        return false;
    }
};

int main() {
    cout << "PDF Upload/Download System Test Cases:" << endl;
    
    // Test Case 1: Upload a PDF file
    cout << "\\nTest Case 1: Upload PDF" << endl;
    string result1 = Lse64::uploadPdf("test1.pdf", "document1.pdf");
    cout << "Upload result: " << (result1.empty() ? "Failed" : "Success - " + result1) << endl;
    
    // Test Case 2: Upload another PDF file
    cout << "\\nTest Case 2: Upload another PDF" << endl;
    string result2 = Lse64::uploadPdf("test2.pdf", "document2.pdf");
    cout << "Upload result: " << (result2.empty() ? "Failed" : "Success - " + result2) << endl;
    
    // Test Case 3: Upload PDF with special characters
    cout << "\\nTest Case 3: Upload PDF with special name" << endl;
    string result3 = Lse64::uploadPdf("test3.pdf", "my_document_2024.pdf");
    cout << "Upload result: " << (result3.empty() ? "Failed" : "Success - " + result3) << endl;
    
    // Test Case 4: Download PDF by ID
    cout << "\\nTest Case 4: Download PDF" << endl;
    bool download1 = Lse64::downloadPdf(1, "downloaded_file1.pdf");
    cout << "Download result: " << (download1 ? "Success" : "Failed") << endl;
    
    // Test Case 5: Download non-existent PDF
    cout << "\\nTest Case 5: Download non-existent PDF" << endl;
    bool download2 = Lse64::downloadPdf(9999, "downloaded_file2.pdf");
    cout << "Download result: " << (download2 ? "Success" : "Failed (Expected)") << endl;
    
    return 0;
}
