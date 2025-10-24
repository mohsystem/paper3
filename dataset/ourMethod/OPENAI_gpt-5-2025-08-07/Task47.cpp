#include <curl/curl.h>
#include <sqlite3.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

namespace SecureScraper {

static const size_t MAX_DOWNLOAD_BYTES = 5 * 1024 * 1024; // 5 MiB
static const size_t MAX_LINKS = 1000;
static const long HTTP_TIMEOUT_SECONDS = 20;
static const long CONNECT_TIMEOUT_SECONDS = 10;
static const long MAX_REDIRECTS = 5;

struct HttpResponse {
    long status = 0;
    std::string body;
    std::string error;
};

struct ParsedData {
    std::string title;
    std::vector<std::string> links;
};

struct WriteCtx {
    std::string* buffer;
    size_t maxBytes;
    bool overflow;
};

static bool starts_with_https(const std::string& url) {
    if (url.size() < 9) return false;
    std::string prefix = "https://";
    return std::equal(prefix.begin(), prefix.end(), url.begin(), url.begin() + prefix.size(),
                      [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

static bool validate_url(const std::string& url, std::string& out_error) {
    if (url.empty()) { out_error = "URL is empty"; return false; }
    if (url.size() > 2048) { out_error = "URL too long"; return false; }
    if (!starts_with_https(url)) { out_error = "Only https scheme is allowed"; return false; }
    // Basic whitespace check
    if (url.find_first_of(" \t\r\n") != std::string::npos) { out_error = "URL contains whitespace"; return false; }
    return true;
}

static size_t curl_write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t realSize = size * nmemb;
    WriteCtx* ctx = static_cast<WriteCtx*>(userdata);
    if (!ctx || !ctx->buffer) return 0;
    if (ctx->buffer->size() + realSize > ctx->maxBytes) {
        size_t toCopy = ctx->maxBytes > ctx->buffer->size() ? (ctx->maxBytes - ctx->buffer->size()) : 0;
        if (toCopy > 0) ctx->buffer->append(ptr, ptr + toCopy);
        ctx->overflow = true;
        return 0; // abort transfer
    }
    ctx->buffer->append(ptr, ptr + realSize);
    return realSize;
}

static HttpResponse https_get(const std::string& url) {
    HttpResponse resp;
    std::string validationError;
    if (!validate_url(url, validationError)) {
        resp.error = validationError;
        return resp;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        resp.error = "Failed to init CURL";
        return resp;
    }

    char errbuf[CURL_ERROR_SIZE];
    errbuf[0] = '\0';

    std::string buffer;
    WriteCtx ctx{&buffer, MAX_DOWNLOAD_BYTES, false};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIRECTS);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureScraperCPP/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
    // TLS/SSL verification
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    // Protocol hardening: disallow redirects to non-HTTPS
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);

    CURLcode code = curl_easy_perform(curl);
    long http_code = 0;
    if (code == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }

    if (code != CURLE_OK) {
        resp.error = errbuf[0] ? std::string(errbuf) : curl_easy_strerror(code);
    } else if (ctx.overflow) {
        resp.error = "Download exceeded maximum allowed size";
    } else if (http_code < 200 || http_code >= 300) {
        resp.error = "HTTP error code: " + std::to_string(http_code);
    } else {
        resp.body = std::move(buffer);
    }

    resp.status = http_code;
    curl_easy_cleanup(curl);
    return resp;
}

static std::string to_lower(const std::string& s) {
    std::string o;
    o.reserve(s.size());
    for (char c : s) o.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    return o;
}

static std::string extract_title(const std::string& html) {
    std::string lower = to_lower(html);
    auto pos1 = lower.find("<title>");
    if (pos1 == std::string::npos) return std::string();
    auto pos2 = lower.find("</title>", pos1 + 7);
    if (pos2 == std::string::npos) return std::string();
    std::string title = html.substr(pos1 + 7, pos2 - (pos1 + 7));
    // Trim
    auto l = title.find_first_not_of(" \t\r\n");
    auto r = title.find_last_not_of(" \t\r\n");
    if (l == std::string::npos) return std::string();
    return title.substr(l, r - l + 1);
}

static std::vector<std::string> extract_links(const std::string& html) {
    std::vector<std::string> links;
    links.reserve(64);
    try {
        std::regex r(R"(href\s*=\s*("(.*?)"|'(.*?)'))", std::regex::icase);
        auto begin = std::sregex_iterator(html.begin(), html.end(), r);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            if (links.size() >= MAX_LINKS) break;
            const std::smatch& m = *it;
            std::string candidate;
            if (m.size() >= 4) {
                candidate = m[2].matched ? m[2].str() : (m[3].matched ? m[3].str() : std::string());
            }
            if (candidate.empty()) continue;
            // Basic sanitization: keep only https links, skip javascript:, mailto:, data:
            std::string lower = to_lower(candidate);
            if (lower.rfind("https://", 0) == 0) {
                links.push_back(candidate);
            }
        }
    } catch (...) {
        // If regex engine throws, return what we have
    }
    return links;
}

static bool ensure_schema(sqlite3* db, std::string& out_error) {
    static const char* ddl =
        "PRAGMA foreign_keys = ON; "
        "CREATE TABLE IF NOT EXISTS pages ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  url TEXT NOT NULL,"
        "  title TEXT,"
        "  fetched_at TEXT NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS links ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  page_id INTEGER NOT NULL,"
        "  href TEXT NOT NULL,"
        "  FOREIGN KEY(page_id) REFERENCES pages(id) ON DELETE CASCADE"
        ");";
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, ddl, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        out_error = errMsg ? std::string(errMsg) : "Failed to create schema";
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

static bool store_to_db(const std::string& db_path,
                        const std::string& url,
                        const ParsedData& data,
                        std::string& out_error) {
    sqlite3* db = nullptr;
    int rc = sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) {
        out_error = "Failed to open database";
        if (db) sqlite3_close(db);
        return false;
    }

    bool ok = ensure_schema(db, out_error);
    if (!ok) { sqlite3_close(db); return false; }

    // Begin transaction
    rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        out_error = "Failed to begin transaction";
        sqlite3_close(db);
        return false;
    }

    sqlite3_stmt* pageStmt = nullptr;
    const char* insertPageSql = "INSERT INTO pages(url, title, fetched_at) VALUES(?, ?, datetime('now'));";
    rc = sqlite3_prepare_v2(db, insertPageSql, -1, &pageStmt, nullptr);
    if (rc != SQLITE_OK) {
        out_error = "Failed to prepare page insert";
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(pageStmt, 1, url.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(pageStmt, 2, data.title.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(pageStmt);
    if (rc != SQLITE_DONE) {
        out_error = "Failed to insert page";
        sqlite3_finalize(pageStmt);
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return false;
    }
    sqlite3_finalize(pageStmt);

    sqlite3_int64 pageId = sqlite3_last_insert_rowid(db);

    sqlite3_stmt* linkStmt = nullptr;
    const char* insertLinkSql = "INSERT INTO links(page_id, href) VALUES(?, ?);";
    rc = sqlite3_prepare_v2(db, insertLinkSql, -1, &linkStmt, nullptr);
    if (rc != SQLITE_OK) {
        out_error = "Failed to prepare link insert";
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        return false;
    }

    for (const auto& href : data.links) {
        sqlite3_bind_int64(linkStmt, 1, pageId);
        sqlite3_bind_text(linkStmt, 2, href.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(linkStmt);
        if (rc != SQLITE_DONE) {
            out_error = "Failed to insert link";
            sqlite3_finalize(linkStmt);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            sqlite3_close(db);
            return false;
        }
        sqlite3_reset(linkStmt);
        sqlite3_clear_bindings(linkStmt);
    }
    sqlite3_finalize(linkStmt);

    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        out_error = "Failed to commit transaction";
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    return true;
}

static ParsedData parse_html(const std::string& html) {
    ParsedData out;
    out.title = extract_title(html);
    out.links = extract_links(html);
    return out;
}

// Public API: takes URL and DB path, returns true on success
bool scrape_and_store(const std::string& url, const std::string& db_path, std::string& out_error) {
    HttpResponse resp = https_get(url);
    if (!resp.error.empty()) {
        out_error = resp.error;
        return false;
    }
    ParsedData parsed = parse_html(resp.body);
    return store_to_db(db_path, url, parsed, out_error);
}

} // namespace SecureScraper

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    struct TestCase { std::string url; std::string db; };
    std::vector<TestCase> tests = {
        {"https://example.com", "scrape_cpp.db"},
        {"https://www.iana.org/domains/reserved", "scrape_cpp.db"},
        {"https://www.wikipedia.org/", "scrape_cpp.db"},
        {"https://httpbin.org/html", "scrape_cpp.db"},
        {"https://www.rfc-editor.org/", "scrape_cpp.db"}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::string err;
        bool ok = SecureScraper::scrape_and_store(tests[i].url, tests[i].db, err);
        if (ok) {
            std::cout << "Test " << (i + 1) << " OK: " << tests[i].url << std::endl;
        } else {
            std::cout << "Test " << (i + 1) << " FAILED: " << tests[i].url << " - " << err << std::endl;
        }
    }

    curl_global_cleanup();
    return 0;
}