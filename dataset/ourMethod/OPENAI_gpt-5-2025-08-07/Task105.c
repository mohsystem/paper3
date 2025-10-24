// Secure HTTPS client in C using libcurl with strict TLS settings and hostname verification.
// Builds with: cc -Wall -Wextra -pedantic -O2 task105.c -o task105 -lcurl
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <curl/curl.h>

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

// Configuration constants
#define MAX_BODY_SIZE (5U * 1024U * 1024U) // 5 MiB cap to prevent memory exhaustion
#define CONNECT_TIMEOUT_MS 10000L          // 10s connect timeout
#define TOTAL_TIMEOUT_MS   20000L          // 20s total timeout
#define MAX_REDIRECTS      3L              // limit redirects
#define USER_AGENT_STR     "SecureTLSClient/1.0 (+https://security.example)"

typedef struct {
    unsigned char *data;
    size_t size;
    size_t cap;
    int overflowed;
} Buffer;

typedef struct {
    int success;           // 1 if cURL performed successfully and cert verified
    long http_status;      // HTTP status code if available
    unsigned char *data;   // response body (null-terminated for convenience)
    size_t size;           // size of response body
    CURLcode curl_code;    // cURL result code
} FetchResult;

static void secure_memzero(void *v, size_t n) {
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) { *p++ = 0u; }
}

static int is_https_url(const char *url) {
    if (url == NULL) return 0;
    // Enforce exact lowercase "https://" prefix for simplicity and safety.
    const char *pfx = "https://";
    size_t plen = 8u;
    size_t ulen = strnlen(url, 2048u);
    if (ulen < plen) return 0;
    return (strncmp(url, pfx, plen) == 0) ? 1 : 0;
}

static int buffer_init(Buffer *buf, size_t initial) {
    if (!buf) return 0;
    buf->data = NULL;
    buf->size = 0u;
    buf->cap = 0u;
    buf->overflowed = 0;
    if (initial == 0u) initial = 4096u;
    buf->data = (unsigned char *)malloc(initial);
    if (!buf->data) return 0;
    buf->cap = initial;
    buf->data[0] = '\0';
    return 1;
}

static void buffer_free(Buffer *buf) {
    if (!buf) return;
    if (buf->data) {
        // Not sensitive, but wipe anyway as a good practice
        secure_memzero(buf->data, buf->cap);
        free(buf->data);
        buf->data = NULL;
    }
    buf->size = 0u;
    buf->cap = 0u;
    buf->overflowed = 0;
}

static int buffer_append(Buffer *buf, const unsigned char *src, size_t n) {
    if (!buf || !src) return 0;
    if (n == 0u) return 1;
    // Cap response size
    if (buf->size >= MAX_BODY_SIZE || n > (MAX_BODY_SIZE - buf->size)) {
        buf->overflowed = 1;
        return 0;
    }
    // Ensure capacity (include room for terminating NUL)
    size_t needed = buf->size + n + 1u;
    if (needed < buf->size) return 0; // overflow check
    if (needed > buf->cap) {
        size_t newcap = buf->cap;
        while (newcap < needed) {
            if (newcap > SIZE_MAX / 2u) { newcap = needed; break; }
            newcap *= 2u;
        }
        unsigned char *tmp = (unsigned char *)realloc(buf->data, newcap);
        if (!tmp) return 0;
        buf->data = tmp;
        buf->cap = newcap;
    }
    memcpy(buf->data + buf->size, src, n);
    buf->size += n;
    buf->data[buf->size] = '\0';
    return 1;
}

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    // size*nmemb bytes available
    size_t total;
    if (size == 0u || nmemb == 0u) return 0u;
    // Prevent overflow in multiplication
    if (nmemb > SIZE_MAX / size) return 0u;
    total = size * nmemb;

    Buffer *buf = (Buffer *)userdata;
    if (!buf) return 0u;

    if (!buffer_append(buf, (const unsigned char *)ptr, total)) {
        // Signal curl to abort
        return 0u;
    }
    return total;
}

// Secure HTTPS GET using libcurl with strict verification and TLS 1.2+ requirement.
static FetchResult secure_https_get(const char *url, long connect_timeout_ms, long total_timeout_ms) {
    FetchResult res;
    res.success = 0;
    res.http_status = 0;
    res.data = NULL;
    res.size = 0u;
    res.curl_code = CURLE_FAILED_INIT;

    if (!is_https_url(url)) {
        res.curl_code = CURLE_UNSUPPORTED_PROTOCOL;
        return res;
    }

    CURLcode ginit = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (ginit != CURLE_OK) {
        res.curl_code = ginit;
        return res;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        curl_global_cleanup();
        res.curl_code = CURLE_FAILED_INIT;
        return res;
    }

    Buffer buf;
    if (!buffer_init(&buf, 8192u)) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        res.curl_code = CURLE_OUT_OF_MEMORY;
        return res;
    }

    struct curl_slist *hdrs = NULL;
    hdrs = curl_slist_append(hdrs, "Accept: text/plain, application/json;q=0.9, */*;q=0.8");
    if (!hdrs) {
        buffer_free(&buf);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        res.curl_code = CURLE_OUT_OF_MEMORY;
        return res;
    }

    char errbuf[CURL_ERROR_SIZE];
    errbuf[0] = '\0';

    if (curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hdrs) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT_STR) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIRECTS) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS, (long)CURLPROTO_HTTPS) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, (long)CURLPROTO_HTTPS) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, connect_timeout_ms) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, total_timeout_ms) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb) != CURLE_OK ||
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf) != CURLE_OK) {
        curl_slist_free_all(hdrs);
        buffer_free(&buf);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        res.curl_code = CURLE_FAILED_INIT;
        return res;
    }

#ifdef CURL_HTTP_VERSION_2TLS
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
#endif

    // Enforce TLS 1.2 minimum, allow up to TLS 1.3 if available
    long sslver = CURL_SSLVERSION_TLSv1_2;
#ifdef CURL_SSLVERSION_MAX_TLSv1_3
    sslver |= (CURL_SSLVERSION_MAX_TLSv1_3 << 16);
#endif
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, sslver);

    CURLcode cc = curl_easy_perform(curl);
    res.curl_code = cc;

    if (cc == CURLE_OK && !buf.overflowed) {
        long code = 0;
        if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code) == CURLE_OK) {
            res.http_status = code;
        }
        // Success path; return collected buffer
        res.success = 1;
        res.data = buf.data;
        res.size = buf.size;
        // Do not free buf here; ownership transferred to res
    } else {
        // Failure; cleanup buffer
        buffer_free(&buf);
        res.success = 0;
        res.data = NULL;
        res.size = 0u;
    }

    curl_slist_free_all(hdrs);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return res;
}

static void fetch_result_free(FetchResult *r) {
    if (!r) return;
    if (r->data) {
        secure_memzero(r->data, r->size);
        free(r->data);
        r->data = NULL;
    }
    r->size = 0u;
}

static void print_summary(const char *url, const FetchResult *r) {
    if (!url || !r) return;
    printf("URL: %s\n", url);
    printf("  cURL code: %d (%s)\n", r->curl_code, curl_easy_strerror(r->curl_code));
    printf("  Success: %s\n", r->success ? "yes" : "no");
    printf("  HTTP status: %ld\n", r->http_status);
    printf("  Body size: %zu bytes\n", r->size);
    if (r->success && r->data && r->size > 0u) {
        size_t preview = r->size < 200u ? r->size : 200u;
        printf("  Body preview (%zu bytes):\n", preview);
        // Print safely; data may contain binary. Print as escaped.
        for (size_t i = 0; i < preview; i++) {
            unsigned char c = r->data[i];
            if (c >= 32 && c <= 126) {
                fputc((int)c, stdout);
            } else if (c == '\n' || c == '\r' || c == '\t') {
                fputc((int)c, stdout);
            } else {
                printf("\\x%02X", (unsigned)c);
            }
        }
        fputc('\n', stdout);
    }
    printf("\n");
}

int main(void) {
    // 5 test cases (includes one that should fail hostname verification)
    const char *tests[5] = {
        "https://www.example.com/",
        "https://www.wikipedia.org/",
        "https://api.github.com/",
        "https://www.cloudflare.com/",
        "https://wrong.host.badssl.com/" // expected to fail due to hostname mismatch
    };

    for (size_t i = 0; i < 5; i++) {
        FetchResult r = secure_https_get(tests[i], CONNECT_TIMEOUT_MS, TOTAL_TIMEOUT_MS);
        print_summary(tests[i], &r);
        fetch_result_free(&r);
    }
    return 0;
}