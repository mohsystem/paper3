/*
 * Instructions for running this C code:
 * 1. You need to install two libraries: libcurl and sqlite3.
 *    On Debian/Ubuntu:
 *    sudo apt-get update
 *    sudo apt-get install libcurl4-openssl-dev libsqlite3-dev
 *    On RedHat/CentOS:
 *    sudo yum install libcurl-devel sqlite-devel
 *    On macOS (using Homebrew):
 *    brew install curl sqlite
 * 2. Compile the code from your terminal, linking against these libraries.
 *    gcc Task47.c -o Task47 -lcurl -lsqlite3
 * 3. Run the compiled executable:
 *    ./Task47
 *
 * Note: The HTML parsing in this C example is very basic and fragile.
 * It uses string searching (strstr) and is highly dependent on the exact
 * format of the target website's HTML. A real-world C application would use
 * a proper parsing library like libxml2, but that would add more complexity
 * and dependencies to this example.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>

// Struct to hold the fetched data from curl
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback function for libcurl to write data into our MemoryStruct
static size_t write_memory_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Function to perform very basic parsing and insert data into the DB
void parse_and_store(sqlite3 *db, const char *html) {
    const char *cursor = html;
    sqlite3_stmt *stmt;
    const char *sql_insert = "INSERT INTO books (title, price) VALUES (?, ?);";
    
    if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }
    
    int book_count = 0;
    // Loop through the HTML looking for book entries
    while ((cursor = strstr(cursor, "<article class=\"product_pod\">"))) {
        // Find title
        const char *title_start = strstr(cursor, "title=\"");
        if (!title_start) continue;
        title_start += 7; // Move past 'title="'
        const char *title_end = strchr(title_start, '\"');
        if (!title_end) continue;
        
        // Find price
        const char *price_start = strstr(cursor, "price_color\">");
        if (!price_start) continue;
        price_start += 13; // Move past 'price_color">'
        const char *price_end = strstr(price_start, "</p>");
        if (!price_end) continue;

        // Extract title and price strings
        char title[256];
        char price[32];
        strncpy(title, title_start, title_end - title_start);
        title[title_end - title_start] = '\0';
        strncpy(price, price_start, price_end - price_start);
        price[price_end - price_start] = '\0';
        
        // Insert into database
        sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, price, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_reset(stmt);
        
        book_count++;
        cursor = price_end; // Move cursor to continue search
    }
    printf("Scraped and stored %d books.\n", book_count);
    sqlite3_finalize(stmt);
}

void scrape_and_store(const char* url, const char* db_name) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    sqlite3 *db;
    
    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    
    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        // Open database
        if (sqlite3_open(db_name, &db)) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        } else {
            // Setup table
            char *err_msg = 0;
            const char* sql_create = "CREATE TABLE IF NOT EXISTS books (id INTEGER PRIMARY KEY, title TEXT, price TEXT);";
            if (sqlite3_exec(db, sql_create, 0, 0, &err_msg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", err_msg);
                sqlite3_free(err_msg);
            } else {
                 parse_and_store(db, chunk.memory);
            }
            sqlite3_close(db);
        }
    }

    // Cleanup
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();
}

void print_data(const char* db_name) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    printf("\n--- Data in %s ---\n", db_name);
    if (sqlite3_open(db_name, &db)) return;

    const char* sql = "SELECT id, title, price FROM books;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = 1;
        printf("ID: %d\tTitle: %s\tPrice: %s\n",
               sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_text(stmt, 2));
    }
     if (!found) {
        printf("No data found in the 'books' table.\n");
    }
    printf("--------------------------\n");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void clear_table(const char* db_name) {
    sqlite3 *db;
    if (sqlite3_open(db_name, &db)) return;
    
    char *err_msg = 0;
    const char* sql = "DELETE FROM books;";
    if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error on DELETE: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("\nTable 'books' cleared.\n");
    }
    sqlite3_close(db);
}

int main(void) {
    const char* db_name = "scraper_db.sqlite";
    const char* url1 = "http://books.toscrape.com/";
    const char* url2 = "http://books.toscrape.com/catalogue/page-2.html";

    // Test Case 1: Scrape data from the first URL
    printf("--- Test Case 1: Scraping %s ---\n", url1);
    scrape_and_store(url1, db_name);

    // Test Case 2: Print the stored data
    printf("\n--- Test Case 2: Printing data ---\n");
    print_data(db_name);

    // Test Case 3: Clear the database table
    printf("\n--- Test Case 3: Clearing table ---\n");
    clear_table(db_name);

    // Test Case 4: Print the data again to confirm it's empty
    printf("\n--- Test Case 4: Printing empty table ---\n");
    print_data(db_name);
    
    // Test Case 5: Scrape data from the second URL and print it
    printf("\n--- Test Case 5: Scraping %s and printing ---\n", url2);
    scrape_and_store(url2, db_name);
    print_data(db_name);

    return 0;
}