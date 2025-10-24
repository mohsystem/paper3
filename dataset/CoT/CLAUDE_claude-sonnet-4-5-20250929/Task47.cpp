
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <curl/curl.h>
#include <regex>
#include <stdexcept>
#include <cstring>
#include <algorithm>

class Task47 {
private:
    static const std::string DB_NAME;
    static const int TIMEOUT = 5;
    static const size_t MAX_CONTENT_LENGTH = 1000000;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t totalSize = size * nmemb;
        if (userp->size() + totalSize > MAX_CONTENT_LENGTH) {
            return 0;
        }
        userp->append((char*)contents, totalSize);
        return totalSize;
    }
    
public:
    static bool initDatabase() {
        sqlite3* db;
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        const char* sql = 
            "CREATE TABLE IF NOT EXISTS scraped_data ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "url TEXT NOT NULL,"
            "title TEXT,"
            "content TEXT,"
            "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
        
        char* errMsg = nullptr;
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_close(db);
        return true;
    }
    
    static std::string scrapeWebsite(const std::string& url) {
        if (url.empty()) {
            throw std::invalid_argument("URL cannot be empty");
        }
        
        if (!std::regex_match(url, std::regex("^https?://.*"))) {
            throw std::invalid_argument("Invalid URL format");
        }
        
        CURL* curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }
        
        std::string response;
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Secure Scraper)");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        
        CURLcode res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            curl_easy_cleanup(curl);
            throw std::runtime_error(std::string("CURL error: ") + curl_easy_strerror(res));
        }
        
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        curl_easy_cleanup(curl);
        
        if (response_code != 200) {
            throw std::runtime_error("HTTP Error: " + std::to_string(response_code));
        }
        
        return response;
    }
    
    static std::string extractTitle(const std::string& html) {
        if (html.empty()) return "";
        
        std::regex titleRegex("<title>(.*?)</title>", std::regex::icase);
        std::smatch match;
        
        if (std::regex_search(html, match, titleRegex)) {
            return sanitizeString(match[1].str());
        }
        
        return "No title found";
    }
    
    static std::string sanitizeString(const std::string& input) {
        if (input.empty()) return "";
        
        std::string sanitized = input;
        std::string dangerousChars = "<>\\"'%;()&+";
        
        sanitized.erase(
            std::remove_if(sanitized.begin(), sanitized.end(),
                [&dangerousChars](char c) {
                    return dangerousChars.find(c) != std::string::npos;
                }),
            sanitized.end()
        );
        
        return sanitized;
    }
    
    static bool storeInDatabase(const std::string& url, const std::string& title, const std::string& content) {
        if (url.empty()) {
            throw std::invalid_argument("URL cannot be empty");
        }
        
        sqlite3* db;
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        const char* sql = "INSERT INTO scraped_data (url, title, content) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt;
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        std::string sanitizedUrl = sanitizeString(url);
        std::string sanitizedTitle = sanitizeString(title);
        std::string truncatedContent = content.substr(0, std::min(content.length(), size_t(10000)));
        
        sqlite3_bind_text(stmt, 1, sanitizedUrl.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, sanitizedTitle.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, truncatedContent.c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        
        bool success = (rc == SQLITE_DONE);
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return success;
    }
    
    static void scrapeAndStore(const std::string& url) {
        std::string content = scrapeWebsite(url);
        std::string title = extractTitle(content);
        bool success = storeInDatabase(url, title, content);
        
        if (!success) {
            throw std::runtime_error("Failed to store data in database");
        }
    }
};

const std::string Task47::DB_NAME = "scraped_data.db";

int main() {
    try {
        Task47::initDatabase();
        std::cout << "Database initialized successfully" << std::endl;
        
        // Test case 1
        std::string testUrl1 = "https://example.com/test1";
        std::cout << "\\nTest 1: Storing test data for " << testUrl1 << std::endl;
        Task47::storeInDatabase(testUrl1, "Test Page 1", "<html><body>Test content 1</body></html>");
        std::cout << "Test 1 passed" << std::endl;
        
        // Test case 2
        std::string testUrl2 = "https://example.com/test2";
        std::cout << "\\nTest 2: Storing test data for " << testUrl2 << std::endl;
        Task47::storeInDatabase(testUrl2, "Test Page 2", "<html><body>Test content 2</body></html>");
        std::cout << "Test 2 passed" << std::endl;
        
        // Test case 3
        std::cout << "\\nTest 3: Extracting title" << std::endl;
        std::string html = "<html><head><title>Sample Title</title></head><body>Content</body></html>";
        std::string title = Task47::extractTitle(html);
        std::cout << "Extracted title: " << title << std::endl;
        std::cout << "Test 3 passed" << std::endl;
        
        // Test case 4
        std::cout << "\\nTest 4: String sanitization" << std::endl;
        std::string unsafe = "<script>alert('xss')</script>";
        std::string safe = Task47::sanitizeString(unsafe);
        std::cout << "Sanitized string: " << safe << std::endl;
        std::cout << "Test 4 passed" << std::endl;
        
        // Test case 5
        std::cout << "\\nTest 5: Invalid URL handling" << std::endl;
        try {
            Task47::scrapeWebsite("invalid-url");
            std::cout << "Test 5 failed - should throw exception" << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cout << "Test 5 passed - exception caught: " << e.what() << std::endl;
        }
        
        std::cout << "\\nAll tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
