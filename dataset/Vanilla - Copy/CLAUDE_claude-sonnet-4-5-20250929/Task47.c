
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <curl/curl.h>

#define DB_NAME "scraped_data.db"
#define MAX_CONTENT_SIZE 100000

typedef struct {
    char* data;
    size_t size;
} MemoryStruct;

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    MemoryStruct* mem = (MemoryStruct*)userp;
    
    char* ptr = realloc(mem->data, mem->size + realsize + 1);
    if(ptr == NULL) {
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

void create_database() {
    sqlite3* db;
    char* err_msg = 0;
    
    int rc = sqlite3_open(DB_NAME, &db);
    
    const char* sql = "CREATE TABLE IF NOT EXISTS scraped_data ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "url TEXT, "
                     "title TEXT, "
                     "content TEXT, "
                     "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
    
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    sqlite3_close(db);
}

char* scrape_website(const char* url) {
    CURL* curl;
    CURLcode res;
    MemoryStruct chunk;
    
    chunk.data = malloc(1);
    chunk.size = 0;
    
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    
    return chunk.data;
}

void scrape_and_store(const char* url) {
    create_database();
    
    char* content = scrape_website(url);
    
    if(content != NULL) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        sqlite3_open(DB_NAME, &db);
        
        const char* sql = "INSERT INTO scraped_data (url, title, content) VALUES (?, ?, ?);";
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        
        char truncated[501];
        strncpy(truncated, content, 500);
        truncated[500] = '\\0';
        
        sqlite3_bind_text(stmt, 1, url, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, "Scraped Title", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, truncated, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        printf("Data scraped and stored successfully from: %s\\n", url);
        
        free(content);
    } else {
        printf("Error: Could not scrape %s\\n", url);
    }
}

int main() {
    printf("Web Scraper Test Cases:\\n");
    printf("========================\\n\\n");
    
    // Test case 1
    printf("Test Case 1: Scraping example.com\\n");
    scrape_and_store("http://example.com");
    
    // Test case 2
    printf("\\nTest Case 2: Scraping example.org\\n");
    scrape_and_store("http://example.org");
    
    // Test case 3
    printf("\\nTest Case 3: Scraping example.net\\n");
    scrape_and_store("http://example.net");
    
    // Test case 4
    printf("\\nTest Case 4: Invalid URL test\\n");
    scrape_and_store("http://invalid-url-that-does-not-exist-12345.com");
    
    // Test case 5
    printf("\\nTest Case 5: Another valid URL\\n");
    scrape_and_store("http://info.cern.ch");
    
    return 0;
}
