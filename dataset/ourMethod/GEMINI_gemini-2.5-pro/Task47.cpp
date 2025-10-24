/*
 * Instructions:
 * 1. This is a single C++ source file.
 * 2. It requires libcurl and libsqlite3 to be installed.
 * 3. On Debian/Ubuntu: sudo apt-get install libcurl4-openssl-dev libsqlite3-dev
 * 4. On Fedora/CentOS: sudo dnf install libcurl-devel sqlite-devel
 * 5. On macOS (with Homebrew): brew install curl sqlite
 *
 * Compilation command:
 * g++ your_source_file.cpp -o scraper_cpp -lcurl -lsqlite3 -std=c++11
 */

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <chrono>
#include <iomanip>
#include <sstream>

#include <curl/curl.h>
#include <sqlite3.h>

// RAII wrapper for sqlite3 database connection to ensure it's always closed.
class SQLiteDB {
private:
    sqlite3* db;

public:
    explicit SQLiteDB(const char* filename) : db(nullptr) {
        if (sqlite3_open(filename, &db)) {
            std::string errMsg = "Can't open database: ";
            if (db) {
                errMsg += sqlite3_errmsg(db);
                sqlite3_close(db); // sqlite3_close can be called on a failed open
            } else {
                errMsg += "memory allocation error";
            }
            db = nullptr;
            throw std::runtime_error(errMsg);
        }
    }

    ~SQLiteDB() {
        if (db) {
            sqlite3_close(db);
        }
    }

    // Disable copy and assign to prevent duplicate ownership.
    SQLiteDB(const SQLiteDB&) = delete;
    SQLiteDB& operator=(const SQLiteDB&) = delete;

    sqlite3* get() const {
        return db;
    }
};

// Callback function for libcurl to write received data into a std::string.
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    try {
        static_cast<std::string*>(userp)->append(static_cast<char*>(contents), realsize);
        return realsize;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed in WriteCallback: " << e.what() << std::endl;
        return 0; // Returning 0 will signal an error to libcurl.
    }
}

// Scrapes the HTML content of a given URL.
std::string scrape_website(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize libcurl.");
    }
    // RAII for CURL handle
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl_ptr(curl, &curl_easy_cleanup);

    std::string readBuffer;
    
    // Set libcurl options.
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects.
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "simple-scraper/1.0"); // Set a user agent.
    // By default, libcurl performs peer SSL/TLS certificate verification (Rule #1)
    // and hostname verification (Rule #2). We do not disable these features.

    CURLcode res = curl_easy_perform(curl);
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }
    
    if (http_code < 200 || http_code >= 300) {
        throw std::runtime_error("HTTP request failed with status code: " + std::to_string(http_code));
    }

    return readBuffer;
}

// Parses the <title> tag from HTML content.
// Note: This is a very basic parser and will fail on complex HTML.
// A proper HTML parsing library would be more robust.
std::string parse_title(const std::string& html) {
    size_t title_start = html.find("<title>");
    if (title_start == std::string::npos) {
        title_start = html.find("<TITLE>");
    }

    if (title_start == std::string::npos) {
        return "No title found";
    }
    title_start += 7; // Length of "<title>" or "<TITLE>"

    size_t title_end = html.find("</title>", title_start);
     if (title_end == std::string::npos) {
        title_end = html.find("</TITLE>", title_start);
    }
    
    if (title_end == std::string::npos) {
        return "No closing title tag found";
    }

    return html.substr(title_start, title_end - title_start);
}

// Initializes the database and creates the necessary table.
void init_db(sqlite3* db) {
    const char* sql = "CREATE TABLE IF NOT EXISTS scraped_data ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "url TEXT NOT NULL, "
                      "title TEXT, "
                      "timestamp TEXT NOT NULL);";
    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::string errMsg = "SQL error: " + std::string(zErrMsg);
        sqlite3_free(zErrMsg);
        throw std::runtime_error(errMsg);
    }
}

// Stores the scraped data into the database using prepared statements.
void store_in_db(sqlite3* db, const std::string& url, const std::string& title) {
    auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    std::string timestamp = ss.str();

    const char* sql = "INSERT INTO scraped_data (url, title, timestamp) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }
    
    // Use a smart pointer for RAII on the statement handle.
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt_ptr(stmt, &sqlite3_finalize);

    // Bind parameters to prevent SQL injection (Rule #3).
    sqlite3_bind_text(stmt_ptr.get(), 1, url.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_ptr.get(), 2, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_ptr.get(), 3, timestamp.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt_ptr.get());
    if (rc != SQLITE_DONE) {
         throw std::runtime_error("Failed to execute statement: " + std::string(sqlite3_errmsg(db)));
    }
}

int main() {
    const char* db_file = "scraped_data.db";
    
    try {
        SQLiteDB db(db_file);
        init_db(db.get());
        
        std::vector<std::string> test_urls = {
            "https://www.example.com",
            "https://www.google.com",
            "https://www.github.com",
            "https://www.iana.org/domains/reserved",
            "https://www.w3.org"
        };
        
        for (const auto& url : test_urls) {
            try {
                std::cout << "Scraping " << url << "..." << std::endl;
                std::string html = scrape_website(url);
                std::string title = parse_title(html);
                store_in_db(db.get(), url, title);
                std::cout << "  -> Stored title: " << title << std::endl;
            } catch (const std::runtime_error& e) {
                std::cerr << "  -> Error processing " << url << ": " << e.what() << std::endl;
            }
        }
        
    } catch (const std::runtime_error& e) {
        std::cerr << "A critical error occurred: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\nScraping complete. Data saved to " << db_file << std::endl;
    return 0;
}