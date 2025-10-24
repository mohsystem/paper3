/*
 * Instructions:
 * 1. This is a single C source file.
 * 2. It requires libcurl and libsqlite3 to be installed.
 * 3. On Debian/Ubuntu: sudo apt-get install libcurl4-openssl-dev libsqlite3-dev
 * 4. On Fedora/CentOS: sudo dnf install libcurl-devel sqlite-devel
 * 5. On macOS (with Homebrew): brew install curl sqlite
 *
 * Compilation command:
 * gcc your_source_file.c -o scraper_c -lcurl -lsqlite3
 *
 * Note: strcasestr is a non-standard but common extension used for case-insensitive
 * search. If not available on your system, you may need to implement it or use
 * a case-sensitive search with strstr.
 */
#define _DEFAULT_SOURCE // For strcasestr on glibc
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <sqlite3.h>

// Max allowed response size to prevent memory exhaustion (Rule #5)
#define MAX_RESPONSE_SIZE (10 * 1024 * 1024) // 10 MB

// Struct to hold memory buffer for libcurl response.
typedef struct {
    char* memory;
    size_t size;
} MemoryStruct;

// Callback function for libcurl to write data into our MemoryStruct.
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    MemoryStruct* mem = (MemoryStruct*)userp;
    
    if (mem->size + realsize >= MAX_RESPONSE_SIZE) {
        fprintf(stderr, "Error: Exceeded maximum response size of %d bytes.\n", MAX_RESPONSE_SIZE);
        return 0; // Signal error to curl.
    }
    
    char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        fprintf(stderr, "Error: not enough memory (realloc returned NULL)\n");
        return 0; // Signal error to curl.
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0; // Null-terminate the string.

    return realsize;
}

// Scrapes the HTML content of a given URL.
// The caller is responsible for freeing the chunk->memory.
int scrape_website(const char* url, MemoryStruct* chunk) {
    CURL* curl_handle = NULL;
    CURLcode res;
    
    // Initialize the memory chunk.
    chunk->memory = (char*)malloc(1);
    if (chunk->memory == NULL) {
        fprintf(stderr, "Error: malloc failed to allocate memory.\n");
        return 1;
    }
    chunk->size = 0;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "Error: Failed to initialize libcurl.\n");
        free(chunk->memory);
        chunk->memory = NULL;
        curl_global_cleanup();
        return 1;
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)chunk);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "simple-scraper-c/1.0");
    // By default, libcurl performs peer SSL/TLS certificate verification (Rule #1)
    // and hostname verification (Rule #2). We do not disable these features.

    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk->memory);
        chunk->memory = NULL;
        chunk->size = 0;
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        return 1;
    }

    long http_code = 0;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code < 200 || http_code >= 300) {
        fprintf(stderr, "HTTP request failed for %s with status code: %ld\n", url, http_code);
        free(chunk->memory);
        chunk->memory = NULL;
        chunk->size = 0;
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        return 1;
    }

    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();
    return 0; // Success
}

// Parses the <title> tag from HTML content.
// Note: This is a very basic parser.
// The caller is responsible for freeing the returned string.
char* parse_title(const char* html) {
    const char* title_start_tag = "<title>";
    const char* title_end_tag = "</title>";
    const char* title_start = strcasestr(html, title_start_tag);
    
    if (title_start == NULL) {
        char* no_title = (char*)malloc(15);
        if (no_title) snprintf(no_title, 15, "No title found");
        return no_title;
    }
    title_start += strlen(title_start_tag);

    const char* title_end = strcasestr(title_start, title_end_tag);

    if (title_end == NULL) {
        char* no_end_tag = (char*)malloc(30);
        if(no_end_tag) snprintf(no_end_tag, 30, "No closing title tag found");
        return no_end_tag;
    }
    
    size_t title_len = title_end - title_start;
    char* title = (char*)malloc(title_len + 1);
    if (title == NULL) {
        fprintf(stderr, "Error: malloc failed for title string.\n");
        return NULL;
    }

    memcpy(title, title_start, title_len);
    title[title_len] = '\0';
    return title;
}

// Initializes the database and creates the necessary table.
int init_db(sqlite3* db) {
    const char* sql = "CREATE TABLE IF NOT EXISTS scraped_data ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "url TEXT NOT NULL, "
                      "title TEXT, "
                      "timestamp TEXT NOT NULL);";
    char* zErrMsg = NULL;
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return 1;
    }
    return 0;
}

// Stores the scraped data into the database using prepared statements.
int store_in_db(sqlite3* db, const char* url, const char* title) {
    time_t now = time(NULL);
    struct tm t_local;
#ifdef _WIN32
    localtime_s(&t_local, &now);
#else
    localtime_r(&now, &t_local);
#endif
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &t_local);

    const char* sql = "INSERT INTO scraped_data (url, title, timestamp) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    int rc;

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Bind parameters to prevent SQL injection (Rule #3).
    sqlite3_bind_text(stmt, 1, url, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, timestamp, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 1;
    }
    
    sqlite3_finalize(stmt);
    return 0;
}

int main(void) {
    const char* db_file = "scraped_data_c.db";
    sqlite3* db = NULL;
    
    int rc = sqlite3_open(db_file, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        if (db) sqlite3_close(db);
        return 1;
    }

    if (init_db(db) != 0) {
        sqlite3_close(db);
        return 1;
    }
    
    const char* test_urls[] = {
        "https://www.example.com",
        "https://www.google.com",
        "https://www.github.com",
        "https://www.iana.org/domains/reserved",
        "https://www.w3.org"
    };
    int num_urls = sizeof(test_urls) / sizeof(test_urls[0]);
    
    for (int i = 0; i < num_urls; ++i) {
        const char* url = test_urls[i];
        printf("Scraping %s...\n", url);
        
        MemoryStruct chunk = {NULL, 0};
        
        if (scrape_website(url, &chunk) == 0) {
            char* title = parse_title(chunk.memory);
            if (title) {
                if (store_in_db(db, url, title) == 0) {
                    printf("  -> Stored title: %s\n", title);
                } else {
                    fprintf(stderr, "  -> Error storing data for %s\n", url);
                }
                free(title);
                title = NULL;
            } else {
                fprintf(stderr, "  -> Failed to parse title for %s\n", url);
            }
            free(chunk.memory);
            chunk.memory = NULL;
        } else {
            fprintf(stderr, "  -> Error scraping %s\n", url);
        }
    }

    sqlite3_close(db);
    printf("\nScraping complete. Data saved to %s\n", db_file);
    return 0;
}