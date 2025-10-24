
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include <ctype.h>

#define MAX_URL_LENGTH 2048
#define MAX_RESPONSE_SIZE (10 * 1024 * 1024)
#define MAX_CONTENT_LENGTH 50000
#define INITIAL_BUFFER_SIZE 4096

/* Structure to hold response data with bounds checking */
typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} ResponseBuffer;

/* Initialize response buffer with bounds checking (CWE-119, CWE-131) */
static int init_response_buffer(ResponseBuffer *buffer) {
    if (buffer == NULL) {
        return 0;
    }
    
    buffer->data = malloc(INITIAL_BUFFER_SIZE);
    if (buffer->data == NULL) {
        return 0;
    }
    
    buffer->data[0] = '\\0';
    buffer->size = 0;
    buffer->capacity = INITIAL_BUFFER_SIZE;
    
    return 1;
}

/* Free response buffer and clear sensitive data (CWE-404, CWE-459) */
static void free_response_buffer(ResponseBuffer *buffer) {
    if (buffer == NULL || buffer->data == NULL) {
        return;
    }
    
    /* Clear memory before freeing */
    memset_s(buffer->data, buffer->capacity, 0, buffer->capacity);
    free(buffer->data);
    buffer->data = NULL;
    buffer->size = 0;
    buffer->capacity = 0;
}

/* Callback for libcurl to write response data with bounds checking (CWE-120, CWE-787) */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize;
    ResponseBuffer *buffer;
    char *new_data;
    size_t new_capacity;
    
    if (contents == NULL || userp == NULL) {
        return 0;
    }
    
    /* Check for integer overflow (CWE-190) */
    if (size > SIZE_MAX / nmemb) {
        return 0;
    }
    
    realsize = size * nmemb;
    buffer = (ResponseBuffer *)userp;
    
    /* Prevent excessive memory usage (CWE-400) */
    if (buffer->size + realsize > MAX_RESPONSE_SIZE) {
        fprintf(stderr, "Warning: Response size limit reached\\n");
        return 0;
    }
    
    /* Check if buffer needs to grow (CWE-131) */
    if (buffer->size + realsize + 1 > buffer->capacity) {
        /* Calculate new capacity with overflow check (CWE-190) */
        new_capacity = buffer->capacity * 2;
        if (new_capacity < buffer->capacity || 
            new_capacity > MAX_RESPONSE_SIZE) {
            new_capacity = MAX_RESPONSE_SIZE;
        }
        
        if (buffer->size + realsize + 1 > new_capacity) {
            return 0;
        }
        
        /* Reallocate with bounds checking (CWE-122) */
        new_data = realloc(buffer->data, new_capacity);
        if (new_data == NULL) {
            return 0;
        }
        
        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }
    
    /* Copy data with bounds checking (CWE-120, CWE-787) */
    memcpy(&(buffer->data[buffer->size]), contents, realsize);
    buffer->size += realsize;
    buffer->data[buffer->size] = '\\0';
    
    return realsize;
}

/* Validate URL format and ensure HTTPS only (CWE-20, CWE-319) */
static int validate_url(const char *url) {
    size_t len;
    size_t i;
    
    if (url == NULL) {
        return 0;
    }
    
    /* Check length to prevent DoS (CWE-400) */
    len = strnlen(url, MAX_URL_LENGTH + 1);
    if (len == 0 || len > MAX_URL_LENGTH) {
        return 0;
    }
    
    /* Must start with https:// (CWE-319) */
    if (strncmp(url, "https://", 8) != 0) {
        return 0;
    }
    
    /* Basic format validation - check for valid characters */
    for (i = 8; i < len; i++) {
        unsigned char c = (unsigned char)url[i];
        if (!isalnum(c) && c != '-' && c != '.' && c != '/' && 
            c != ':' && c != '?' && c != '&' && c != '=') {
            return 0;
        }
    }
    
    return 1;
}

/* Scrape website using secure HTTPS connection (CWE-295, CWE-297, CWE-327) */
static char* scrape_website(const char *url) {
    CURL *curl;
    CURLcode res;
    ResponseBuffer buffer;
    char *result;
    
    /* Validate URL (CWE-20) */
    if (!validate_url(url)) {
        fprintf(stderr, "Error: Invalid URL. Must be HTTPS and properly formatted.\\n");
        return NULL;
    }
    
    /* Initialize response buffer (CWE-131) */
    if (!init_response_buffer(&buffer)) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    /* Initialize libcurl */
    curl = curl_easy_init();
    if (curl == NULL) {
        fprintf(stderr, "Error: Failed to initialize libcurl\\n");
        free_response_buffer(&buffer);
        return NULL;
    }
    
    /* Configure secure HTTPS connection (CWE-295, CWE-297, CWE-327) */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&buffer);
    
    /* Enforce certificate validation (CWE-295) */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    
    /* Enforce hostname verification (CWE-297) */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    /* Enforce TLS 1.2 or higher (CWE-327) */
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, 
                     CURL_SSLVERSION_TLSv1_2 | CURL_SSLVERSION_MAX_TLSv1_3);
    
    /* Set timeout to prevent hanging (CWE-400) */
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    
    /* Set user agent */
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureScraper/1.0");
    
    /* Perform request */
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        fprintf(stderr, "Error: curl_easy_perform() failed: %s\\n", 
                curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free_response_buffer(&buffer);
        return NULL;
    }
    
    /* Check HTTP response code */
    {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
            fprintf(stderr, "Error: HTTP %ld\\n", http_code);
            curl_easy_cleanup(curl);
            free_response_buffer(&buffer);
            return NULL;
        }
    }
    
    /* Cleanup curl */
    curl_easy_cleanup(curl);
    
    /* Return allocated string (caller must free) */
    result = buffer.data;
    buffer.data = NULL;
    
    return result;
}

/* Validate database path to prevent path traversal (CWE-22) */
static int validate_db_path(const char *db_path) {
    size_t len;
    size_t i;
    
    if (db_path == NULL) {
        return 0;
    }
    
    /* Check length */
    len = strnlen(db_path, 256);
    if (len == 0 || len > 255) {
        return 0;
    }
    
    /* Check for path traversal patterns (CWE-22) */
    if (strstr(db_path, "..") != NULL || 
        strstr(db_path, "//") != NULL ||
        db_path[0] == '/') {
        return 0;
    }
    
    /* Check for valid characters */
    for (i = 0; i < len; i++) {
        unsigned char c = (unsigned char)db_path[i];
        if (!isalnum(c) && c != '_' && c != '-' && c != '.') {
            return 0;
        }
    }
    
    return 1;
}

/* Store data in SQLite database using parameterized queries (CWE-89) */
static int store_in_database(const char *data, const char *db_path, 
                             const char *source_url) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char *line_start;
    char *line_end;
    char line_buffer[MAX_CONTENT_LENGTH + 1];
    
    /* Validate database path (CWE-22) */
    if (!validate_db_path(db_path)) {
        fprintf(stderr, "Error: Invalid database path\\n");
        return 0;
    }
    
    /* Validate inputs (CWE-20) */
    if (data == NULL || source_url == NULL) {
        fprintf(stderr, "Error: Invalid input data\\n");
        return 0;
    }
    
    if (strnlen(source_url, MAX_URL_LENGTH + 1) > MAX_URL_LENGTH) {
        fprintf(stderr, "Error: Source URL too long\\n");
        return 0;
    }
    
    /* Open database connection (CWE-404) */
    rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error: Cannot open database: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    /* Create table with proper schema */
    rc = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS scraped_data ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "source_url TEXT NOT NULL, "
        "content TEXT NOT NULL, "
        "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)",
        NULL, NULL, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error: Failed to create table: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    /* Create index */
    sqlite3_exec(db,
        "CREATE INDEX IF NOT EXISTS idx_source_url ON scraped_data(source_url)",
        NULL, NULL, NULL);
    
    /* Begin transaction */
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    
    /* Prepare parameterized query (CWE-89) */
    rc = sqlite3_prepare_v2(db,
        "INSERT INTO scraped_data (source_url, content) VALUES (?, ?)",
        -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error: Failed to prepare statement: %s\\n", 
                sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    /* Parse lines and insert with bounds checking (CWE-120) */
    line_start = (char *)data;
    while (*line_start != '\\0') {
        size_t line_len;
        
        /* Find end of line */
        line_end = strchr(line_start, '\\n');
        if (line_end == NULL) {
            line_end = line_start + strlen(line_start);
        }
        
        /* Calculate line length with bounds check (CWE-131) */
        line_len = line_end - line_start;
        if (line_len > 0 && line_len < MAX_CONTENT_LENGTH) {
            /* Copy line with bounds checking (CWE-120, CWE-787) */
            memcpy(line_buffer, line_start, line_len);
            line_buffer[line_len] = '\\0';
            
            /* Trim whitespace */
            {
                char *trimmed = line_buffer;
                while (*trimmed != '\\0' && isspace((unsigned char)*trimmed)) {
                    trimmed++;
                }
                
                if (*trimmed != '\\0') {
                    /* Bind parameters using parameterized query (CWE-89) */
                    sqlite3_bind_text(stmt, 1, source_url, -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, trimmed, -1, SQLITE_TRANSIENT);
                    
                    /* Execute statement (CWE-755) */
                    rc = sqlite3_step(stmt);
                    if (rc != SQLITE_DONE) {
                        fprintf(stderr, "Error: Failed to insert data\\n");
                    }
                    
                    /* Reset statement for next iteration */
                    sqlite3_reset(stmt);
                }
            }
        }
        
        /* Move to next line */
        line_start = (*line_end == '\\0') ? line_end : line_end + 1;
    }
    
    /* Finalize statement (CWE-404) */
    sqlite3_finalize(stmt);
    
    /* Commit transaction */
    rc = sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error: Failed to commit transaction\\n");
        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        sqlite3_close(db);
        return 0;
    }
    
    printf("Successfully stored data in database\\n");
    
    /* Close database (CWE-404) */
    sqlite3_close(db);
    
    return 1;
}

/* Main function with test cases */
int main(void) {
    char *data;
    int result;
    
    printf("=== Secure Web Scraper Test Cases ===\\n\\n");
    
    /* Initialize libcurl globally */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    /* Test case 1: Valid HTTPS URL */
    printf("Test 1: Scraping from example.com...\\n");
    data = scrape_website("https://example.com");
    if (data != NULL) {
        result = store_in_database(data, "test_data1.db", "https://example.com");
        printf("Result: %s\\n\\n", result ? "Success" : "Failed");
        memset_s(data, strlen(data), 0, strlen(data));
        free(data);
    } else {
        printf("Result: Failed to scrape\\n\\n");
    }
    
    /* Test case 2: Invalid URL - HTTP instead of HTTPS (should fail) */
    printf("Test 2: Testing HTTP URL (should reject)...\\n");
    data = scrape_website("http://example.com");
    printf("Result: %s\\n\\n", data == NULL ? "Rejected" : "Failed - should reject HTTP");
    if (data != NULL) {
        memset_s(data, strlen(data), 0, strlen(data));
        free(data);
    }
    
    /* Test case 3: Invalid URL - malformed (should fail) */
    printf("Test 3: Testing malformed URL (should reject)...\\n");
    data = scrape_website("not-a-valid-url");
    printf("Result: %s\\n\\n", data == NULL ? "Rejected" : "Failed - should reject malformed");
    if (data != NULL) {
        memset_s(data, strlen(data), 0, strlen(data));
        free(data);
    }
    
    /* Test case 4: Path traversal attempt in database path (should fail) */
    printf("Test 4: Testing path traversal in DB path (should reject)...\\n");
    result = store_in_database("test data", "../etc/passwd", "https://example.com");
    printf("Result: %s\\n\\n", !result ? "Rejected" : "Failed - should reject traversal");
    
    /* Test case 5: Valid scrape and store */
    printf("Test 5: Complete scrape and store operation...\\n");
    data = scrape_website("https://www.ietf.org");
    if (data != NULL) {
        result = store_in_database(data, "test_data5.db", "https://www.ietf.org");
        printf("Result: %s\\n\\n", result ? "Success" : "Failed");
        memset_s(data, strlen(data), 0, strlen(data));
        free(data);
    } else {
        printf("Result: Failed to scrape\\n\\n");
    }
    
    printf("=== Test Cases Complete ===\\n");
    
    /* Cleanup libcurl */
    curl_global_cleanup();
    
    return 0;
}
