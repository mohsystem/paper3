#include <curl/curl.h>
#include <sqlite3.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DOWNLOAD_BYTES (5 * 1024 * 1024) /* 5 MiB */
#define MAX_LINKS 1000
#define HTTP_TIMEOUT_SECONDS 20L
#define CONNECT_TIMEOUT_SECONDS 10L
#define MAX_REDIRECTS 5L

typedef struct {
    long status;
    char *body;
    size_t body_len;
    char error[256];
} HttpResponse;

typedef struct {
    char *title;
    char **links;
    size_t link_count;
} ParsedData;

typedef struct {
    char *buf;
    size_t size;
    size_t cap;
    size_t max;
    int overflow;
} Buffer;

static void buffer_init(Buffer *b, size_t cap, size_t max) {
    if (!b) return;
    b->buf = (char *)malloc(cap);
    if (b->buf) b->buf[0] = '\0';
    b->size = 0;
    b->cap = b->buf ? cap : 0;
    b->max = max;
    b->overflow = 0;
}

static void buffer_free(Buffer *b) {
    if (!b) return;
    if (b->buf) free(b->buf);
    b->buf = NULL;
    b->size = 0;
    b->cap = 0;
    b->max = 0;
    b->overflow = 0;
}

static int buffer_append(Buffer *b, const char *data, size_t n) {
    if (!b || !data) return 0;
    if (b->size + n > b->max) {
        size_t toCopy = (b->max > b->size) ? (b->max - b->size) : 0;
        if (toCopy > 0) {
            if (b->size + toCopy + 1 > b->cap) {
                size_t newCap = b->cap ? b->cap : 1;
                while (b->size + toCopy + 1 > newCap) newCap *= 2;
                char *nb = (char *)realloc(b->buf, newCap);
                if (!nb) { b->overflow = 1; return 0; }
                b->buf = nb; b->cap = newCap;
            }
            memcpy(b->buf + b->size, data, toCopy);
            b->size += toCopy;
            b->buf[b->size] = '\0';
        }
        b->overflow = 1;
        return 0;
    }
    if (b->size + n + 1 > b->cap) {
        size_t newCap = b->cap ? b->cap : 1;
        while (b->size + n + 1 > newCap) newCap *= 2;
        char *nb = (char *)realloc(b->buf, newCap);
        if (!nb) return 0;
        b->buf = nb; b->cap = newCap;
    }
    memcpy(b->buf + b->size, data, n);
    b->size += n;
    b->buf[b->size] = '\0';
    return 1;
}

static size_t curl_write_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t realSize = size * nmemb;
    Buffer *b = (Buffer *)userdata;
    if (!b) return 0;
    if (!buffer_append(b, ptr, realSize)) {
        return 0; /* abort */
    }
    return realSize;
}

static int starts_with_https(const char *url) {
    if (!url) return 0;
    const char *p = "https://";
    for (size_t i = 0; p[i]; ++i) {
        if (!url[i]) return 0;
        char a = (char)tolower((unsigned char)p[i]);
        char b = (char)tolower((unsigned char)url[i]);
        if (a != b) return 0;
    }
    return 1;
}

static int validate_url(const char *url, char *out_err, size_t out_err_sz) {
    if (!url || !*url) { snprintf(out_err, out_err_sz, "%s", "URL is empty"); return 0; }
    size_t len = strlen(url);
    if (len > 2048) { snprintf(out_err, out_err_sz, "%s", "URL too long"); return 0; }
    if (!starts_with_https(url)) { snprintf(out_err, out_err_sz, "%s", "Only https scheme is allowed"); return 0; }
    for (size_t i = 0; i < len; ++i) {
        if (url[i] == ' ' || url[i] == '\t' || url[i] == '\r' || url[i] == '\n') {
            snprintf(out_err, out_err_sz, "%s", "URL contains whitespace"); return 0;
        }
    }
    return 1;
}

static HttpResponse https_get(const char *url) {
    HttpResponse resp;
    resp.status = 0;
    resp.body = NULL;
    resp.body_len = 0;
    resp.error[0] = '\0';

    char verr[128]; verr[0] = '\0';
    if (!validate_url(url, verr, sizeof(verr))) {
        snprintf(resp.error, sizeof(resp.error), "%s", verr);
        return resp;
    }

    CURL *curl = curl_easy_init();
    if (!curl) { snprintf(resp.error, sizeof(resp.error), "%s", "Failed to init CURL"); return resp; }

    Buffer b; buffer_init(&b, 8192, MAX_DOWNLOAD_BYTES);

    char errbuf[CURL_ERROR_SIZE]; errbuf[0] = '\0';
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIRECTS);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &b);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureScraperC/1.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
    /* TLS verification */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    /* Restrict to HTTPS only, including redirects */
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);

    CURLcode code = curl_easy_perform(curl);
    long http_code = 0;
    if (code == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }

    if (code != CURLE_OK) {
        snprintf(resp.error, sizeof(resp.error), "%s", errbuf[0] ? errbuf : curl_easy_strerror(code));
    } else if (b.overflow) {
        snprintf(resp.error, sizeof(resp.error), "%s", "Download exceeded maximum allowed size");
    } else if (http_code < 200 || http_code >= 300) {
        snprintf(resp.error, sizeof(resp.error), "HTTP error code: %ld", http_code);
    } else {
        resp.body = b.buf; /* take ownership */
        resp.body_len = b.size;
        b.buf = NULL; /* prevent free */
    }
    resp.status = http_code;

    buffer_free(&b);
    curl_easy_cleanup(curl);
    return resp;
}

static char *str_to_lower_dup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char *o = (char *)malloc(n + 1);
    if (!o) return NULL;
    for (size_t i = 0; i < n; ++i) o[i] = (char)tolower((unsigned char)s[i]);
    o[n] = '\0';
    return o;
}

static char *safe_strndup(const char *s, size_t n) {
    if (!s) return NULL;
    char *o = (char *)malloc(n + 1);
    if (!o) return NULL;
    memcpy(o, s, n);
    o[n] = '\0';
    return o;
}

static char *trim_dup(const char *s, size_t n) {
    if (!s) return NULL;
    size_t l = 0, r = n;
    while (l < r && (s[l] == ' ' || s[l] == '\t' || s[l] == '\r' || s[l] == '\n')) l++;
    while (r > l && (s[r - 1] == ' ' || s[r - 1] == '\t' || s[r - 1] == '\r' || s[r - 1] == '\n')) r--;
    return safe_strndup(s + l, r - l);
}

static char *extract_title(const char *html) {
    if (!html) return NULL;
    char *lower = str_to_lower_dup(html);
    if (!lower) return NULL;
    const char *open = strstr(lower, "<title>");
    if (!open) { free(lower); return NULL; }
    const char *close = strstr(open + 7, "</title>");
    if (!close) { free(lower); return NULL; }
    size_t start = (size_t)(open + 7 - lower);
    size_t end = (size_t)(close - lower);
    char *title = trim_dup(html + start, end - start);
    free(lower);
    return title;
}

static int parse_links(const char *html, char ***out_links, size_t *out_count) {
    if (!html || !out_links || !out_count) return 0;
    *out_links = NULL;
    *out_count = 0;

    size_t cap = 16;
    char **links = (char **)malloc(sizeof(char *) * cap);
    if (!links) return 0;

    size_t count = 0;
    const char *p = html;
    while (*p && count < MAX_LINKS) {
        const char *h = p;
        /* find 'href' case-insensitive */
        while (*h) {
            if ((h[0] == 'h' || h[0] == 'H') &&
                (h[1] == 'r' || h[1] == 'R') &&
                (h[2] == 'e' || h[2] == 'E') &&
                (h[3] == 'f' || h[3] == 'F')) {
                break;
            }
            h++;
        }
        if (!*h) break;
        const char *q = h + 4;
        /* skip spaces */
        while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n') q++;
        if (*q != '=') { p = h + 4; continue; }
        q++;
        while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n') q++;
        if (*q != '"' && *q != '\'') { p = q; continue; }
        char quote = *q;
        q++;
        const char *val_start = q;
        while (*q && *q != quote) q++;
        if (*q != quote) { p = q; continue; } /* malformed */
        size_t len = (size_t)(q - val_start);
        if (len > 0) {
            /* Only accept https:// links */
            if (len >= 8) {
                int ok = 1;
                const char *s = val_start;
                const char *pref = "https://";
                for (int i = 0; i < 8; ++i) {
                    char a = (char)tolower((unsigned char)pref[i]);
                    char b = (char)tolower((unsigned char)s[i]);
                    if (a != b) { ok = 0; break; }
                }
                if (ok) {
                    char *href = safe_strndup(val_start, len);
                    if (!href) {
                        /* free allocated links */
                        for (size_t i = 0; i < count; ++i) free(links[i]);
                        free(links);
                        return 0;
                    }
                    if (count == cap) {
                        size_t ncap = cap * 2;
                        if (ncap > MAX_LINKS) ncap = MAX_LINKS;
                        char **nlinks = (char **)realloc(links, sizeof(char *) * ncap);
                        if (!nlinks) {
                            free(href);
                            for (size_t i = 0; i < count; ++i) free(links[i]);
                            free(links);
                            return 0;
                        }
                        links = nlinks; cap = ncap;
                    }
                    links[count++] = href;
                }
            }
        }
        p = q + 1;
    }

    *out_links = links;
    *out_count = count;
    return 1;
}

static void free_parsed(ParsedData *pd) {
    if (!pd) return;
    if (pd->title) { free(pd->title); pd->title = NULL; }
    if (pd->links) {
        for (size_t i = 0; i < pd->link_count; ++i) {
            if (pd->links[i]) free(pd->links[i]);
        }
        free(pd->links);
        pd->links = NULL;
    }
    pd->link_count = 0;
}

static int ensure_schema(sqlite3 *db, char *errbuf, size_t errbuf_sz) {
    const char *ddl =
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
    char *errmsg = NULL;
    int rc = sqlite3_exec(db, ddl, NULL, NULL, &errmsg);
    if (rc != SQLITE_OK) {
        snprintf(errbuf, errbuf_sz, "%s", errmsg ? errmsg : "Failed to create schema");
        if (errmsg) sqlite3_free(errmsg);
        return 0;
    }
    return 1;
}

static int store_to_db(const char *db_path, const char *url, const ParsedData *pd, char *errbuf, size_t errbuf_sz) {
    sqlite3 *db = NULL;
    int rc = sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc != SQLITE_OK) {
        snprintf(errbuf, errbuf_sz, "%s", "Failed to open database");
        if (db) sqlite3_close(db);
        return 0;
    }

    if (!ensure_schema(db, errbuf, errbuf_sz)) {
        sqlite3_close(db);
        return 0;
    }

    rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        snprintf(errbuf, errbuf_sz, "%s", "Failed to begin transaction");
        sqlite3_close(db);
        return 0;
    }

    sqlite3_stmt *pstmt = NULL;
    const char *sql_page = "INSERT INTO pages(url, title, fetched_at) VALUES(?, ?, datetime('now'));";
    rc = sqlite3_prepare_v2(db, sql_page, -1, &pstmt, NULL);
    if (rc != SQLITE_OK) {
        snprintf(errbuf, errbuf_sz, "%s", "Failed to prepare page insert");
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_close(db);
        return 0;
    }
    sqlite3_bind_text(pstmt, 1, url, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(pstmt, 2, pd->title ? pd->title : "", -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(pstmt);
    if (rc != SQLITE_DONE) {
        snprintf(errbuf, errbuf_sz, "%s", "Failed to insert page");
        sqlite3_finalize(pstmt);
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_close(db);
        return 0;
    }
    sqlite3_finalize(pstmt);

    sqlite3_int64 page_id = sqlite3_last_insert_rowid(db);

    sqlite3_stmt *lstmt = NULL;
    const char *sql_link = "INSERT INTO links(page_id, href) VALUES(?, ?);";
    rc = sqlite3_prepare_v2(db, sql_link, -1, &lstmt, NULL);
    if (rc != SQLITE_OK) {
        snprintf(errbuf, errbuf_sz, "%s", "Failed to prepare link insert");
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_close(db);
        return 0;
    }

    for (size_t i = 0; i < pd->link_count; ++i) {
        sqlite3_bind_int64(lstmt, 1, page_id);
        sqlite3_bind_text(lstmt, 2, pd->links[i], -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(lstmt);
        if (rc != SQLITE_DONE) {
            snprintf(errbuf, errbuf_sz, "%s", "Failed to insert link");
            sqlite3_finalize(lstmt);
            sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
            sqlite3_close(db);
            return 0;
        }
        sqlite3_reset(lstmt);
        sqlite3_clear_bindings(lstmt);
    }
    sqlite3_finalize(lstmt);

    rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        snprintf(errbuf, errbuf_sz, "%s", "Failed to commit transaction");
        sqlite3_close(db);
        return 0;
    }

    sqlite3_close(db);
    return 1;
}

/* Public API: returns 1 on success, 0 on error; errbuf filled on error */
int scrape_and_store(const char *url, const char *db_path, char *errbuf, size_t errbuf_sz) {
    if (!url || !db_path || !errbuf) return 0;
    errbuf[0] = '\0';

    HttpResponse resp = https_get(url);
    if (resp.error[0]) {
        snprintf(errbuf, errbuf_sz, "%s", resp.error);
        if (resp.body) free(resp.body);
        return 0;
    }

    ParsedData pd;
    pd.title = extract_title(resp.body);
    pd.links = NULL;
    pd.link_count = 0;
    int ok = parse_links(resp.body, &pd.links, &pd.link_count);
    if (!ok) {
        snprintf(errbuf, errbuf_sz, "%s", "Failed to parse links");
        if (resp.body) free(resp.body);
        free_parsed(&pd);
        return 0;
    }

    int stored = store_to_db(db_path, url, &pd, errbuf, errbuf_sz);

    if (resp.body) {
        /* Clear and free response body */
        memset(resp.body, 0, resp.body_len);
        free(resp.body);
    }
    free_parsed(&pd);

    return stored;
}

int main(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    const char *db = "scrape_c.db";
    const char *tests[5] = {
        "https://example.com",
        "https://www.iana.org/domains/reserved",
        "https://www.wikipedia.org/",
        "https://httpbin.org/html",
        "https://www.rfc-editor.org/"
    };

    for (int i = 0; i < 5; ++i) {
        char err[256]; err[0] = '\0';
        int ok = scrape_and_store(tests[i], db, err, sizeof(err));
        if (ok) {
            printf("Test %d OK: %s\n", i + 1, tests[i]);
        } else {
            printf("Test %d FAILED: %s - %s\n", i + 1, tests[i], err);
        }
    }

    curl_global_cleanup();
    return 0;
}