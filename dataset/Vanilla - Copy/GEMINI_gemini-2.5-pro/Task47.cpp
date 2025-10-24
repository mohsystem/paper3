/*
 * Instructions for running this C++ code:
 * 1. You need to install two libraries: libcurl and sqlite3.
 *    On Debian/Ubuntu:
 *    sudo apt-get update
 *    sudo apt-get install libcurl4-openssl-dev libsqlite3-dev
 *    On RedHat/CentOS:
 *    sudo yum install libcurl-devel sqlite-devel
 *    On macOS (using Homebrew):
 *    brew install curl sqlite
 * 2. Compile the code from your terminal, linking against these libraries.
 *    g++ -std=c++11 Task47.cpp -o Task47 -lcurl -lsqlite3
 * 3. Run the compiled executable:
 *    ./Task47
 */
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <curl/curl.h>
#include <sqlite3.h>

// Callback function for libcurl to write received data into a string
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to fetch HTML content from a URL
std::string fetch_html(const std::string& url) {
    CURL *curl;
    CURLcode res;
    std::string read_buffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
        // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        // Provide a user agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return read_buffer;
}

// Function to store scraped data in SQLite
void store_data(const std::string& db_name, const std::vector<std::pair<std::string, std::string>>& books) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql_create = "CREATE TABLE IF NOT EXISTS books ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "title TEXT NOT NULL, price TEXT NOT NULL);";
    if (sqlite3_exec(db, sql_create, 0, 0, 0) != SQLITE_OK) {
        std::cerr << "SQL error on CREATE: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    const char* sql_insert = "INSERT INTO books (title, price) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    for (const auto& book : books) {
        sqlite3_bind_text(stmt, 1, book.first.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, book.second.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Main function to scrape a URL and store the data
void scrape_and_store(const std::string& url, const std::string& db_name) {
    std::string html = fetch_html(url);
    if (html.empty()) {
        std::cerr << "Failed to fetch HTML from " << url << std::endl;
        return;
    }

    std::vector<std::pair<std::string, std::string>> books;
    std::regex article_regex("<article class=\"product_pod\">(.*?)</article>");
    std::regex title_regex("<h3><a href=\".*?\" title=\"(.*?)\">");
    std::regex price_regex("<p class=\"price_color\">(.*?)</p>");

    auto articles_begin = std::sregex_iterator(html.begin(), html.end(), article_regex);
    auto articles_end = std::sregex_iterator();

    for (std::sregex_iterator i = articles_begin; i != articles_end; ++i) {
        std::smatch article_match = *i;
        std::string article_html = article_match.str(1);
        std::smatch title_match;
        std::smatch price_match;

        if (std::regex_search(article_html, title_match, title_regex) &&
            std::regex_search(article_html, price_match, price_regex)) {
            books.push_back({title_match[1].str(), price_match[1].str()});
        }
    }
    
    store_data(db_name, books);
    std::cout << "Scraped and stored " << books.size() << " books from " << url << std::endl;
}

void print_data(const std::string& db_name) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    std::cout << "\n--- Data in " << db_name << " ---" << std::endl;
    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql = "SELECT id, title, price FROM books;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    bool found = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = true;
        std::cout << "ID: " << sqlite3_column_int(stmt, 0) << "\t"
                  << "Title: " << sqlite3_column_text(stmt, 1) << "\t"
                  << "Price: " << sqlite3_column_text(stmt, 2) << std::endl;
    }
    if (!found) {
        std::cout << "No data found in the 'books' table." << std::endl;
    }
    std::cout << "--------------------------" << std::endl;

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void clear_table(const std::string& db_name) {
    sqlite3 *db;
    if (sqlite3_open(db_name.c_str(), &db) != SQLITE_OK) return;
    
    const char* sql = "DELETE FROM books;";
    if (sqlite3_exec(db, sql, 0, 0, 0) != SQLITE_OK) {
         std::cerr << "SQL error on DELETE: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "\nTable 'books' cleared." << std::endl;
    }
    sqlite3_close(db);
}

int main() {
    const std::string db_name = "scraper_db.sqlite";
    const std::string url1 = "http://books.toscrape.com/";
    const std::string url2 = "http://books.toscrape.com/catalogue/page-2.html";

    // Test Case 1: Scrape data from the first URL
    std::cout << "--- Test Case 1: Scraping " << url1 << " ---" << std::endl;
    scrape_and_store(url1, db_name);

    // Test Case 2: Print the stored data
    std::cout << "\n--- Test Case 2: Printing data ---" << std::endl;
    print_data(db_name);

    // Test Case 3: Clear the database table
    std::cout << "\n--- Test Case 3: Clearing table ---" << std::endl;
    clear_table(db_name);

    // Test Case 4: Print the data again to confirm it's empty
    std::cout << "\n--- Test Case 4: Printing empty table ---" << std::endl;
    print_data(db_name);
    
    // Test Case 5: Scrape data from the second URL and print it
    std::cout << "\n--- Test Case 5: Scraping " << url2 << " and printing ---" << std::endl;
    scrape_and_store(url2, db_name);
    print_data(db_name);

    return 0;
}