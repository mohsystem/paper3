// Chain-of-Through secure implementation
// 1) Problem: Fetch HTML from URL, extract anchors, store into SQLite. Input: url, dbPath. Output: count inserted.
// 2) Security: allow http/https only, timeouts, max size 2MB, parameterized SQL, safe UA, error checks, follow redirects limited.
// 3) Implementation: libcurl for HTTP with capped write callback, sqlite3 with prepared statements, std::regex for anchors.
// 4) Review: validated scheme, capped size, prepared statements, no unsafe casts, handle errors.
// 5) Output: final code below.

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <ctime>
#include <curl/curl.h>
#include <sqlite3.h>

struct Buffer {
    std::string data;
    size_t maxBytes;
};

static size_t write_capped(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    Buffer* buf = static_cast<Buffer*>(userp);
    if (buf->data.size() >= buf->maxBytes) {
        return 0; // stop
    }
    size_t space = buf->maxBytes - buf->data.size();
    size_t toCopy = total < space ? total : space;
    buf->data.append(static_cast<const char*>(contents), toCopy);
    // If we didn't accept full chunk, returning less signals libcurl to consider it an error to stop fetching more.
    return toCopy;
}

static bool is_http_url(const std::string& url) {
    return url.rfind("http://", 0) == 0 || url.rfind("https://", 0) == 0;
}

static std::string now_iso() {
    std::time_t t = std::time(nullptr);
    char buf[32];
    std::tm gm{};
#if defined(_WIN32)
    gmtime_s(&gm, &t);
#else
    gmtime_r(&t, &gm);
#endif
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &gm);
    return std::string(buf);
}

static std::string fetch_limited(const std::string& url, size_t maxBytes) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init curl");
    char errbuf[CURL_ERROR_SIZE] = {0};
    Buffer buf;
    buf.maxBytes = maxBytes;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task47Bot/1.0 (+https://example.local/bot)");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_capped);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::string emsg = errbuf[0] ? errbuf : curl_easy_strerror(res);
        throw std::runtime_error("curl error: " + emsg);
    }
    if (status < 200 || status >= 300) {
        throw std::runtime_error("HTTP status: " + std::to_string(status));
    }
    return buf.data;
}

static std::vector<std::pair<std::string, std::string>> extract_links(const std::string& html) {
    std::vector<std::pair<std::string, std::string>> v;
    if (html.empty()) return v;
    std::regex re(R"(((?i)<a\s+[^>]*href\s*=\s*\"([^\"]+)\"[^>]*>(.*?)</a>))", std::regex::icase | std::regex::optimize);
    auto it = std::sregex_iterator(html.begin(), html.end(), re);
    auto end = std::sregex_iterator();
    for (; it != end; ++it) {
        std::smatch m = *it;
        std::string href = m[2].str();
        std::string text = m[3].str();
        // strip tags from text
        text = std::regex_replace(text, std::regex(R"(<[^>]+>)", std::regex::icase), "");
        // trim
        auto ltrim = [](std::string&s){ s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c){return !std::isspace(c);}));};
        auto rtrim = [](std::string&s){ s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char c){return !std::isspace(c);} ).base(), s.end());};
        ltrim(href); rtrim(href); ltrim(text); rtrim(text);
        v.emplace_back(href, text);
    }
    return v;
}

int scrape_and_store(const std::string& url, const std::string& dbPath) {
    if (url.empty() || dbPath.empty()) throw std::invalid_argument("Null/empty arguments");
    if (!is_http_url(url)) throw std::invalid_argument("Only http/https URLs allowed");

    const size_t MAX_BYTES = 2'000'000;
    std::string html = fetch_limited(url, MAX_BYTES);
    auto links = extract_links(html);
    std::string when = now_iso();

    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        throw std::runtime_error("Failed to open database");
    }

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS links ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "page_url TEXT NOT NULL,"
        "link_url TEXT NOT NULL,"
        "link_text TEXT,"
        "scraped_at TEXT NOT NULL);";
    const char* idx_sql = "CREATE INDEX IF NOT EXISTS idx_links_page ON links(page_url);";

    char* errmsg = nullptr;
    if (sqlite3_exec(db, create_sql, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        std::string e = errmsg ? errmsg : "schema error";
        sqlite3_free(errmsg);
        sqlite3_close(db);
        throw std::runtime_error(e);
    }
    if (sqlite3_exec(db, idx_sql, nullptr, nullptr, &errmsg) != SQLITE_OK) {
        std::string e = errmsg ? errmsg : "index error";
        sqlite3_free(errmsg);
        sqlite3_close(db);
        throw std::runtime_error(e);
    }

    const char* ins_sql = "INSERT INTO links(page_url, link_url, link_text, scraped_at) VALUES(?,?,?,?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, ins_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        throw std::runtime_error("prepare failed");
    }

    int inserted = 0;
    if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw std::runtime_error("begin tx failed");
    }

    for (const auto& pr : links) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_text(stmt, 1, url.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, pr.first.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, pr.second.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, when.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) inserted++;
        else if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
            // continue but could log
        }
    }

    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return inserted;
}

int main() {
    std::string db = "scrape_demo_cpp.db";
    std::vector<std::string> tests = {
        "https://example.com",
        "https://www.iana.org/domains/reserved",
        "https://httpbin.org/html",
        "https://www.wikipedia.org",
        "https://www.rfc-editor.org/rfc/rfc2616"
    };
    for (const auto& t : tests) {
        try {
            int n = scrape_and_store(t, db);
            std::cout << "Scraped " << t << " -> stored " << n << " links.\n";
        } catch (const std::exception& e) {
            std::cout << "Error scraping " << t << ": " << e.what() << "\n";
        }
    }
    return 0;
}