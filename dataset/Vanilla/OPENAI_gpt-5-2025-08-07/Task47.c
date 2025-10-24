#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <sqlite3.h>
#include <time.h>

typedef struct {
    char* data;
    size_t size;
} Buffer;

static size_t write_cb(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    Buffer* buf = (Buffer*)userp;
    char* newp = (char*)realloc(buf->data, buf->size + total + 1);
    if (!newp) return 0;
    buf->data = newp;
    memcpy(buf->data + buf->size, contents, total);
    buf->size += total;
    buf->data[buf->size] = '\0';
    return total;
}

static char* fetch_url_c(const char* url) {
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;
    Buffer buf = {0};
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task47Bot/1.0 (+https://example.org)");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        if (buf.data) free(buf.data);
        return NULL;
    }
    return buf.data;
}

static void tolower_inplace(char* s) {
    while (*s) { *s = (char)tolower((unsigned char)*s); s++; }
}

static char* strcasestr_simple(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    size_t nlen = strlen(needle);
    for (const char* p = haystack; *p; p++) {
        if (strncasecmp(p, needle, nlen) == 0) return (char*)p;
    }
    return NULL;
}

static char* extract_title_c(const char* html) {
    const char* p = strcasestr_simple(html, "<title");
    if (!p) return strdup("");
    p = strchr(p, '>');
    if (!p) return strdup("");
    p++;
    const char* q = strcasestr_simple(p, "</title>");
    if (!q) return strdup("");
    size_t len = (size_t)(q - p);
    char* t = (char*)malloc(len + 1);
    if (!t) return strdup("");
    memcpy(t, p, len);
    t[len] = '\0';
    // collapse whitespace
    char* out = (char*)malloc(len + 1);
    if (!out) { free(t); return strdup(""); }
    size_t oi = 0; int ws = 0;
    for (size_t i = 0; i < len; i++) {
        char c = t[i];
        if (isspace((unsigned char)c)) {
            if (!ws) { out[oi++] = ' '; ws = 1; }
        } else { out[oi++] = c; ws = 0; }
    }
    while (oi > 0 && isspace((unsigned char)out[oi-1])) oi--;
    size_t si = 0; while (si < oi && isspace((unsigned char)out[si])) si++;
    size_t newlen = oi - si;
    char* res = (char*)malloc(newlen + 1);
    if (!res) { free(t); free(out); return strdup(""); }
    memcpy(res, out + si, newlen);
    res[newlen] = '\0';
    free(t);
    free(out);
    return res;
}

typedef struct {
    char* href;
    char* text;
} LinkC;

typedef struct {
    LinkC* items;
    size_t size;
    size_t cap;
} LinkVec;

static void lv_init(LinkVec* v) { v->items = NULL; v->size = 0; v->cap = 0; }
static void lv_push(LinkVec* v, const char* href, const char* text) {
    if (v->size == v->cap) {
        size_t ncap = v->cap ? v->cap * 2 : 16;
        LinkC* n = (LinkC*)realloc(v->items, ncap * sizeof(LinkC));
        if (!n) return;
        v->items = n; v->cap = ncap;
    }
    v->items[v->size].href = strdup(href ? href : "");
    v->items[v->size].text = strdup(text ? text : "");
    v->size++;
}
static void lv_free(LinkVec* v) {
    for (size_t i = 0; i < v->size; i++) {
        free(v->items[i].href);
        free(v->items[i].text);
    }
    free(v->items);
}

static void strip_tags_inplace(char* s) {
    int in_tag = 0; size_t w = 0;
    for (size_t r = 0; s[r]; r++) {
        if (s[r] == '<') { in_tag = 1; continue; }
        if (s[r] == '>') { in_tag = 0; continue; }
        if (!in_tag) s[w++] = s[r];
    }
    s[w] = '\0';
}

static void collapse_ws_inplace(char* s) {
    size_t w = 0; int ws = 0;
    for (size_t r = 0; s[r]; r++) {
        if (isspace((unsigned char)s[r])) {
            if (!ws) { s[w++] = ' '; ws = 1; }
        } else { s[w++] = s[r]; ws = 0; }
    }
    while (w > 0 && isspace((unsigned char)s[w-1])) w--;
    size_t start = 0;
    while (start < w && isspace((unsigned char)s[start])) start++;
    if (start > 0) memmove(s, s + start, w - start);
    s[w - start] = '\0';
}

static LinkVec extract_links_c(const char* html) {
    LinkVec v; lv_init(&v);
    const char* p = html;
    while ((p = strcasestr_simple(p, "<a")) != NULL) {
        const char* tag_end = strchr(p, '>');
        if (!tag_end) break;
        const char* close = strcasestr_simple(tag_end, "</a>");
        if (!close) break;

        // Find href within [p, tag_end)
        const char* h = p;
        const char* href_pos = NULL;
        while ((h = strcasestr_simple(h, "href")) && h < tag_end) {
            href_pos = h;
            break;
        }
        char* href_val = NULL;
        if (href_pos && href_pos < tag_end) {
            const char* eq = strchr(href_pos, '=');
            if (eq && eq < tag_end) {
                eq++;
                while (eq < tag_end && isspace((unsigned char)*eq)) eq++;
                if (eq < tag_end) {
                    char quote = 0;
                    if (*eq == '"' || *eq == '\'') { quote = *eq; eq++; }
                    const char* end = eq;
                    if (quote) {
                        while (end < tag_end && *end != quote) end++;
                    } else {
                        while (end < tag_end && !isspace((unsigned char)*end) && *end != '>') end++;
                    }
                    size_t len = (size_t)(end - eq);
                    href_val = (char*)malloc(len + 1);
                    if (href_val) { memcpy(href_val, eq, len); href_val[len] = '\0'; }
                }
            }
        }
        // text between tag_end+1 and close
        size_t tlen = (size_t)(close - (tag_end + 1));
        char* text = (char*)malloc(tlen + 1);
        if (text) {
            memcpy(text, tag_end + 1, tlen);
            text[tlen] = '\0';
            strip_tags_inplace(text);
            collapse_ws_inplace(text);
        }
        if (!href_val) href_val = strdup("");
        if (!text) text = strdup("");
        lv_push(&v, href_val, text);
        free(href_val);
        free(text);

        p = close + 4; // move past </a>
    }
    return v;
}

static int store_to_sqlite_c(const char* db_path, const char* url, const char* title, LinkVec* links) {
    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return -1;
    }
    char* err = NULL;
    const char* createPages = "CREATE TABLE IF NOT EXISTS pages (id INTEGER PRIMARY KEY AUTOINCREMENT, url TEXT, title TEXT, fetched_at TEXT)";
    const char* createLinks = "CREATE TABLE IF NOT EXISTS links (id INTEGER PRIMARY KEY AUTOINCREMENT, page_id INTEGER, href TEXT, text TEXT, FOREIGN KEY(page_id) REFERENCES pages(id))";
    if (sqlite3_exec(db, createPages, NULL, NULL, &err) != SQLITE_OK) { if (err) sqlite3_free(err); sqlite3_close(db); return -2; }
    if (sqlite3_exec(db, createLinks, NULL, NULL, &err) != SQLITE_OK) { if (err) sqlite3_free(err); sqlite3_close(db); return -3; }
    if (sqlite3_exec(db, "BEGIN", NULL, NULL, &err) != SQLITE_OK) { if (err) sqlite3_free(err); sqlite3_close(db); return -4; }

    sqlite3_stmt* insPage = NULL;
    sqlite3_prepare_v2(db, "INSERT INTO pages(url, title, fetched_at) VALUES (?,?,?)", -1, &insPage, NULL);
    time_t tt = time(NULL);
    char ts[64]; strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", gmtime(&tt));
    sqlite3_bind_text(insPage, 1, url, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insPage, 2, title, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insPage, 3, ts, -1, SQLITE_TRANSIENT);
    if (sqlite3_step(insPage) != SQLITE_DONE) { sqlite3_finalize(insPage); sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL); sqlite3_close(db); return -5; }
    sqlite3_finalize(insPage);
    sqlite3_int64 pageId = sqlite3_last_insert_rowid(db);

    sqlite3_stmt* insLink = NULL;
    sqlite3_prepare_v2(db, "INSERT INTO links(page_id, href, text) VALUES (?,?,?)", -1, &insLink, NULL);
    for (size_t i = 0; i < links->size; i++) {
        sqlite3_bind_int64(insLink, 1, pageId);
        sqlite3_bind_text(insLink, 2, links->items[i].href, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insLink, 3, links->items[i].text, -1, SQLITE_TRANSIENT);
        if (sqlite3_step(insLink) != SQLITE_DONE) { sqlite3_finalize(insLink); sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL); sqlite3_close(db); return -6; }
        sqlite3_reset(insLink);
        sqlite3_clear_bindings(insLink);
    }
    sqlite3_finalize(insLink);
    if (sqlite3_exec(db, "COMMIT", NULL, NULL, &err) != SQLITE_OK) { if (err) sqlite3_free(err); sqlite3_close(db); return -7; }
    sqlite3_close(db);
    return (int)links->size;
}

int scrape_and_store(const char* url, const char* db_path) {
    char* html = fetch_url_c(url);
    if (!html) return -100;
    char* title = extract_title_c(html);
    LinkVec links = extract_links_c(html);
    int res = store_to_sqlite_c(db_path, url, title, &links);
    lv_free(&links);
    free(title);
    free(html);
    return res;
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    const char* db_path = "scrape_c.db";
    const char* urls[5] = {
        "https://example.com",
        "https://www.wikipedia.org",
        "https://www.python.org",
        "https://www.gnu.org",
        "https://www.rfc-editor.org"
    };
    for (int i = 0; i < 5; i++) {
        int n = scrape_and_store(urls[i], db_path);
        if (n >= 0) {
            printf("C: Inserted %d links from %s into %s\n", n, urls[i], db_path);
        } else {
            printf("C: Failed for %s -> code %d\n", urls[i], n);
        }
    }
    curl_global_cleanup();
    return 0;
}