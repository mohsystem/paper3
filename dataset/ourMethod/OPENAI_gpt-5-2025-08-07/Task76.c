/*
Build: cc -Wall -Wextra -O2 task.c -o task -lcurl
Run: ./task https://example.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_URL_LEN 2048
#define MAX_DOWNLOAD_SIZE (1024 * 1024 * 2) /* 2 MiB cap */
#define CONNECT_TIMEOUT 10L
#define TRANSFER_TIMEOUT 30L
#define MAX_TITLE_LEN (1024 * 64) /* 64 KiB cap for extracted title */

typedef struct {
    char *data;
    size_t size;
    size_t cap;
    size_t max;
    int too_large;
} DynBuffer;

static int validate_https_url(const char *url) {
    if (url == NULL) return 0;
    size_t len = strnlen(url, MAX_URL_LEN + 1);
    if (len == 0 || len > MAX_URL_LEN) return 0;
    const char *prefix = "https://";
    size_t pfx = strlen(prefix);
    if (len < pfx || strncmp(url, prefix, pfx) != 0) return 0;
    /* Basic sanitation: no spaces or control chars */
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)url[i];
        if (c < 0x20 || c == 0x7F || isspace((int)c)) return 0;
    }
    /* Ensure there is at least one host character */
    if (url[pfx] == '\0' || url[pfx] == '/' ) return 0;
    return 1;
}

static void dbuf_init(DynBuffer *b, size_t initial_cap, size_t max) {
    b->data = NULL;
    b->size = 0;
    b->cap = 0;
    b->max = max;
    b->too_large = 0;
    if (initial_cap > 0) {
        if (initial_cap > max) initial_cap = max;
        b->data = (char*)malloc(initial_cap);
        if (b->data != NULL) {
            b->cap = initial_cap;
        }
    }
}

static void dbuf_free(DynBuffer *b) {
    if (b && b->data) {
        free(b->data);
        b->data = NULL;
        b->size = 0;
        b->cap = 0;
        b->max = 0;
    }
}

static int dbuf_reserve(DynBuffer *b, size_t need) {
    if (need <= b->cap) return 1;
    if (need > b->max) return 0;
    size_t newcap = b->cap ? b->cap : 8192;
    while (newcap < need) {
        if (newcap > b->max / 2) {
            newcap = b->max;
            break;
        }
        newcap *= 2;
    }
    if (newcap > b->max) newcap = b->max;
    char *p = (char*)realloc(b->data, newcap);
    if (!p) return 0;
    b->data = p;
    b->cap = newcap;
    return 1;
}

static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    DynBuffer *b = (DynBuffer*)userp;
    if (size != 0 && nmemb > SIZE_MAX / size) {
        return 0; /* overflow risk */
    }
    size_t realsize = size * nmemb;
    if (realsize == 0) return 0;
    if (b->size > b->max) return 0;
    if (realsize > b->max - b->size) {
        b->too_large = 1;
        return 0; /* exceed cap */
    }
    size_t need = b->size + realsize + 1;
    if (!dbuf_reserve(b, need)) {
        b->too_large = 1;
        return 0;
    }
    memcpy(b->data + b->size, contents, realsize);
    b->size += realsize;
    b->data[b->size] = '\0';
    return realsize;
}

static int fetch_url_secure(const char *url, char **out_buf, size_t *out_len) {
    *out_buf = NULL;
    *out_len = 0;

    if (!validate_https_url(url)) {
        return -1; /* invalid input */
    }

    CURLcode ginit = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (ginit != CURLE_OK) return -2;

    CURL *curl = curl_easy_init();
    if (!curl) {
        curl_global_cleanup();
        return -3;
    }

    DynBuffer b;
    dbuf_init(&b, 0, MAX_DOWNLOAD_SIZE);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, (long)CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, (long)CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef CURL_SSLVERSION_TLSv1_2
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, (long)CURL_SSLVERSION_TLSv1_2);
#endif
#ifdef CURL_HTTP_VERSION_2TLS
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
#endif
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task76TitleFetcher/1.0");
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TRANSFER_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&b);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    CURLcode res = curl_easy_perform(curl);

    long http_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if (res != CURLE_OK || b.too_large || http_code < 200 || http_code >= 300) {
        dbuf_free(&b);
        return -4; /* network or HTTP error */
    }

    /* Success */
    *out_buf = b.data;
    *out_len = b.size;
    return 0;
}

static const char* ci_memmem(const char *hay, size_t hlen, const char *needle, size_t nlen) {
    if (!hay || !needle || nlen == 0 || hlen < nlen) return NULL;
    for (size_t i = 0; i + nlen <= hlen; i++) {
        size_t j = 0;
        for (; j < nlen; j++) {
            unsigned char a = (unsigned char)hay[i + j];
            unsigned char b = (unsigned char)needle[j];
            if (tolower(a) != tolower(b)) break;
        }
        if (j == nlen) return hay + i;
    }
    return NULL;
}

static size_t html_entity_decode_copy(const char *in, size_t inlen, char *out, size_t outcap) {
    size_t oi = 0;
    for (size_t i = 0; i < inlen && oi + 1 < outcap; ) {
        if (in[i] == '&') {
            size_t j = i + 1;
            size_t maxscan = 10; /* limit entity length to avoid abuse */
            while (j < inlen && j - i <= maxscan && in[j] != ';') j++;
            if (j < inlen && in[j] == ';' && j - i <= maxscan) {
                size_t elen = j - i - 1;
                const char *e = in + i + 1;
                char ch = 0;
                int decoded = 0;
                if (elen > 0) {
                    if (strncmp(e, "amp", elen) == 0) { ch = '&'; decoded = 1; }
                    else if (strncmp(e, "lt", elen) == 0) { ch = '<'; decoded = 1; }
                    else if (strncmp(e, "gt", elen) == 0) { ch = '>'; decoded = 1; }
                    else if (strncmp(e, "quot", elen) == 0) { ch = '"'; decoded = 1; }
                    else if (strncmp(e, "apos", elen) == 0) { ch = '\''; decoded = 1; }
                    else if (e[0] == '#') {
                        long code = 0;
                        if (elen > 1 && (e[1] == 'x' || e[1] == 'X')) {
                            char buf[12]; size_t k = elen - 2; if (k >= sizeof(buf)) k = sizeof(buf) - 1;
                            memcpy(buf, e + 2, k); buf[k] = '\0';
                            char *endp = NULL;
                            code = strtol(buf, &endp, 16);
                            if (endp && *endp == '\0') decoded = 1;
                        } else {
                            char buf[12]; size_t k = elen - 1; if (k >= sizeof(buf)) k = sizeof(buf) - 1;
                            memcpy(buf, e + 1, k); buf[k] = '\0';
                            char *endp = NULL;
                            code = strtol(buf, &endp, 10);
                            if (endp && *endp == '\0') decoded = 1;
                        }
                        if (decoded) {
                            if (code <= 0) { ch = '?'; }
                            else if (code < 0x80) { ch = (char)code; }
                            else { ch = '?'; } /* limit to ASCII for safety */
                        }
                    }
                }
                if (decoded) {
                    out[oi++] = ch;
                    i = j + 1;
                    continue;
                }
            }
        }
        out[oi++] = in[i++];
    }
    if (oi < outcap) out[oi] = '\0';
    return oi;
}

static void trim_ascii_whitespace(const char **start, const char **end) {
    const char *s = *start;
    const char *e = *end;
    while (s < e && isspace((unsigned char)*s)) s++;
    while (e > s && isspace((unsigned char)*(e - 1))) e--;
    *start = s;
    *end = e;
}

static char* extract_title_from_html(const char *html, size_t len) {
    if (!html || len == 0) return NULL;

    const char *open = ci_memmem(html, len, "<title", 6);
    if (!open) return NULL;

    /* find end of opening tag '>' */
    const char *p = open + 6;
    while (p < html + len && *p != '>') p++;
    if (p >= html + len || *p != '>') return NULL;
    p++; /* first char of title text */

    /* find closing tag */
    const char *close = ci_memmem(p, (size_t)(html + len - p), "</title", 7);
    if (!close) return NULL;

    const char *start = p;
    const char *end = close;
    trim_ascii_whitespace(&start, &end);
    if (end <= start) return NULL;

    size_t rawlen = (size_t)(end - start);
    if (rawlen > MAX_TITLE_LEN) rawlen = MAX_TITLE_LEN;

    /* Allocate output buffer: worst case same length + 1 */
    char *out = (char*)malloc(rawlen + 1);
    if (!out) return NULL;

    size_t outlen = html_entity_decode_copy(start, rawlen, out, rawlen + 1);
    /* Final trim after entity decoding */
    const char *ts = out;
    const char *te = out + outlen;
    trim_ascii_whitespace(&ts, &te);
    size_t finlen = (size_t)(te - ts);

    if (ts != out && finlen > 0) memmove(out, ts, finlen);
    out[finlen] = '\0';

    if (finlen == 0) {
        free(out);
        return NULL;
    }
    return out;
}

static char* fetch_title_from_https_url(const char *url) {
    char *body = NULL;
    size_t blen = 0;
    int r = fetch_url_secure(url, &body, &blen);
    if (r != 0) {
        if (body) free(body);
        return NULL;
    }
    char *title = extract_title_from_html(body, blen);
    free(body);
    return title;
}

int main(int argc, char **argv) {
    const char *tests[5] = {
        "https://example.com",
        "https://www.iana.org/domains/reserved",
        "https://www.wikipedia.org/",
        "https://www.gnu.org/",
        "https://www.rfc-editor.org/"
    };

    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            const char *url = argv[i];
            char *title = fetch_title_from_https_url(url);
            if (title) {
                printf("URL: %s\nTitle: %s\n\n", url, title);
                free(title);
            } else {
                printf("URL: %s\nError: Unable to retrieve title.\n\n", url);
            }
        }
    } else {
        /* Run 5 predefined test cases if no arguments are provided */
        for (int i = 0; i < 5; i++) {
            const char *url = tests[i];
            char *title = fetch_title_from_https_url(url);
            if (title) {
                printf("URL: %s\nTitle: %s\n\n", url, title);
                free(title);
            } else {
                printf("URL: %s\nError: Unable to retrieve title.\n\n", url);
            }
        }
    }

    return 0;
}