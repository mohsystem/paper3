
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <mysql/mysql.h>
#include <algorithm>
#include <set>

// Maximum file size: 5MB to prevent DoS
const size_t MAX_FILE_SIZE = 5 * 1024 * 1024;

// Maximum filename length
const size_t MAX_FILENAME_LENGTH = 255;

// Allowed file extensions - whitelist for security
const std::set<std::string> ALLOWED_EXTENSIONS = {
    ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"
};

/**
 * Converts string to lowercase for case-insensitive comparison
 */
std::string to_lowercase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

/**
 * Extracts file extension from filename
 */
std::string get_file_extension(const std::string& filename) {
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return "";
    }
    return to_lowercase(filename.substr(dot_pos));
}

/**
 * Extracts filename from path
 */
std::string get_filename_from_path(const std::string& path) {
    size_t slash_pos = path.find_last_of('/');
    if (slash_pos == std::string::npos) {
        slash_pos = path.find_last_of('\\\\');\n    }\n    if (slash_pos == std::string::npos) {\n        return path;\n    }\n    return path.substr(slash_pos + 1);\n}\n\n/**\n * Securely stores an image file in MySQL database\n */\nbool store_image_in_database(const char* file_path, const char* db_host,\n                             const char* db_user, const char* db_password,\n                             const char* db_name) {\n    // Input validation: reject NULL or empty paths\n    if (file_path == NULL || strlen(file_path) == 0) {\n        std::cerr << "Error: File path cannot be NULL or empty" << std::endl;\n        return false;\n    }\n    \n    MYSQL* conn = NULL;\n    MYSQL_STMT* stmt = NULL;\n    int fd = -1;\n    unsigned char* image_data = NULL;\n    \n    // Secure file handling: open first with O_RDONLY and O_NOFOLLOW\n    fd = open(file_path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);\n    if (fd < 0) {\n        std::cerr << "Error: Failed to open file" << std::endl;\n        return false;\n    }\n    \n    // Validation: fstat the opened descriptor (TOCTOU prevention)\n    struct stat st;\n    if (fstat(fd, &st) != 0) {\n        std::cerr << "Error: Failed to stat file" << std::endl;\n        close(fd);\n        return false;\n    }\n    \n    // Validation: ensure it's a regular file
    if (!S_ISREG(st.st_mode)) {
        std::cerr << "Error: Not a regular file" << std::endl;
        close(fd);
        return false;
    }
    
    // Validation: check file size to prevent DoS
    if (st.st_size > static_cast<off_t>(MAX_FILE_SIZE)) {
        std::cerr << "Error: File size exceeds maximum" << std::endl;
        close(fd);
        return false;
    }
    
    if (st.st_size == 0) {
        std::cerr << "Error: File is empty" << std::endl;
        close(fd);
        return false;
    }
    
    size_t file_size = static_cast<size_t>(st.st_size);
    
    // Extract and validate filename
    std::string filename = get_filename_from_path(file_path);
    if (filename.length() > MAX_FILENAME_LENGTH) {
        std::cerr << "Error: Filename too long" << std::endl;
        close(fd);
        return false;
    }
    
    // Validation: check file extension
    std::string extension = get_file_extension(filename);
    if (ALLOWED_EXTENSIONS.find(extension) == ALLOWED_EXTENSIONS.end()) {
        std::cerr << "Error: Invalid or unsupported file type" << std::endl;
        close(fd);
        return false;
    }
    
    // Allocate buffer for image data with overflow check
    if (file_size > SIZE_MAX) {
        std::cerr << "Error: File size overflow" << std::endl;
        close(fd);
        return false;
    }
    
    image_data = new (std::nothrow) unsigned char[file_size];
    if (image_data == NULL) {
        std::cerr << "Error: Memory allocation failed" << std::endl;
        close(fd);
        return false;
    }
    
    // Read file content using the opened fd
    size_t total_read = 0;
    while (total_read < file_size) {
        ssize_t bytes_read = read(fd, image_data + total_read, file_size - total_read);
        if (bytes_read < 0) {
            std::cerr << "Error: Failed to read file" << std::endl;
            delete[] image_data;
            close(fd);
            return false;
        }
        if (bytes_read == 0) {
            break;
        }
        total_read += bytes_read;
    }
    
    close(fd);
    fd = -1;
    
    if (total_read != file_size) {
        std::cerr << "Error: Failed to read complete file" << std::endl;
        delete[] image_data;
        return false;
    }
    
    // Initialize MySQL connection
    conn = mysql_init(NULL);
    if (conn == NULL) {
        std::cerr << "Error: MySQL init failed" << std::endl;
        delete[] image_data;
        return false;
    }
    
    // Connect to database
    if (mysql_real_connect(conn, db_host, db_user, db_password, db_name, 0, NULL, 0) == NULL) {
        std::cerr << "Error: Database connection failed" << std::endl;
        mysql_close(conn);
        delete[] image_data;
        return false;
    }
    
    // Prepare parameterized statement to prevent SQL injection
    const char* query = "INSERT INTO images (filename, file_size, image_data) VALUES (?, ?, ?)";
    stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        std::cerr << "Error: Statement init failed" << std::endl;
        mysql_close(conn);
        delete[] image_data;
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        std::cerr << "Error: Statement prepare failed" << std::endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        delete[] image_data;
        return false;
    }
    
    // Bind parameters
    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));
    
    // Filename parameter
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = const_cast<char*>(filename.c_str());
    bind[0].buffer_length = filename.length();
    
    // File size parameter
    unsigned long long size_param = file_size;
    bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[1].buffer = &size_param;
    bind[1].is_unsigned = 1;
    
    // Image data parameter
    unsigned long data_length = file_size;
    bind[2].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[2].buffer = image_data;
    bind[2].buffer_length = file_size;
    bind[2].length = &data_length;
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        std::cerr << "Error: Parameter binding failed" << std::endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        delete[] image_data;
        return false;
    }
    
    // Execute statement
    bool success = (mysql_stmt_execute(stmt) == 0);
    
    // Cleanup resources
    mysql_stmt_close(stmt);
    mysql_close(conn);
    delete[] image_data;
    
    return success;
}

int main() {
    // Test configuration
    const char* db_host = "localhost";
    const char* db_user = "testuser";
    const char* db_password = "testpass";
    const char* db_name = "imagedb";
    
    // Test case 1: Valid JPEG
    std::cout << "Test 1 - Valid JPEG:" << std::endl;
    bool result1 = store_image_in_database("test1.jpg", db_host, db_user, db_password, db_name);
    std::cout << "Result: " << (result1 ? "true" : "false") << "\\n" << std::endl;
    
    // Test case 2: Valid PNG
    std::cout << "Test 2 - Valid PNG:" << std::endl;
    bool result2 = store_image_in_database("test2.png", db_host, db_user, db_password, db_name);
    std::cout << "Result: " << (result2 ? "true" : "false") << "\\n" << std::endl;
    
    // Test case 3: NULL path
    std::cout << "Test 3 - NULL path:" << std::endl;
    bool result3 = store_image_in_database(NULL, db_host, db_user, db_password, db_name);
    std::cout << "Result: " << (result3 ? "true" : "false") << "\\n" << std::endl;
    
    // Test case 4: Empty path
    std::cout << "Test 4 - Empty path:" << std::endl;
    bool result4 = store_image_in_database("", db_host, db_user, db_password, db_name);
    std::cout << "Result: " << (result4 ? "true" : "false") << "\\n" << std::endl;
    
    // Test case 5: Non-existent file
    std::cout << "Test 5 - Non-existent file:" << std::endl;
    bool result5 = store_image_in_database("nonexistent.jpg", db_host, db_user, db_password, db_name);
    std::cout << "Result: " << (result5 ? "true" : "false") << std::endl;
    
    return 0;
}
