
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>
#include <curl/curl.h>

class Task47 {
private:
    static const std::string DB_NAME;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    
    static void createDatabase() {
        sqlite3* db;
        char* errMsg = 0;
        
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        
        const char* sql = "CREATE TABLE IF NOT EXISTS scraped_data ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "url TEXT, "
                         "title TEXT, "
                         "content TEXT, "
                         "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
        
        rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
        sqlite3_close(db);
    }
    
    static std::string scrapeWebsite(const std::string& url) {
        CURL* curl;
        CURLcode res;
        std::string content;
        
        curl = curl_easy_init();
        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }
        
        return content;
    }
    
public:
    static void scrapeAndStore(const std::string& url) {
        try {
            createDatabase();
            
            std::string content = scrapeWebsite(url);
            
            std::regex pattern("<a[^>]+href=\\"([^\\"]*)\\"[^>]*>([^<]*)</a>");
            std::smatch matches;
            
            sqlite3* db;
            sqlite3_open(DB_NAME.c_str(), &db);
            
            std::string sql = "INSERT INTO scraped_data (url, title, content) VALUES (?, ?, ?);";
            sqlite3_stmt* stmt;
            
            auto it = std::sregex_iterator(content.begin(), content.end(), pattern);
            auto end = std::sregex_iterator();
            
            for (; it != end; ++it) {
                std::smatch match = *it;
                std::string link = match[1].str();
                std::string title = match[2].str();
                
                sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, 0);
                sqlite3_bind_text(stmt, 1, link.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, content.substr(0, 500).c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
            
            sqlite3_close(db);
            
            std::cout << "Data scraped and stored successfully from: " << url << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
};

const std::string Task47::DB_NAME = "scraped_data.db";

int main() {
    std::cout << "Web Scraper Test Cases:" << std::endl;
    std::cout << "========================\\n" << std::endl;
    
    // Test case 1
    std::cout << "Test Case 1: Scraping example.com" << std::endl;
    Task47::scrapeAndStore("http://example.com");
    
    // Test case 2
    std::cout << "\\nTest Case 2: Scraping example.org" << std::endl;
    Task47::scrapeAndStore("http://example.org");
    
    // Test case 3
    std::cout << "\\nTest Case 3: Scraping example.net" << std::endl;
    Task47::scrapeAndStore("http://example.net");
    
    // Test case 4
    std::cout << "\\nTest Case 4: Invalid URL test" << std::endl;
    Task47::scrapeAndStore("http://invalid-url-that-does-not-exist-12345.com");
    
    // Test case 5
    std::cout << "\\nTest Case 5: Another valid URL" << std::endl;
    Task47::scrapeAndStore("http://info.cern.ch");
    
    return 0;
}
