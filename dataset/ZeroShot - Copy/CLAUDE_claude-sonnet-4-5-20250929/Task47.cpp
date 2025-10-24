
#include <iostream>
#include <string>
#include <regex>
#include <curl/curl.h>
#include <sqlite3.h>
#include <ctime>

class Task47 {
private:
    std::string db_name;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        userp->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    
    bool isValidUrl(const std::string& url) {
        std::regex url_regex("^https?://[^\\\\s/$.?#].[^\\\\s]*$");
        return std::regex_match(url, url_regex);
    }
    
    std::string sanitizeTableName(const std::string& name) {
        std::string sanitized = name;
        std::regex invalid_chars("[^a-zA-Z0-9_]");
        return std::regex_replace(sanitized, invalid_chars, "_");
    }
    
    void initializeDatabase(const std::string& tableName) {
        sqlite3* db;
        char* errMsg = nullptr;
        
        if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Cannot open database" << std::endl;
            return;
        }
        
        std::string sanitized = sanitizeTableName(tableName);
        std::string sql = "CREATE TABLE IF NOT EXISTS " + sanitized + 
                         " (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "url TEXT NOT NULL, "
                         "content TEXT, "
                         "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
        
        sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
        sqlite3_close(db);
    }
    
    std::string scrapeWebsite(const std::string& url) {
        CURL* curl;
        CURLcode res;
        std::string readBuffer;
        
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            
            if (res != CURLE_OK) {
                throw std::runtime_error("Failed to fetch URL");
            }
        }
        
        return readBuffer;
    }
    
    void storeData(const std::string& tableName, const std::string& url, 
                   const std::string& content) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Cannot open database" << std::endl;
            return;
        }
        
        std::string sanitized = sanitizeTableName(tableName);
        std::string sql = "INSERT INTO " + sanitized + " (url, content) VALUES (?, ?)";
        
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, url.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_TRANSIENT);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
    
public:
    Task47(const std::string& dbName = "scraped_data.db") : db_name(dbName) {}
    
    void scrapeAndStore(const std::string& url, const std::string& tableName) {
        try {
            if (!isValidUrl(url)) {
                std::cout << "Invalid URL provided" << std::endl;
                return;
            }
            
            initializeDatabase(tableName);
            std::string content = scrapeWebsite(url);
            storeData(tableName, url, content);
            
            std::cout << "Data scraped and stored successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
};

int main() {
    std::cout << "=== Web Scraper Test Cases ===" << std::endl << std::endl;
    Task47 scraper;
    
    std::cout << "Test 1: Scraping example.com" << std::endl;
    scraper.scrapeAndStore("http://example.com", "test_data");
    
    std::cout << "\\nTest 2: Invalid URL" << std::endl;
    scraper.scrapeAndStore("invalid-url", "test_data");
    
    std::cout << "\\nTest 3: HTTPS URL" << std::endl;
    scraper.scrapeAndStore("https://example.com", "secure_data");
    
    std::cout << "\\nTest 4: Different table" << std::endl;
    scraper.scrapeAndStore("http://example.com", "website_content");
    
    std::cout << "\\nTest 5: Table name sanitization" << std::endl;
    scraper.scrapeAndStore("http://example.com", "data-table-2024");
    
    return 0;
}
