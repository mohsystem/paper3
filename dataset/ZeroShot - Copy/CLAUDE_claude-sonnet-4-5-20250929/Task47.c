
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include <regex.h>
#include <ctype.h>

typedef struct {
    char* data;
    size_t size;
} MemoryStruct;

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    MemoryStruct* mem = (MemoryStruct*)userp;
    
    char* ptr = realloc(mem->data, mem->size + realsize + 1);
    if (ptr == NULL) {
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

int is_valid_url(const char* url) {
    if (strncmp(url, "http://", 7) == 0 || strncmp(url, "https://", 8) == 0) {
        return 1;
    }
    return 0;
}

void sanitize_table_name(const char* input, char* output) {
    int j = 0;
    for (int i = 0; input[i] != '\\0'; i++) {
        if (isalnum(input[i]) || input[i] == '_') {
            output[j++] = input[i];
        } else {
            output[j++] = '_';
        }
    }
    output[j] = '\\0';
}

void initialize_database(const char* db_name, const char* table_name) {
    sqlite3* db;
    char* err_msg = NULL;
    char sanitized[256];
    char sql[512];
    
    sanitize_table_name(table_name, sanitized);
    
    if (sqlite3_open(db_name, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n");
        return;
    }
    
    snprintf(sql, sizeof(sql),
             "CREATE TABLE IF NOT EXISTS %s ("
             "id INTEGER PRIMARY KEY AUTOINCREMENT, "
             "url TEXT NOT NULL, "
             "content TEXT, "
             "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)", sanitized);
    
    sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    sqlite3_close(db);
}

char* scrape_website(const char* url) {
    CURL* curl;
    CURLcode res;
    MemoryStruct chunk = {NULL, 0};
    
    chunk.data = malloc(1);
    chunk.size = 0;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            free(chunk.data);
            return NULL;
        }
    }
    
    return chunk.data;
}

void store_data(const char* db_name, const char* table_name, 
                const char* url, const char* content) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    char sanitized[256];
    char sql[512];
    
    sanitize_table_name(table_name, sanitized);
    
    if (sqlite3_open(db_name, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n");
        return;
    }
    
    snprintf(sql, sizeof(sql), "INSERT INTO %s (url, content) VALUES (?, ?)", sanitized);
    
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, url, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, content, -1, SQLITE_TRANSIENT);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void scrape_and_store(const char* url, const char* table_name) {
    if (!is_valid_url(url)) {
        printf("Invalid URL provided\\n");
        return;
    }
    
    const char* db_name = "scraped_data.db";
    
    initialize_database(db_name, table_name);
    
    char* content = scrape_website(url);
    if (content == NULL) {
        printf("Error: Failed to scrape website\\n");
        return;
    }
    
    store_data(db_name, table_name, url, content);
    free(content);
    
    printf("Data scraped and stored successfully\\n");
}

int main() {
    printf("=== Web Scraper Test Cases ===\\n\\n");
    
    printf("Test 1: Scraping example.com\\n");
    scrape_and_store("http://example.com", "test_data");
    
    printf("\\nTest 2: Invalid URL\\n");
    scrape_and_store("invalid-url", "test_data");
    
    printf("\\nTest 3: HTTPS URL\\n");
    scrape_and_store("https://example.com", "secure_data");
    
    printf("\\nTest 4: Different table\\n");
    scrape_and_store("http://example.com", "website_content");
    
    printf("\\nTest 5: Table name sanitization\\n");
    scrape_and_store("http://example.com", "data-table-2024");
    
    return 0;
}
