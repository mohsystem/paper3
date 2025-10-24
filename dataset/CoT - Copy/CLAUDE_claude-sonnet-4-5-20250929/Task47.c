
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <curl/curl.h>
#include <ctype.h>

#define DB_NAME "scraped_data.db"
#define TIMEOUT 5
#define MAX_CONTENT_LENGTH 1000000
#define MAX_URL_LENGTH 2048
#define MAX_TITLE_LENGTH 512

typedef struct {
    char* data;
    size_t size;
} MemoryStruct;

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    MemoryStruct* mem = (MemoryStruct*)userp;
    
    if (mem->size + realsize > MAX_CONTENT_LENGTH) {
        return 0;
    }
    
    char* ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

int init_database() {
    sqlite3* db;
    char* err_msg = NULL;
    int rc;
    
    rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS scraped_data ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "url TEXT NOT NULL,"
        "title TEXT,"
        "content TEXT,"
        "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_close(db);
    return 1;
}

void sanitize_string(char* input, char* output, size_t max_len) {
    if (!input || !output) return;
    
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < max_len - 1; i++) {
        char c = input[i];
        if (c != '<' && c != '>' && c != '"' && c != '\\'' && 
            c != '%' && c != ';' && c != '(' && c != ')' && 
            c != '&' && c != '+') {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
}

char* scrape_website(const char* url) {
    if (!url || strlen(url) == 0) {
        fprintf(stderr, "URL cannot be empty\\n");
        return NULL;
    }
    
    if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0) {
        fprintf(stderr, "Invalid URL format\\n");
        return NULL;
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL\\n");
        return NULL;
    }
    
    MemoryStruct chunk = {NULL, 0};
    chunk.data = malloc(1);
    chunk.data[0] = '\\0';
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Secure Scraper)");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\\n", curl_easy_strerror(res));
        free(chunk.data);
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    long response_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_cleanup(curl);
    
    if (response_code != 200) {
        fprintf(stderr, "HTTP Error: %ld\\n", response_code);
        free(chunk.data);
        return NULL;
    }
    
    return chunk.data;
}

void extract_title(const char* html, char* title, size_t max_len) {
    if (!html || !title) return;
    
    const char* start = strcasestr(html, "<title>");
    if (!start) {
        strncpy(title, "No title found", max_len - 1);
        title[max_len - 1] = '\\0';
        return;
    }
    
    start += 7;
    const char* end = strcasestr(start, "</title>");
    
    if (!end) {
        strncpy(title, "No title found", max_len - 1);
        title[max_len - 1] = '\\0';
        return;
    }
    
    size_t len = end - start;
    if (len >= max_len) len = max_len - 1;
    
    strncpy(title, start, len);
    title[len] = '\\0';
}

int store_in_database(const char* url, const char* title, const char* content) {
    if (!url || strlen(url) == 0) {
        fprintf(stderr, "URL cannot be empty\\n");
        return 0;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;
    
    rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    const char* sql = "INSERT INTO scraped_data (url, title, content) VALUES (?, ?, ?);";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    char sanitized_url[MAX_URL_LENGTH];
    char sanitized_title[MAX_TITLE_LENGTH];
    
    sanitize_string((char*)url, sanitized_url, MAX_URL_LENGTH);
    sanitize_string((char*)title, sanitized_title, MAX_TITLE_LENGTH);
    
    sqlite3_bind_text(stmt, 1, sanitized_url, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, sanitized_title, -1, SQLITE_TRANSIENT);
    
    if (content) {
        size_t content_len = strlen(content);
        if (content_len > 10000) content_len = 10000;
        sqlite3_bind_text(stmt, 3, content, content_len, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_text(stmt, 3, "", -1, SQLITE_TRANSIENT);
    }
    
    rc = sqlite3_step(stmt);
    int success = (rc == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return success;
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    if (init_database()) {
        printf("Database initialized successfully\\n");
    } else {
        fprintf(stderr, "Failed to initialize database\\n");
        curl_global_cleanup();
        return 1;
    }
    
    // Test case 1
    printf("\\nTest 1: Storing test data\\n");
    if (store_in_database("https://example.com/test1", "Test Page 1", "<html><body>Test content 1</body></html>")) {
        printf("Test 1 passed\\n");
    } else {
        printf("Test 1 failed\\n");
    }
    
    // Test case 2
    printf("\\nTest 2: Storing another test data\\n");
    if (store_in_database("https://example.com/test2", "Test Page 2", "<html><body>Test content 2</body></html>")) {
        printf("Test 2 passed\\n");
    } else {
        printf("Test 2 failed\\n");
    }
    
    // Test case 3
    printf("\\nTest 3: Extracting title\\n");
    char title[MAX_TITLE_LENGTH];
    const char* html = "<html><head><title>Sample Title</title></head><body>Content</body></html>";
    extract_title(html, title, MAX_TITLE_LENGTH);
    printf("Extracted title: %s\\n", title);
    printf("Test 3 passed\\n");
    
    // Test case 4
    printf("\\nTest 4: String sanitization\\n");
    char unsafe[] = "<script>alert('xss')</script>";
    char safe[MAX_TITLE_LENGTH];
    sanitize_string(unsafe, safe, MAX_TITLE_LENGTH);
    printf("Sanitized string: %s\\n", safe);
    printf("Test 4 passed\\n");
    
    // Test case 5
    printf("\\nTest 5: Invalid URL handling\\n");
    char* result = scrape_website("invalid-url");
    if (!result) {
        printf("Test 5 passed - invalid URL rejected\\n");
    } else {
        printf("Test 5 failed\\n");
        free(result);
    }
    
    printf("\\nAll tests completed!\\n");
    
    curl_global_cleanup();
    return 0;
}
