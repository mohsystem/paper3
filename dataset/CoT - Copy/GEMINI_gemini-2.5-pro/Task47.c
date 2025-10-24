// Dependencies: libcurl, libsqlite3
// How to compile and run on Linux/macOS:
// gcc this_file.c -o Task47 -lcurl -lsqlite3
// ./Task47
// On Windows, you will need to link against the respective libraries.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <sqlite3.h>

const char* DB_PATH = "scraping_results_c.db";

struct MemoryStruct {
    char* memory;
    size_t size;
};

// Callback function for libcurl to write received data into our MemoryStruct
static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = realloc(mem->memory, mem->size + realsize + 1);
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

// Function to trim leading/trailing whitespace from a string
char* trim_whitespace(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}


// Simple and non-robust function to extract content between <title> tags
char* extract_title(const char* html) {
    const char* start_tag = "<title>";
    const char* end_tag = "</title>";
    char* title = NULL;

    const char* start_ptr = strstr(html, start_tag);
    if (start_ptr) {
        start_ptr += strlen(start_tag);
        const char* end_ptr = strstr(start_ptr, end_tag);
        if (end_ptr) {
            size_t length = end_ptr - start_ptr;
            title = (char*)malloc(length + 1);
            if (title) {
                strncpy(title, start_ptr, length);
                title[length] = '\0';
                // Note: trim_whitespace modifies the string in-place but also returns a pointer
                // to the start of the trimmed content, which might be different from the original pointer.
                // However, the original malloc'd block must be freed. So we copy to a new buffer after trimming.
                char* trimmed_start = trim_whitespace(title);
                char* final_title = strdup(trimmed_start);
                free(title);
                return final_title;
            }
        }
    }
    return NULL;
}

void setup_database() {
    sqlite3* db;
    char* err_msg = 0;
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS pages("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "url TEXT NOT NULL UNIQUE, title TEXT NOT NULL, "
                      "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    sqlite3_close(db);
}

void scrape_and_store(const char* url) {
    printf("Scraping: %s\n", url);
    CURL* curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "C-Scraper-Bot/1.0");
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

        res = curl_easy_perform(curl_handle);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            char* title = extract_title(chunk.memory);
            if (title) {
                sqlite3* db;
                sqlite3_stmt* stmt;
                if (sqlite3_open(DB_PATH, &db) == SQLITE_OK) {
                    const char* sql = "INSERT OR IGNORE INTO pages(url, title) VALUES(?, ?);";
                    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
                        sqlite3_bind_text(stmt, 1, url, -1, SQLITE_STATIC);
                        sqlite3_bind_text(stmt, 2, title, -1, SQLITE_STATIC);
                        if (sqlite3_step(stmt) != SQLITE_DONE) {
                            fprintf(stderr, "Insertion failed: %s\n", sqlite3_errmsg(db));
                        } else {
                             printf("Successfully stored title: \"%s\"\n", title);
                        }
                    } else {
                        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
                    }
                    sqlite3_finalize(stmt);
                } else {
                    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
                }
                sqlite3_close(db);
                free(title);
            } else {
                fprintf(stderr, "Could not find title for URL: %s\n", url);
            }
        }
        curl_easy_cleanup(curl_handle);
    }
    free(chunk.memory);
    curl_global_cleanup();
}


static int print_callback(void* NotUsed, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        printf("%s\t", argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

void print_database_contents() {
    printf("\n--- Database Contents ---\n");
    sqlite3* db;
    char* err_msg = 0;
    int rc = sqlite3_open(DB_PATH, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char* sql = "SELECT * FROM pages";
    rc = sqlite3_exec(db, sql, print_callback, 0, &err_msg);

    if (rc != SQLITE_OK ) {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
    printf("-------------------------\n\n");
}


int main(void) {
    setup_database();

    const char* test_urls[] = {
        "http://books.toscrape.com/",
        "http://books.toscrape.com/catalogue/category/books/travel_2/index.html",
        "http://books.toscrape.com/catalogue/a-light-in-the-attic_1000/index.html",
        "http://quotes.toscrape.com/",
        "http://quotes.toscrape.com/tag/humor/"
    };
    int num_urls = sizeof(test_urls) / sizeof(test_urls[0]);

    for (int i = 0; i < num_urls; i++) {
        scrape_and_store(test_urls[i]);
    }
    
    print_database_contents();

    return 0;
}