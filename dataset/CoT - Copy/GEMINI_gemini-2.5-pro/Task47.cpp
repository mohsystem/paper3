// Dependencies: libcurl, libsqlite3
// How to compile and run on Linux/macOS:
// g++ this_file.cpp -o Task47 -lcurl -lsqlite3
// ./Task47
// On Windows, you will need to link against the respective libraries.

#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <sqlite3.h>

const char* DB_PATH = "scraping_results_cpp.db";

// Callback function for libcurl to write received data into a std::string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Simple and non-robust function to extract content between <title> tags
std::string extractTitle(const std::string& html) {
    std::string title;
    size_t start_pos = html.find("<title>");
    if (start_pos != std::string::npos) {
        start_pos += 7; // Length of "<title>"
        size_t end_pos = html.find("</title>", start_pos);
        if (end_pos != std::string::npos) {
            title = html.substr(start_pos, end_pos - start_pos);
            // Trim whitespace
            size_t first = title.find_first_not_of(" \t\n\r");
            if (std::string::npos == first) return "";
            size_t last = title.find_last_not_of(" \t\n\r");
            return title.substr(first, (last - first + 1));
        }
    }
    return ""; // Return empty if not found
}

void setupDatabase() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(DB_PATH, &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS pages ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "url TEXT NOT NULL UNIQUE,"
                      "title TEXT NOT NULL,"
                      "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
}

void scrapeAndStore(const std::string& url) {
    std::cout << "Scraping: " << url << std::endl;
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "CPP-Scraper-Bot/1.0");
        
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::string title = extractTitle(readBuffer);
            if (!title.empty()) {
                sqlite3* db;
                sqlite3_stmt* stmt;
                int rc = sqlite3_open(DB_PATH, &db);
                if (rc) {
                    std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
                } else {
                    const char* sql = "INSERT OR IGNORE INTO pages(url, title) VALUES(?, ?);";
                    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
                    if (rc == SQLITE_OK) {
                        sqlite3_bind_text(stmt, 1, url.c_str(), -1, SQLITE_STATIC);
                        sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_STATIC);
                        if (sqlite3_step(stmt) != SQLITE_DONE) {
                            std::cerr << "Insertion failed: " << sqlite3_errmsg(db) << std::endl;
                        } else {
                            std::cout << "Successfully stored title: \"" << title << "\"" << std::endl;
                        }
                    } else {
                        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
                    }
                    sqlite3_finalize(stmt);
                }
                sqlite3_close(db);
            } else {
                std::cerr << "Could not find title for URL: " << url << std::endl;
            }
        }
        curl_easy_cleanup(curl);
    }
}

// Callback for printing database contents
static int printCallback(void* NotUsed, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        std::cout << (argv[i] ? argv[i] : "NULL") << "\t";
    }
    std::cout << std::endl;
    return 0;
}

void printDatabaseContents() {
    std::cout << "\n--- Database Contents ---" << std::endl;
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(DB_PATH, &db);

    if(rc) {
       std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
       return;
    }
    
    const char* sql = "SELECT * from pages";
    rc = sqlite3_exec(db, sql, printCallback, 0, &zErrMsg);
    
    if( rc != SQLITE_OK ) {
       std::cerr << "SQL error: " << zErrMsg << std::endl;
       sqlite3_free(zErrMsg);
    }
    
    sqlite3_close(db);
    std::cout << "-------------------------\n" << std::endl;
}


int main() {
    setupDatabase();
    
    std::vector<std::string> testUrls = {
        "http://books.toscrape.com/",
        "http://books.toscrape.com/catalogue/category/books/travel_2/index.html",
        "http://books.toscrape.com/catalogue/a-light-in-the-attic_1000/index.html",
        "http://quotes.toscrape.com/",
        "http://quotes.toscrape.com/tag/humor/"
    };

    for (const auto& url : testUrls) {
        scrapeAndStore(url);
    }

    printDatabaseContents();
    
    return 0;
}