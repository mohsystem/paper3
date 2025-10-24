#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <curl/curl.h>

/*
Chain-of-Through process:
1) Problem understanding:
   - Download HTML from HTTPS and return the page <title>.
2) Security requirements:
   - Use HTTPS only; restrict redirects to HTTPS.
   - Apply timeouts and cap download size.
3) Secure coding generation:
   - Use libcurl with protocol restrictions and size-limited write callback.
4) Code review:
   - Checked for safe memory handling and input validation.
5) Secure code output:
   - Final code includes mitigations and robust title extraction.
*/

#define MAX_BYTES (1048576) // 1 MB

typedef struct {
    char* data;
    size_t size;
    int overflow;
} Buffer;

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total = size * nmemb;
    Buffer* buf = (Buffer*)userdata;
    if (buf->overflow) return 0;
    size_t space = (buf->size < MAX_BYTES) ? (MAX_BYTES - buf->size) : 0;
    size_t to_copy = total < space ? total : space;
    if (to_copy > 0) {
        memcpy(buf->data + buf->size, ptr, to_copy);
        buf->size += to_copy;
    }
    if (total > to_copy) {
        buf->overflow = 1;
        return 0; // abort transfer
    }
    return total;
}

static int starts_with_https_ci(const char* url) {
    const char* p = "https://";
    for (int i = 0; p[i] && url[i]; ++i) {
        if (tolower((unsigned char)url[i]) != p[i]) return 0;
    }
    return 1;
}

static char* strcasestr_simple(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    size_t nlen = strlen(needle);
    for (const char* p = haystack; *p; ++p) {
        size_t i = 0;
        while (i < nlen && p[i] && tolower((unsigned char)p[i]) == tolower((unsigned char)needle[i])) {
            i++;
        }
        if (i == nlen) return (char*)p;
        if (!p[i]) break;
    }
    return NULL;
}

// Extract title: find "<title", then next '>', then find "</title>" (case-insensitive)
// Returns newly allocated string (caller must free), or NULL if not found
static char* extract_title(const char* html, size_t len) {
    (void)len;
    const char* p = strcasestr_simple(html, "<title");
    if (!p) return NULL;
    const char* gt = strchr(p, '>');
    if (!gt) return NULL;
    const char* start = gt + 1;
    const char* close = strcasestr_simple(start, "</title>");
    if (!close || close <= start) return NULL;
    size_t tlen = (size_t)(close - start);
    char* out = (char*)malloc(tlen + 1);
    if (!out) return NULL;
    memcpy(out, start, tlen);
    out[tlen] = '\0';
    // collapse whitespace and trim
    size_t w = 0;
    int in_space = 0;
    for (size_t r = 0; r < tlen; ++r) {
        unsigned char ch = (unsigned char)out[r];
        if (isspace(ch)) {
            if (!in_space) {
                out[w++] = ' ';
                in_space = 1;
            }
        } else {
            out[w++] = (char)ch;
            in_space = 0;
        }
    }
    // trim
    size_t starti = 0;
    while (starti < w && out[starti] == ' ') starti++;
    size_t endi = w;
    while (endi > starti && out[endi - 1] == ' ') endi--;
    size_t final_len = endi > starti ? (endi - starti) : 0;
    char* trimmed = (char*)malloc(final_len + 1);
    if (!trimmed) { free(out); return NULL; }
    if (final_len > 0) memcpy(trimmed, out + starti, final_len);
    trimmed[final_len] = '\0';
    free(out);
    return trimmed;
}

char* get_page_title(const char* url) {
    if (!url || !starts_with_https_ci(url)) {
        char* empty = (char*)malloc(1); if (empty) empty[0] = '\0'; return empty;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        char* empty = (char*)malloc(1); if (empty) empty[0] = '\0'; return empty;
    }

    char* bufmem = (char*)malloc(MAX_BYTES + 1);
    if (!bufmem) {
        curl_easy_cleanup(curl);
        char* empty = (char*)malloc(1); if (empty) empty[0] = '\0'; return empty;
    }
    Buffer buf = { bufmem, 0, 0 };

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task76-Client/1.0 (+https)");
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); // enable compression
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || buf.size == 0) {
        free(bufmem);
        char* empty = (char*)malloc(1); if (empty) empty[0] = '\0'; return empty;
    }
    buf.data[buf.size] = '\0';
    char* title = extract_title(buf.data, buf.size);
    free(bufmem);
    if (!title) {
        char* empty = (char*)malloc(1); if (empty) empty[0] = '\0'; return empty;
    }
    return title;
}

int main(int argc, char** argv) {
    const char* tests[5] = {
        "https://example.com",
        "https://www.wikipedia.org/",
        "https://www.mozilla.org/en-US/",
        "https://www.bbc.com/",
        "https://www.oracle.com/"
    };
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            char* t = get_page_title(argv[i]);
            printf("%s -> %s\n", argv[i], t);
            free(t);
        }
    } else {
        for (int i = 0; i < 5; ++i) {
            char* t = get_page_title(tests[i]);
            printf("%s -> %s\n", tests[i], t);
            free(t);
        }
    }
    return 0;
}