#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <ctime>
#include <curl/curl.h>
#include <sqlite3.h>

struct LinkCPP {
    std::string href;
    std::string text;
};

static size_t CurlWriteCB(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    std::string* s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), total);
    return total;
}

static std::string fetchUrlCPP(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to init curl");
    std::string out;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task47Bot/1.0 (+https://example.org)");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCB);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::string err = curl_easy_strerror(res);
        curl_easy_cleanup(curl);
        throw std::runtime_error("CURL error: " + err);
    }
    curl_easy_cleanup(curl);
    return out;
}

static std::string extractTitleCPP(const std::string& html) {
    std::regex re("<title\\b[^>]*>(.*?)</title>", std::regex::icase | std::regex::optimize);
    std::smatch m;
    if (std::regex_search(html, m, re)) {
        std::string t = m[1].str();
        // collapse whitespace
        std::string out;
        bool ws = false;
        for (char c : t) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (!ws) { out.push_back(' '); ws = true; }
            } else { out.push_back(c); ws = false; }
        }
        // trim
        while (!out.empty() && std::isspace(static_cast<unsigned char>(out.front()))) out.erase(out.begin());
        while (!out.empty() && std::isspace(static_cast<unsigned char>(out.back()))) out.pop_back();
        return out;
    }
    return "";
}

static std::vector<LinkCPP> extractLinksCPP(const std::string& html) {
    std::vector<LinkCPP> res;
    std::regex re("<a\\b[^>]*href\\s*=\\s*([\"'])(.*?)\\1[^>]*>(.*?)</a>", std::regex::icase | std::regex::optimize);
    for (std::sregex_iterator it(html.begin(), html.end(), re), end; it != end; ++it) {
        std::string href = (*it)[2].str();
        std::string text = (*it)[3].str();
        // strip HTML tags from text
        text = std::regex_replace(text, std::regex("<[^>]+>"), "");
        // collapse whitespace
        std::string out;
        bool ws = false;
        for (char c : text) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (!ws) { out.push_back(' '); ws = true; }
            } else { out.push_back(c); ws = false; }
        }
        while (!out.empty() && std::isspace(static_cast<unsigned char>(out.front()))) out.erase(out.begin());
        while (!out.empty() && std::isspace(static_cast<unsigned char>(out.back()))) out.pop_back();
        res.push_back({href, out});
    }
    return res;
}

static int storeToSqliteCPP(const std::string& dbPath, const std::string& url, const std::string& title, const std::vector<LinkCPP>& links) {
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open DB: " + std::string(sqlite3_errmsg(db)));
    }
    char* err = nullptr;
    const char* createPages = "CREATE TABLE IF NOT EXISTS pages (id INTEGER PRIMARY KEY AUTOINCREMENT, url TEXT, title TEXT, fetched_at TEXT)";
    const char* createLinks = "CREATE TABLE IF NOT EXISTS links (id INTEGER PRIMARY KEY AUTOINCREMENT, page_id INTEGER, href TEXT, text TEXT, FOREIGN KEY(page_id) REFERENCES pages(id))";
    if (sqlite3_exec(db, createPages, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string e = err ? err : "unknown";
        sqlite3_free(err);
        sqlite3_close(db);
        throw std::runtime_error("Create pages failed: " + e);
    }
    if (sqlite3_exec(db, createLinks, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string e = err ? err : "unknown";
        sqlite3_free(err);
        sqlite3_close(db);
        throw std::runtime_error("Create links failed: " + e);
    }

    if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, &err) != SQLITE_OK) {
        std::string e = err ? err : "unknown";
        sqlite3_free(err);
        sqlite3_close(db);
        throw std::runtime_error("BEGIN failed: " + e);
    }

    sqlite3_stmt* insPage = nullptr;
    sqlite3_prepare_v2(db, "INSERT INTO pages(url, title, fetched_at) VALUES (?,?,?)", -1, &insPage, nullptr);
    std::time_t t = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", std::gmtime(&t));
    sqlite3_bind_text(insPage, 1, url.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insPage, 2, title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insPage, 3, buf, -1, SQLITE_TRANSIENT);
    if (sqlite3_step(insPage) != SQLITE_DONE) {
        sqlite3_finalize(insPage);
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        throw std::runtime_error("Insert page failed");
    }
    sqlite3_finalize(insPage);
    sqlite3_int64 pageId = sqlite3_last_insert_rowid(db);

    sqlite3_stmt* insLink = nullptr;
    sqlite3_prepare_v2(db, "INSERT INTO links(page_id, href, text) VALUES (?,?,?)", -1, &insLink, nullptr);
    for (const auto& l : links) {
        sqlite3_bind_int64(insLink, 1, pageId);
        sqlite3_bind_text(insLink, 2, l.href.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insLink, 3, l.text.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(insLink) != SQLITE_DONE) {
            sqlite3_finalize(insLink);
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            sqlite3_close(db);
            throw std::runtime_error("Insert link failed");
        }
        sqlite3_reset(insLink);
        sqlite3_clear_bindings(insLink);
    }
    sqlite3_finalize(insLink);

    if (sqlite3_exec(db, "COMMIT", nullptr, nullptr, &err) != SQLITE_OK) {
        std::string e = err ? err : "unknown";
        sqlite3_free(err);
        sqlite3_close(db);
        throw std::runtime_error("COMMIT failed: " + e);
    }

    sqlite3_close(db);
    return static_cast<int>(links.size());
}

int scrapeAndStore(const std::string& url, const std::string& dbPath) {
    std::string html = fetchUrlCPP(url);
    std::string title = extractTitleCPP(html);
    std::vector<LinkCPP> links = extractLinksCPP(html);
    int inserted = storeToSqliteCPP(dbPath, url, title, links);
    return inserted;
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::vector<std::string> urls = {
        "https://example.com",
        "https://www.wikipedia.org",
        "https://www.python.org",
        "https://www.gnu.org",
        "https://www.rfc-editor.org"
    };
    std::string dbPath = "scrape_cpp.db";
    for (const auto& u : urls) {
        try {
            int n = scrapeAndStore(u, dbPath);
            std::cout << "C++: Inserted " << n << " links from " << u << " into " << dbPath << std::endl;
        } catch (const std::exception& ex) {
            std::cout << "C++: Failed for " << u << " -> " << ex.what() << std::endl;
        }
    }
    curl_global_cleanup();
    return 0;
}