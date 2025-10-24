#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <curl/curl.h>
#include <arpa/inet.h>

#define MAX_BYTES 1048576  // 1 MiB

typedef struct {
    char* data;
    size_t size;
} Buffer;

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t realsize = size * nmemb;
    Buffer* buf = (Buffer*)userdata;
    if (buf->size >= MAX_BYTES) {
        return 0; // stop
    }
    size_t remaining = MAX_BYTES - buf->size;
    size_t to_copy = realsize < remaining ? realsize : remaining;

    char* newp = (char*)realloc(buf->data, buf->size + to_copy + 1);
    if (!newp) return 0;
    buf->data = newp;
    memcpy(buf->data + buf->size, ptr, to_copy);
    buf->size += to_copy;
    buf->data[buf->size] = '\0';

    if (realsize > to_copy) {
        return 0; // indicate error to stop further data
    }
    return realsize;
}

static int is_literal_ip(const char* host) {
    if (!host) return 0;
    // Handle IPv6 in [addr]
    const char* h = host;
    char tmp[256];
    memset(tmp, 0, sizeof(tmp));

    if (h[0] == '[') {
        const char* end = strchr(h, ']');
        if (!end) return 0;
        size_t len = (size_t)(end - h - 1);
        if (len >= sizeof(tmp)) return 0;
        memcpy(tmp, h + 1, len);
    } else {
        // strip port if present
        const char* colon = strchr(h, ':');
        size_t len = colon ? (size_t)(colon - h) : strlen(h);
        if (len >= sizeof(tmp)) len = sizeof(tmp) - 1;
        memcpy(tmp, h, len);
    }

    struct in_addr v4;
    struct in6_addr v6;
    if (inet_pton(AF_INET, tmp, &v4) == 1) return 1;
    if (inet_pton(AF_INET6, tmp, &v6) == 1) return 1;
    return 0;
}

static char* trim_and_collapse_ws(const char* s) {
    if (!s) return strdup("");
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return strdup("");
    size_t j = 0;
    int in_ws = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (isspace(c)) {
            if (!in_ws) {
                out[j++] = ' ';
                in_ws = 1;
            }
        } else {
            out[j++] = (char)c;
            in_ws = 0;
        }
    }
    // trim
    size_t start = 0;
    while (start < j && out[start] == ' ') start++;
    size_t end = j;
    while (end > start && out[end - 1] == ' ') end--;
    size_t len = end > start ? (end - start) : 0;
    char* res = (char*)malloc(len + 1);
    if (!res) {
        free(out);
        return strdup("");
    }
    if (len > 0) memcpy(res, out + start, len);
    res[len] = '\0';
    free(out);
    return res;
}

// Case-insensitive search for needle in haystack. Returns pointer or NULL.
static const char* ci_strstr(const char* haystack, const char* needle) {
    if (!*needle) return haystack;
    size_t nlen = strlen(needle);
    for (const char* p = haystack; *p; ++p) {
        size_t i = 0;
        while (i < nlen && p[i] && tolower((unsigned char)p[i]) == tolower((unsigned char)needle[i])) {
            i++;
        }
        if (i == nlen) return p;
    }
    return NULL;
}

static char* extract_title_from_html(const char* html) {
    if (!html) return strdup("");
    const char* startTag = ci_strstr(html, "<title");
    if (!startTag) return strdup("");
    const char* gt = strchr(startTag, '>');
    if (!gt) return strdup("");
    gt++; // move past '>'
    const char* endTag = ci_strstr(gt, "</title>");
    if (!endTag) return strdup("");
    size_t len = (size_t)(endTag - gt);
    char* raw = (char*)malloc(len + 1);
    if (!raw) return strdup("");
    memcpy(raw, gt, len);
    raw[len] = '\0';
    char* cleaned = trim_and_collapse_ws(raw);
    free(raw);
    return cleaned;
}

char* fetch_title(const char* url) {
    if (!url) {
        char* empty = (char*)calloc(1, 1);
        return empty;
    }

    CURLU* cu = curl_url();
    if (!cu) {
        char* empty = (char*)calloc(1, 1);
        return empty;
    }

    char* result = NULL;

    CURL* curl = curl_easy_init();
    if (!curl) {
        curl_url_cleanup(cu);
        char* empty = (char*)calloc(1, 1);
        return empty;
    }

    do {
        if (curl_url_set(cu, CURLUPART_URL, url, 0) != CURLUE_OK) {
            break;
        }

        char* scheme = NULL;
        if (curl_url_get(cu, CURLUPART_SCHEME, &scheme, 0) != CURLUE_OK) {
            break;
        }
        int is_https = (scheme && strcasecmp(scheme, "https") == 0);
        curl_free(scheme);
        if (!is_https) break;

        char* host = NULL;
        if (curl_url_get(cu, CURLUPART_HOST, &host, 0) != CURLUE_OK) {
            break;
        }
        if (!host || host[0] == '\0') {
            if (host) curl_free(host);
            break;
        }
        int is_localhost = (strcasecmp(host, "localhost") == 0);
        int is_ip = is_literal_ip(host);
        curl_free(host);
        if (is_localhost || is_ip) break;

        Buffer buf = {0};
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
        curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task76/1.0 (+https://example.invalid)");
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "identity");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

        CURLcode rc = curl_easy_perform(curl);
        if (rc != CURLE_OK) {
            if (buf.data) free(buf.data);
            break;
        }
        long code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        if (code != 200) {
            if (buf.data) free(buf.data);
            break;
        }

        char* title = extract_title_from_html(buf.data ? buf.data : "");
        if (buf.data) free(buf.data);
        result = title;

    } while (0);

    curl_easy_cleanup(curl);
    curl_url_cleanup(cu);

    if (!result) {
        result = (char*)calloc(1, 1);
    }
    return result;
}

int main(int argc, char** argv) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    const char* tests_default[5] = {
        "https://example.com",
        "https://httpbin.org/html",
        "https://www.iana.org/domains/reserved",
        "https://www.wikipedia.org",
        "https://www.gnu.org"
    };

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            char* t = fetch_title(argv[i]);
            printf("%s => %s\n", argv[i], t);
            free(t);
        }
    } else {
        for (int i = 0; i < 5; ++i) {
            char* t = fetch_title(tests_default[i]);
            printf("%s => %s\n", tests_default[i], t);
            free(t);
        }
    }

    curl_global_cleanup();
    return 0;
}