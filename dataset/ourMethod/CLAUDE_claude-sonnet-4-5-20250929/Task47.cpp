
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <algorithm>
#include <curl/curl.h>
#include <sqlite3.h>

constexpr size_t MAX_URL_LENGTH = 2048;
constexpr size_t MAX_RESPONSE_SIZE = 10 * 1024 * 1024;
constexpr size_t MAX_CONTENT_LENGTH = 50000;

// RAII wrapper for CURL handle
class CurlHandle {
private:
    CURL* handle;
    
public:
    CurlHandle() : handle(curl_easy_init()) {}
    
    ~CurlHandle() {
        if (handle) {
            curl_easy_cleanup(handle);
        }
    }
    
    // Delete copy operations
    CurlHandle(const CurlHandle&) = delete;
    CurlHandle& operator=(const CurlHandle&) = delete;
    
    CURL* get() const { return handle; }
    bool isValid() const { return handle != nullptr; }
};

// RAII wrapper for SQLite database
class DatabaseHandle {
private:
    sqlite3* db;
    
public:
    DatabaseHandle() : db(nullptr) {}
    
    ~DatabaseHandle() {
        if (db) {
            sqlite3_close(db);
        }
    }
    
    // Delete copy operations
    DatabaseHandle(const DatabaseHandle&) = delete;
    DatabaseHandle& operator=(const DatabaseHandle&) = delete;
    
    bool open(const std::string& path) {
        int rc = sqlite3_open(path.c_str(), &db);
        return rc == SQLITE_OK;
    }
    
    sqlite3* get() const { return db; }
    bool isValid() const { return db != nullptr; }
};

// Structure to hold response data with automatic memory management
struct ResponseBuffer {
    std::vector<char> data;
    
    // Reserve initial capacity to avoid frequent reallocations
    ResponseBuffer() {
        data.reserve(4096);
    }
};

// Callback for libcurl to write response data with bounds checking (CWE-120, CWE-787)
static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    if (!contents || !userp) {
        return 0;
    }
    
    // Check for integer overflow (CWE-190)
    if (size > SIZE_MAX / nmemb) {
        return 0;
    }
    
    size_t realsize = size * nmemb;
    auto* buffer = static_cast<ResponseBuffer*>(userp);
    
    // Prevent excessive memory usage (CWE-400)
    if (buffer->data.size() + realsize > MAX_RESPONSE_SIZE) {
        std::cerr << "Warning: Response size limit reached" << std::endl;
        return 0;
    }
    
    // Append data with automatic bounds checking via std::vector
    try {
        const char* input = static_cast<const char*>(contents);
        buffer->data.insert(buffer->data.end(), input, input + realsize);
    } catch (const std::bad_alloc&) {
        std::cerr << "Error: Memory allocation failed" << std::endl;
        return 0;
    }
    
    return realsize;
}

// Validate URL format and ensure HTTPS only (CWE-20, CWE-319)
bool validateUrl(const std::string& url) {
    // Check length to prevent DoS (CWE-400)
    if (url.empty() || url.length() > MAX_URL_LENGTH) {
        return false;
    }
    
    // Must start with https:// (CWE-319)
    if (url.substr(0, 8) != "https://") {
        return false;
    }
    
    // Validate format using regex
    static const std::regex urlPattern(
        R"(^https://(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\.)*)"
        R"([a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)"
        R"((?::[0-9]{1,5})?(?:/[^\\s]*)?$)"
    );
    
    return std::regex_match(url, urlPattern);
}

// Scrape website using secure HTTPS connection (CWE-295, CWE-297, CWE-327)
std::unique_ptr<std::string> scrapeWebsite(const std::string& url) {
    // Validate URL (CWE-20)
    if (!validateUrl(url)) {
        std::cerr << "Error: Invalid URL. Must be HTTPS and properly formatted." << std::endl;
        return nullptr;
    }
    
    // Initialize CURL handle with RAII
    CurlHandle curl;
    if (!curl.isValid()) {
        std::cerr << "Error: Failed to initialize libcurl" << std::endl;
        return nullptr;
    }
    
    // Initialize response buffer
    ResponseBuffer buffer;
    
    // Configure secure HTTPS connection (CWE-295, CWE-297, CWE-327)
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &buffer);
    
    // Enforce certificate validation (CWE-295)
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
    
    // Enforce hostname verification (CWE-297)
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);
    
    // Enforce TLS 1.2 or higher (CWE-327)
    curl_easy_setopt(curl.get(), CURLOPT_SSLVERSION, 
                     CURL_SSLVERSION_TLSv1_2 | CURL_SSLVERSION_MAX_TLSv1_3);
    
    // Set timeout to prevent hanging (CWE-400)
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 30L);
    
    // Set user agent
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "SecureScraper/1.0");
    
    // Perform request
    CURLcode res = curl_easy_perform(curl.get());
    
    if (res != CURLE_OK) {
        std::cerr << "Error: curl_easy_perform() failed: " 
                  << curl_easy_strerror(res) << std::endl;
        return nullptr;
    }
    
    // Check HTTP response code
    long httpCode = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &httpCode);
    if (httpCode != 200) {
        std::cerr << "Error: HTTP " << httpCode << std::endl;
        return nullptr;
    }
    
    // Convert buffer to string
    buffer.data.push_back('\\0');
    return std::make_unique<std::string>(buffer.data.data());
}

// Validate database path to prevent path traversal (CWE-22)
bool validateDbPath(const std::string& dbPath) {
    // Check length
    if (dbPath.empty() || dbPath.length() > 255) {
        return false;
    }
    
    // Check for path traversal patterns (CWE-22)
    if (dbPath.find("..") != std::string::npos ||
        dbPath.find("//") != std::string::npos ||
        dbPath[0] == '/') {
        return false;
    }
    
    // Check for valid characters only
    return std::all_of(dbPath.begin(), dbPath.end(), [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || 
               c == '_' || c == '-' || c == '.';
    });
}

// Store data in SQLite database using parameterized queries (CWE-89)
bool storeInDatabase(const std::vector<std::string>& data, 
                     const std::string& dbPath,
                     const std::string& sourceUrl) {
    // Validate database path (CWE-22)
    if (!validateDbPath(dbPath)) {
        std::cerr << "Error: Invalid database path" << std::endl;
        return false;
    }
    
    // Validate inputs (CWE-20)
    if (data.empty()) {
        std::cerr << "Error: Invalid data" << std::endl;
        return false;
    }
    
    if (sourceUrl.length() > MAX_URL_LENGTH) {
        std::cerr << "Error: Source URL too long" << std::endl;
        return false;
    }
    
    // Open database with RAII
    DatabaseHandle db;
    if (!db.open(dbPath)) {
        std::cerr << "Error: Cannot open database" << std::endl;
        return false;
    }
    
    // Create table with proper schema
    const char* createTableSql =
        "CREATE TABLE IF NOT EXISTS scraped_data ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "source_url TEXT NOT NULL, "
        "content TEXT NOT NULL, "
        "scraped_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
    
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db.get(), createTableSql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Failed to create table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    // Create index
    sqlite3_exec(db.get(),
        "CREATE INDEX IF NOT EXISTS idx_source_url ON scraped_data(source_url)",
        nullptr, nullptr, nullptr);
    
    // Begin transaction
    sqlite3_exec(db.get(), "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
    
    // Prepare parameterized query (CWE-89)
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db.get(),
        "INSERT INTO scraped_data (source_url, content) VALUES (?, ?)",
        -1, &stmt, nullptr);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Failed to prepare statement" << std::endl;
        sqlite3_exec(db.get(), "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }
    
    // Insert data using parameterized queries
    for (const auto& item : data) {
        if (item.empty()) {
            continue;
        }
        
        // Limit content length
        std::string content = item;
        if (content.length() > MAX_CONTENT_LENGTH) {
            content = content.substr(0, MAX_CONTENT_LENGTH);
        }
        
        // Bind parameters (CWE-89)
        sqlite3_bind_text(stmt, 1, sourceUrl.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_TRANSIENT);
        
        // Execute statement (CWE-755)
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Error: Failed to insert data" << std::endl;
        }
        
        // Reset statement for next iteration
        sqlite3_reset(stmt);
    }
    
    // Finalize statement (RAII alternative)
    sqlite3_finalize(stmt);
    
    // Commit transaction
    rc = sqlite3_exec(db.get(), "COMMIT", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Failed to commit transaction" << std::endl;
        sqlite3_exec(db.get(), "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }
    
    std::cout << "Successfully stored " << data.size() << " items in database" << std::endl;
    return true;
}

// Parse HTML content into lines (simple text extraction)
std::vector<std::string> parseHtmlContent(const std::string& html) {
    std::vector<std::string> lines;
    
    // Simple parsing: remove tags and extract text
    std::string text = html;
    
    // Remove script and style tags with content
    text = std::regex_replace(text, std::regex("<script[^>]*>.*?</script>"), "");
    text = std::regex_replace(text, std::regex("<style[^>]*>.*?</style>"), "");
    
    // Remove HTML tags
    text = std::regex_replace(text, std::regex("<[^>]+>"), "");
    
    // Remove HTML entities
    text = std::regex_replace(text, std::regex("&[^;]+;"), " ");
    
    // Split into lines
    std::istringstream stream(text);
    std::string line;
    
    while (std::getline(stream, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \\t\\r\\n"));
        line.erase(line.find_last_not_of(" \\t\\r\\n") + 1);
        
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    
    return lines;
}

// Main function with test cases
int main() {
    std::cout << "=== Secure Web Scraper Test Cases ===" << std::endl << std::endl;
    
    // Initialize libcurl globally
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    // Test case 1: Valid HTTPS URL
    std::cout << "Test 1: Scraping from example.com..." << std::endl;
    auto data1 = scrapeWebsite("https://example.com");
    if (data1) {
        auto lines1 = parseHtmlContent(*data1);
        bool result1 = storeInDatabase(lines1, "test_data1.db", "https://example.com");
        std::cout << "Result: " << (result1 ? "Success" : "Failed") << std::endl << std::endl;
    } else {
        std::cout << "Result: Failed to scrape" << std::endl << std::endl;
    }
    
    // Test case 2: Invalid URL - HTTP instead of HTTPS (should fail)
    std::cout << "Test 2: Testing HTTP URL (should reject)..." << std::endl;
    auto data2 = scrapeWebsite("http://example.com");
    std::cout << "Result: " << (data2 == nullptr ? "Rejected" : "Failed - should reject HTTP") 
              << std::endl << std::endl;
    
    // Test case 3: Invalid URL - malformed (should fail)
    std::cout << "Test 3: Testing malformed URL (should reject)..." << std::endl;
    auto data3 = scrapeWebsite("not-a-valid-url");
    std::cout << "Result: " << (data3 == nullptr ? "Rejected" : "Failed - should reject malformed") 
              << std::endl << std::endl;
    
    // Test case 4: Path traversal attempt in database path (should fail)
    std::cout << "Test 4: Testing path traversal in DB path (should reject)..." << std::endl;
    std::vector<std::string> data4 = {"test data"};
    bool result4 = storeInDatabase(data4, "../etc/passwd", "https://example.com");
    std::cout << "Result: " << (!result4 ? "Rejected" : "Failed - should reject traversal") 
              << std::endl << std::endl;
    
    // Test case 5: Valid scrape and store
    std::cout << "Test 5: Complete scrape and store operation..." << std::endl;
    auto data5 = scrapeWebsite("https://www.ietf.org");
    if (data5) {
        auto lines5 = parseHtmlContent(*data5);
        bool result5 = storeInDatabase(lines5, "', type='text')