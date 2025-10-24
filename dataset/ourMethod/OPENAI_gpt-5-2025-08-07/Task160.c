#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>

#define MAX_URL_LEN 2048
#define MAX_BODY_BYTES (1024 * 1024) /* 1 MiB */
#define ERRBUF_SZ 256

typedef struct {
    char *data;
    size_t size;
    size_t cap;
    size_t max;
    char *errbuf;
    size_t errbuf_sz;
} Buffer;

static size_t safe_min(size_t a, size_t b) {
    return a < b ? a : b;
}

static size_t rstrip(char *s) {
    if (!s) return 0;
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' || isspace((unsigned char)s[len - 1]))) {
        s[len - 1] = '\0';
        len--;
    }
    return len;
}

static void set_err(char *err, size_t err_sz, const char *msg) {
    if (!err || err_sz == 0) return;
    if (!msg) { err[0] = '\0'; return; }
    strncpy(err, msg, err_sz - 1);
    err[err_sz - 1] = '\0';
}

static int validate_https_url(const char *input, char *normalized, size_t normalized_sz, char *err, size_t err_sz) {
    if (!input || !normalized || normalized_sz == 0) {
        set_err(err, err_sz, "Invalid parameters");
        return 0;
    }

    size_t in_len = strnlen(input, MAX_URL_LEN + 1);
    if (in_len == 0 || in_len > MAX_URL_LEN) {
        set_err(err, err_sz, "URL length invalid");
        return 0;
    }

    for (size_t i = 0; i < in_len; i++) {
        unsigned char c = (unsigned char)input[i];
        if (c < 0x20 || c == 0x7F) {
            set_err(err, err_sz, "URL contains control characters");
            return 0;
        }
    }

#if LIBCURL_VERSION_NUM >= 0x073900 /* 7.57.0 for URL API is 7.62.0; use runtime check anyway */
    CURLU *h = curl_url();
    if (!h) {
        set_err(err, err_sz, "Internal error");
        return 0;
    }

    CURLUcode cu = curl_url_set(h, CURLUPART_URL, input, 0);
    if (cu) {
        curl_url_cleanup(h);
        set_err(err, err_sz, "Malformed URL");
        return 0;
    }

    char *scheme = NULL;
    cu = curl_url_get(h, CURLUPART_SCHEME, &scheme, 0);
    if (cu || !scheme) {
        if (scheme) curl_free(scheme);
        curl_url_cleanup(h);
        set_err(err, err_sz, "URL missing scheme");
        return 0;
    }
    int ok = 1;
    if (strcmp(scheme, "https") != 0) {
        ok = 0;
        set_err(err, err_sz, "Only https scheme is allowed");
    }
    curl_free(scheme);

    char *host = NULL;
    if (ok) {
        cu = curl_url_get(h, CURLUPART_HOST, &host, 0);
        if (cu || !host || host[0] == '\0') {
            ok = 0;
            set_err(err, err_sz, "URL missing host");
        }
    }
    if (host) curl_free(host);

    if (ok) {
        char *user = NULL;
        if (curl_url_get(h, CURLUPART_USER, &user, 0) == CURLUE_OK && user) {
            ok = 0;
            set_err(err, err_sz, "Credentials in URL are not allowed");
        }
        if (user) curl_free(user);
        char *pass = NULL;
        if (curl_url_get(h, CURLUPART_PASSWORD, &pass, 0) == CURLUE_OK && pass) {
            ok = 0;
            set_err(err, err_sz, "Credentials in URL are not allowed");
        }
        if (pass) curl_free(pass);
    }

    if (ok) {
        char *port = NULL;
        if (curl_url_get(h, CURLUPART_PORT, &port, 0) == CURLUE_OK && port) {
            char *endp = NULL;
            long p = strtol(port, &endp, 10);
            if (!endp || *endp != '\0' || p < 1 || p > 65535) {
                ok = 0;
                set_err(err, err_sz, "Invalid port");
            }
        }
        if (port) curl_free(port);
    }

    if (ok) {
        char *full = NULL;
        cu = curl_url_get(h, CURLUPART_URL, &full, CURLU_URLENCODE);
        if (cu || !full) {
            ok = 0;
            set_err(err, err_sz, "Failed to normalize URL");
        } else {
            size_t flen = strlen(full);
            if (flen == 0 || flen > MAX_URL_LEN) {
                ok = 0;
                set_err(err, err_sz, "Normalized URL too long");
            } else {
                strncpy(normalized, full, normalized_sz - 1);
                normalized[normalized_sz - 1] = '\0';
            }
        }
        if (full) curl_free(full);
    }

    curl_url_cleanup(h);
    return ok ? 1 : 0;
#else
    /* Fallback basic validation if libcurl URL API unavailable */
    (void)err; (void)err_sz;
    if (strncmp(input, "https://", 8) != 0) {
        return 0;
    }
    if (strlen(input) >= normalized_sz) {
        return 0;
    }
    strcpy(normalized, input);
    return 1;
#endif
}

static size_t write_cb(char *ptr, size_t size, size_t nmemb, void *userdata) {
    Buffer *b = (Buffer *)userdata;
    if (!b || !ptr) return 0;

    size_t incoming = size * nmemb;
    if (incoming == 0) return 0;

    if (b->size >= b->max) {
        set_err(b->errbuf, b->errbuf_sz, "Response too large");
        return 0; /* abort */
    }

    size_t allowed = b->max - b->size;
    size_t to_copy = safe_min(incoming, allowed);

    size_t need = b->size + to_copy + 1; /* +1 for NUL */
    if (need > b->cap) {
        size_t newcap = b->cap ? b->cap : 16384;
        if (newcap < need) {
            while (newcap < need) {
                size_t prev = newcap;
                newcap *= 2;
                if (newcap < prev) { /* overflow */
                    newcap = need;
                    break;
                }
            }
        }
        /* Do not exceed max+1 */
        size_t maxcap = (b->max + 1);
        if (newcap > maxcap) newcap = maxcap;
        char *newdata = (char *)realloc(b->data, newcap);
        if (!newdata) {
            set_err(b->errbuf, b->errbuf_sz, "Out of memory");
            return 0; /* abort */
        }
        b->data = newdata;
        b->cap = newcap;
    }

    memcpy(b->data + b->size, ptr, to_copy);
    b->size += to_copy;
    b->data[b->size] = '\0';

    if (to_copy < incoming) {
        set_err(b->errbuf, b->errbuf_sz, "Response truncated at limit");
        return to_copy; /* triggers write error to abort */
    }

    return to_copy;
}

static char *http_get_https(const char *url, size_t max_bytes, long timeout_ms, long max_redirects, size_t *out_len, char *errbuf, size_t errbuf_sz) {
    if (out_len) *out_len = 0;
    set_err(errbuf, errbuf_sz, "");

    if (!url || max_bytes == 0) {
        set_err(errbuf, errbuf_sz, "Invalid parameters");
        return NULL;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        set_err(errbuf, errbuf_sz, "Initialization failure");
        return NULL;
    }

    Buffer b;
    b.data = NULL;
    b.size = 0;
    b.cap = 0;
    b.max = max_bytes > MAX_BODY_BYTES ? MAX_BODY_BYTES : max_bytes;
    b.errbuf = errbuf;
    b.errbuf_sz = errbuf_sz;

    char curl_err[CURL_ERROR_SIZE];
    curl_err[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, max_redirects);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureFetcher/1.0 (+https)");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &b);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curl_err);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); /* enable built-in decompression */
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    /* TLS settings */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef CURL_SSLVERSION_TLSv1_2
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
#endif

    /* Timeouts */
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, (timeout_ms > 0) ? timeout_ms : 5000L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, (timeout_ms > 0) ? (timeout_ms * 2) : 15000L);

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        if (curl_err[0] != '\0') {
            set_err(errbuf, errbuf_sz, curl_err);
        } else {
            set_err(errbuf, errbuf_sz, "Request failed");
        }
        free(b.data);
        curl_easy_cleanup(curl);
        return NULL;
    }

    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    if (code < 200 || code >= 300) {
        set_err(errbuf, errbuf_sz, "Non-2xx response");
        free(b.data);
        curl_easy_cleanup(curl);
        return NULL;
    }

    curl_easy_cleanup(curl);
    if (out_len) *out_len = b.size;
    return b.data; /* caller must free */
}

int main(void) {
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
        fprintf(stderr, "Failed to initialize network library\n");
        return 1;
    }

    const char *tests[5] = {
        "https://example.com",
        "http://example.com",              /* should be rejected (non-https) */
        "https://expired.badssl.com/",     /* should fail TLS verification */
        "https://example.com:443/",
        "https://user:pass@example.com/"   /* should be rejected (credentials) */
    };

    printf("Running 5 test cases:\n");
    for (int i = 0; i < 5; i++) {
        char norm[MAX_URL_LEN + 1];
        char verr[ERRBUF_SZ];
        if (!validate_https_url(tests[i], norm, sizeof(norm), verr, sizeof(verr))) {
            printf("Test %d: Rejected (%s)\n", i + 1, verr[0] ? verr : "invalid URL");
            continue;
        }

        char err[ERRBUF_SZ];
        size_t len = 0;
        char *body = http_get_https(norm, 100 * 1024, 8000L, 3L, &len, err, sizeof(err));
        if (!body) {
            printf("Test %d: Failed (%s)\n", i + 1, err[0] ? err : "request error");
        } else {
            size_t preview = safe_min(len, (size_t)120);
            printf("Test %d: Success, %zu bytes (preview %zu bytes):\n", i + 1, len, preview);
            if (preview > 0) {
                fwrite(body, 1, preview, stdout);
                putchar('\n');
            }
            free(body);
        }
    }

    printf("\nEnter a HTTPS URL to fetch (max %d chars):\n> ", MAX_URL_LEN);
    char input[MAX_URL_LEN + 4];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        rstrip(input);
        if (input[0] == '\0') {
            printf("No URL provided.\n");
        } else {
            char norm[MAX_URL_LEN + 1];
            char verr[ERRBUF_SZ];
            if (!validate_https_url(input, norm, sizeof(norm), verr, sizeof(verr))) {
                printf("Rejected: %s\n", verr[0] ? verr : "invalid URL");
            } else {
                char err[ERRBUF_SZ];
                size_t len = 0;
                char *body = http_get_https(norm, 256 * 1024, 10000L, 3L, &len, err, sizeof(err));
                if (!body) {
                    printf("Request failed: %s\n", err[0] ? err : "error");
                } else {
                    printf("Success, received %zu bytes. Showing first 200 bytes:\n", len);
                    size_t preview = safe_min(len, (size_t)200);
                    fwrite(body, 1, preview, stdout);
                    putchar('\n');
                    free(body);
                }
            }
        }
    } else {
        printf("Failed to read input.\n");
    }

    curl_global_cleanup();
    return 0;
}