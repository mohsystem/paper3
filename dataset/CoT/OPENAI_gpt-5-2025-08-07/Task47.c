/* Chain-of-Through secure implementation
 * 1) Problem: Fetch HTML from URL, extract anchors, store into SQLite. Inputs: url, dbPath. Output: count inserted.
 * 2) Security: allow only http/https, timeouts, max size 2MB, prepared statements, safe UA, error checks.
 * 3) Implementation: libcurl for HTTP with capped write callback, sqlite3 for DB, simple anchor scanner.
 * 4) Review: URL scheme validation, cap size, parameterized inserts, handle errors, bounds checks.
 * 5) Output: final code below.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <curl/curl.h>
#include <sqlite3.h>

typedef struct {
    char* data;
    size_t size;
    size_t cap;
    size_t maxBytes;
} Buffer;

static size_t write_capped(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    Buffer* buf = (Buffer*)userp;
    if (buf->size >= buf->maxBytes) {
        return 0;
    }
    size_t space = buf->maxBytes - buf->size;
    size_t toCopy = total < space ? total : space;
    if (buf->size + toCopy + 1 > buf->cap) {
        size_t newcap = buf->cap ? buf->cap : 4096;
        while (buf->size + toCopy + 1 > newcap) newcap *= 2;
        char* nd = (char*)realloc(buf->data, newcap);
        if (!nd) return 0;
        buf->data = nd;
        buf->cap = newcap;
    }
    memcpy(buf->data + buf->size, contents, toCopy);
    buf->size += toCopy;
    buf->data[buf->size] = '\0';
    return toCopy;
}

static int is_http_url(const char* url) {
    if (!url) return 0;
    return (strncmp(url, "http://", 7) == 0) || (strncmp(url, "https://", 8) == 0);
}

static char* fetch_limited(const char* url, size_t maxBytes) {
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;
    Buffer buf = {0};
    buf.maxBytes = maxBytes;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task47Bot/1.0 (+https://example.local/bot)");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_capped);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || status < 200 || status >= 300) {
        free(buf.data);
        return NULL;
    }
    return buf.data; // caller frees
}

// Simple anchor extraction: searches for <a ... href="...">...</a>, double-quoted href.
typedef struct {
    char* href;
    char* text;
} Link;

static void trim(char* s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t i = 0;
    while (i < len && isspace((unsigned char)s[i])) i++;
    size_t j = len;
    while (j > i && isspace((unsigned char)s[j-1])) j--;
    if (i > 0) memmove(s, s + i, j - i);
    s[j - i] = '\0';
}

static char* strip_tags(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    size_t j = 0;
    int in_tag = 0;
    for (size_t i = 0; i < n; i++) {
        if (s[i] == '<') in_tag = 1;
        else if (s[i] == '>') in_tag = 0;
        else if (!in_tag) out[j++] = s[i];
    }
    out[j] = '\0';
    return out;
}

static Link* extract_links(const char* html, int* outCount) {
    *outCount = 0;
    if (!html) return NULL;
    const char* p = html;
    size_t cap = 16;
    Link* arr = (Link*)malloc(cap * sizeof(Link));
    if (!arr) return NULL;

    while ((p = strcasestr(p, "<a")) != NULL) {
        const char* tag_end = strchr(p, '>');
        if (!tag_end) break;
        const char* href_pos = strcasestr(p, "href=");
        if (!href_pos || href_pos > tag_end) {
            p = tag_end + 1;
            continue;
        }
        href_pos += 5; // after href=
        while (*href_pos && isspace((unsigned char)*href_pos)) href_pos++;
        if (*href_pos != '\"') { p = tag_end + 1; continue; }
        href_pos++; // skip quote
        const char* href_end = strchr(href_pos, '\"');
        if (!href_end || href_end > tag_end) { p = tag_end + 1; continue; }

        size_t href_len = (size_t)(href_end - href_pos);
        char* href = (char*)malloc(href_len + 1);
        if (!href) { p = tag_end + 1; continue; }
        memcpy(href, href_pos, href_len);
        href[href_len] = '\0';
        trim(href);

        // find closing </a>
        const char* close = strcasestr(tag_end + 1, "</a>");
        if (!close) { free(href); p = tag_end + 1; continue; }

        size_t inner_len = (size_t)(close - (tag_end + 1));
        char* inner = (char*)malloc(inner_len + 1);
        if (!inner) { free(href); p = close + 4; continue; }
        memcpy(inner, tag_end + 1, inner_len);
        inner[inner_len] = '\0';

        char* text_no_tags = strip_tags(inner);
        free(inner);
        if (!text_no_tags) { free(href); p = close + 4; continue; }
        trim(text_no_tags);

        if ((size_t)(*outCount) >= cap) {
            cap *= 2;
            Link* tmp = (Link*)realloc(arr, cap * sizeof(Link));
            if (!tmp) { free(href); free(text_no_tags); break; }
            arr = tmp;
        }
        arr[*outCount].href = href;
        arr[*outCount].text = text_no_tags;
        (*outCount)++;

        p = close + 4;
    }
    return arr;
}

static char* now_iso() {
    time_t t = time(NULL);
    struct tm gm;
#if defined(_WIN32)
    gmtime_s(&gm, &t);
#else
    gmtime_r(&t, &gm);
#endif
    char* buf = (char*)malloc(21);
    if (!buf) return NULL;
    strftime(buf, 21, "%Y-%m-%dT%H:%M:%SZ", &gm);
    return buf;
}

int scrape_and_store(const char* url, const char* dbPath) {
    if (!url || !dbPath) return -1;
    if (!is_http_url(url)) return -2;

    const size_t MAX_BYTES = 2000000;
    char* html = fetch_limited(url, MAX_BYTES);
    if (!html) return -3;

    int count = 0;
    Link* links = extract_links(html, &count);
    free(html);

    sqlite3* db = NULL;
    if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        // free links
        for (int i = 0; i < count; i++) { free(links[i].href); free(links[i].text); }
        free(links);
        return -4;
    }

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS links ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "page_url TEXT NOT NULL,"
        "link_url TEXT NOT NULL,"
        "link_text TEXT,"
        "scraped_at TEXT NOT NULL);";
    const char* idx_sql = "CREATE INDEX IF NOT EXISTS idx_links_page ON links(page_url);";
    char* errmsg = NULL;
    if (sqlite3_exec(db, create_sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        if (errmsg) sqlite3_free(errmsg);
        sqlite3_close(db);
        for (int i = 0; i < count; i++) { free(links[i].href); free(links[i].text); }
        free(links);
        return -5;
    }
    if (sqlite3_exec(db, idx_sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        if (errmsg) sqlite3_free(errmsg);
        sqlite3_close(db);
        for (int i = 0; i < count; i++) { free(links[i].href); free(links[i].text); }
        free(links);
        return -6;
    }

    const char* ins_sql = "INSERT INTO links(page_url, link_url, link_text, scraped_at) VALUES(?,?,?,?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, ins_sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        for (int i = 0; i < count; i++) { free(links[i].href); free(links[i].text); }
        free(links);
        return -7;
    }

    char* when = now_iso();
    int inserted = 0;

    sqlite3_exec(db, "BEGIN", NULL, NULL, NULL);
    for (int i = 0; i < count; i++) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_text(stmt, 1, url, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, links[i].href, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, links[i].text, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, when ? when : "", -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) inserted++;
    }
    sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (when) free(when);
    for (int i = 0; i < count; i++) { free(links[i].href); free(links[i].text); }
    free(links);

    return inserted;
}

int main(void) {
    const char* db = "scrape_demo_c.db";
    const char* tests[5] = {
        "https://example.com",
        "https://www.iana.org/domains/reserved",
        "https://httpbin.org/html",
        "https://www.wikipedia.org",
        "https://www.rfc-editor.org/rfc/rfc2616"
    };
    for (int i = 0; i < 5; i++) {
        int n = scrape_and_store(tests[i], db);
        if (n >= 0) {
            printf("Scraped %s -> stored %d links.\n", tests[i], n);
        } else {
            printf("Error scraping %s: code %d\n", tests[i], n);
        }
    }
    return 0;
}