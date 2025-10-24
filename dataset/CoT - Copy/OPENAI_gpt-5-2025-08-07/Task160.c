/* Chain-of-Through Process Implementation in C (using libcurl)
 * 1) Problem understanding: Function accepts a URL and performs a safe HTTP GET request, returning a summary string.
 * 2) Security requirements: Allow only http/https, validate DNS resolves to public IPs, enforce timeouts, limit response size, restrict and validate redirects, restrict protocols.
 * 3) Secure coding generation: Use libcurl with protocol restrictions, manual redirect handling, size cap, and IP validation.
 * 4) Code review: Blocks SSRF, sets safe UA, timeouts, data limits, validates each redirect target, no auto-follow to unknown schemes.
 * 5) Secure code output: Implemented with applied mitigations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <curl/curl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_REDIRECTS 3L
#define CONNECT_TIMEOUT_MS 5000L
#define TOTAL_TIMEOUT_MS 15000L
#define MAX_BYTES (1024*1024) /* 1MB */
#define USER_AGENT "Task160-HTTPClient/1.0 (+https://example.com/security)"

typedef struct {
    char* data;
    size_t size;
    size_t cap;
    size_t max_bytes;
    int truncated;
    char* last_location;
} FetchCtx;

static void ctx_init(FetchCtx* ctx, size_t max_bytes) {
    ctx->data = (char*)malloc(8192);
    ctx->cap = ctx->data ? 8192 : 0;
    ctx->size = 0;
    ctx->max_bytes = max_bytes;
    ctx->truncated = 0;
    ctx->last_location = NULL;
}

static void ctx_free(FetchCtx* ctx) {
    if (ctx->data) free(ctx->data);
    if (ctx->last_location) free(ctx->last_location);
    ctx->data = NULL;
    ctx->last_location = NULL;
    ctx->cap = ctx->size = 0;
}

static size_t write_body_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total = size * nmemb;
    FetchCtx* ctx = (FetchCtx*)userdata;
    if (ctx->size >= ctx->max_bytes) {
        ctx->truncated = 1;
        return 0; /* stop further writes */
    }
    size_t allowed = ctx->max_bytes - ctx->size;
    size_t to_copy = total < allowed ? total : allowed;
    if (ctx->size + to_copy > ctx->cap) {
        size_t newcap = ctx->cap ? ctx->cap : 8192;
        while (ctx->size + to_copy > newcap) newcap *= 2;
        char* nd = (char*)realloc(ctx->data, newcap);
        if (!nd) return 0;
        ctx->data = nd;
        ctx->cap = newcap;
    }
    memcpy(ctx->data + ctx->size, ptr, to_copy);
    ctx->size += to_copy;
    if (to_copy < total) {
        ctx->truncated = 1;
        return to_copy;
    }
    return total;
}

static size_t header_cb(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t total = size * nitems;
    FetchCtx* ctx = (FetchCtx*)userdata;
    /* Extract Location header */
    if (total >= 9) {
        char* p = buffer;
        size_t n = total;
        /* case-insensitive check for "Location:" */
        const char* key = "location:";
        int match = 1;
        for (size_t i = 0; i < 9 && i < n; i++) {
            if (tolower((unsigned char)p[i]) != key[i]) { match = 0; break; }
        }
        if (match) {
            /* skip "Location:" */
            size_t i = 9;
            while (i < n && (p[i] == ' ' || p[i] == '\t')) i++;
            size_t start = i;
            /* trim end CRLF */
            size_t end = n;
            while (end > start && (p[end-1] == '\r' || p[end-1] == '\n' || p[end-1]==' ' || p[end-1]=='\t')) end--;
            size_t len = end > start ? (end - start) : 0;
            if (len > 0) {
                if (ctx->last_location) { free(ctx->last_location); ctx->last_location = NULL; }
                ctx->last_location = (char*)malloc(len + 1);
                if (ctx->last_location) {
                    memcpy(ctx->last_location, p + start, len);
                    ctx->last_location[len] = '\0';
                }
            }
        }
    }
    return total;
}

/* Validate resolved IPs are public */
static int is_public_ipv4(uint32_t ip_host) {
    if ((ip_host & 0xFF000000u) == 0x0A000000u) return 0;      /* 10/8 */
    if ((ip_host & 0xFFF00000u) == 0xAC100000u) return 0;      /* 172.16/12 */
    if ((ip_host & 0xFFFF0000u) == 0xC0A80000u) return 0;      /* 192.168/16 */
    if ((ip_host & 0xFF000000u) == 0x7F000000u) return 0;      /* 127/8 */
    if ((ip_host & 0xFFFF0000u) == 0xA9FE0000u) return 0;      /* 169.254/16 */
    if ((ip_host & 0xFF000000u) == 0x00000000u) return 0;      /* 0.0.0.0/8 */
    if ((ip_host & 0xF0000000u) == 0xE0000000u) return 0;      /* 224/4 multicast */
    if ((ip_host & 0xF0000000u) == 0xF0000000u) return 0;      /* 240/4 reserved */
    if ((ip_host & 0xFFC00000u) == 0x64400000u) return 0;      /* 100.64/10 CGNAT */
    return 1;
}

static int is_public_ipv6(const struct in6_addr* a6) {
    const unsigned char* b = a6->s6_addr;
    int all_zero = 1;
    for (int i=0;i<16;i++) if (b[i]!=0) { all_zero=0; break; }
    if (all_zero) return 0; /* :: */
    int loopback = 1;
    for (int i=0;i<15;i++) if (b[i]!=0) { loopback=0; break; }
    if (loopback && b[15]==1) return 0; /* ::1 */
    if ((b[0]==0xFE) && ((b[1] & 0xC0)==0x80)) return 0; /* fe80::/10 */
    if ((b[0] & 0xFE) == 0xFC) return 0; /* fc00::/7 */
    if (b[0]==0xFF) return 0; /* ff00::/8 */
    return 1;
}

static void parse_url(const char* url, char** scheme, char** host, int* port, char** path) {
    CURLU* u = curl_url();
    if (!u) { *scheme=*host=*path=NULL; *port=-1; return; }
    if (curl_url_set(u, CURLUPART_URL, url, 0)) {
        curl_url_cleanup(u); *scheme=*host=*path=NULL; *port=-1; return;
    }
    char* s=NULL; char* h=NULL; char* p=NULL; char* pa=NULL;
    if (curl_url_get(u, CURLUPART_SCHEME, &s, 0)) s=NULL;
    if (curl_url_get(u, CURLUPART_HOST, &h, 0)) h=NULL;
    if (curl_url_get(u, CURLUPART_PATH, &pa, 0)) pa=NULL;
    long lp=0;
    if (!curl_url_get(u, CURLUPART_PORT, &p, 0) && p) {
        lp = strtol(p, NULL, 10);
    } else {
        lp = -1;
    }
    *scheme = s;
    *host = h;
    *path = pa;
    *port = (lp > 0 && lp <= 65535) ? (int)lp : -1;
    if (p) curl_free(p);
    curl_url_cleanup(u);
}

static void validate_public_resolution(const char* scheme, const char* host, int port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    char portStr[12];
    snprintf(portStr, sizeof(portStr), "%d", (port > 0) ? port : (strcmp(scheme,"http")==0 ? 80 : 443));
    struct addrinfo* res = NULL;
    int rc = getaddrinfo(host, portStr, &hints, &res);
    if (rc != 0 || !res) {
        if (res) freeaddrinfo(res);
        fprintf(stderr, "Resolution failed for host\n");
        /* Use error via exception-like return */
        exit(1);
    }
    int any = 0;
    for (struct addrinfo* p = res; p; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            struct sockaddr_in* sa = (struct sockaddr_in*)p->ai_addr;
            uint32_t ip = ntohl(sa->sin_addr.s_addr);
            any = 1;
            if (!is_public_ipv4(ip)) {
                freeaddrinfo(res);
                fprintf(stderr, "Blocked non-public IPv4 resolution\n");
                exit(1);
            }
        } else if (p->ai_family == AF_INET6) {
            struct sockaddr_in6* sa6 = (struct sockaddr_in6*)p->ai_addr;
            any = 1;
            if (!is_public_ipv6(&sa6->sin6_addr)) {
                freeaddrinfo(res);
                fprintf(stderr, "Blocked non-public IPv6 resolution\n");
                exit(1);
            }
        }
    }
    if (!any) {
        freeaddrinfo(res);
        fprintf(stderr, "No addresses resolved\n");
        exit(1);
    }
    freeaddrinfo(res);
}

static char* str_dup(const char* s) {
    size_t n = strlen(s);
    char* d = (char*)malloc(n+1);
    if (!d) return NULL;
    memcpy(d, s, n+1);
    return d;
}

static char* resolve_redirect(const char* base, const char* location) {
    if (strncmp(location, "http://", 7) == 0 || strncmp(location, "https://", 8) == 0) {
        return str_dup(location);
    }
    /* parse base */
    char *scheme=NULL, *host=NULL, *path=NULL;
    int port=-1;
    parse_url(base, &scheme, &host, &port, &path);
    if (!scheme || !host) {
        if (scheme) curl_free(scheme);
        if (host) curl_free(host);
        if (path) curl_free(path);
        return NULL;
    }
    size_t bufsz = strlen(scheme)+3+strlen(host)+8 + strlen(location) + (path?strlen(path):1) + 8;
    char* out = (char*)malloc(bufsz);
    if (!out) {
        if (scheme) curl_free(scheme);
        if (host) curl_free(host);
        if (path) curl_free(path);
        return NULL;
    }
    char portpart[16] = "";
    if (!((strcmp(scheme,"http")==0 && port==80) || (strcmp(scheme,"https")==0 && port==443)) && port>0) {
        snprintf(portpart, sizeof(portpart), ":%d", port);
    }
    if (location[0] == '/') {
        snprintf(out, bufsz, "%s://%s%s%s", scheme, host, portpart, location);
    } else {
        const char* basepath = (path && path[0]) ? path : "/";
        const char* slash = strrchr(basepath, '/');
        size_t dirlen = slash ? (size_t)(slash - basepath + 1) : 1;
        char* dir = (char*)malloc(dirlen + 1);
        if (!dir) {
            free(out);
            if (scheme) curl_free(scheme);
            if (host) curl_free(host);
            if (path) curl_free(path);
            return NULL;
        }
        memcpy(dir, basepath, dirlen);
        dir[dirlen] = '\0';
        snprintf(out, bufsz, "%s://%s%s%s%s", scheme, host, portpart, dir, location);
        free(dir);
    }
    if (scheme) curl_free(scheme);
    if (host) curl_free(host);
    if (path) curl_free(path);
    return out;
}

/* Return a newly malloc'ed string containing the result summary, caller must free */
char* fetch_url(const char* input_url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        char* e = str_dup("ERROR: Failed to init curl");
        return e;
    }
    char* url = str_dup(input_url);
    if (!url) {
        curl_easy_cleanup(curl);
        return str_dup("ERROR: OOM");
    }

    char* trace = (char*)malloc(1); size_t trace_len = 0; size_t trace_cap = 1;
    if (!trace) { free(url); curl_easy_cleanup(curl); return str_dup("ERROR: OOM"); }
    trace[0] = '\0';

    long redirects = 0;
    char* result = NULL;

    for (;;) {
        /* Validate url */
        char *scheme=NULL, *host=NULL, *path=NULL;
        int port=-1;
        parse_url(url, &scheme, &host, &port, &path);
        if (!scheme || !host) {
            if (scheme) curl_free(scheme);
            if (host) curl_free(host);
            if (path) curl_free(path);
            free(url);
            curl_easy_cleanup(curl);
            return str_dup("ERROR: Invalid URL");
        }
        for (char* p = scheme; *p; ++p) *p = (char)tolower((unsigned char)*p);
        if (strcmp(scheme, "http") != 0 && strcmp(scheme, "https") != 0) {
            curl_free(scheme); curl_free(host); if (path) curl_free(path);
            free(url); curl_easy_cleanup(curl);
            return str_dup("ERROR: Only http/https allowed");
        }
        if (port == 0) {
            curl_free(scheme); curl_free(host); if (path) curl_free(path);
            free(url); curl_easy_cleanup(curl);
            return str_dup("ERROR: Invalid port");
        }
        /* Validate resolution */
        validate_public_resolution(scheme, host, port);
        curl_free(scheme); curl_free(host); if (path) curl_free(path);

        FetchCtx ctx; ctx_init(&ctx, MAX_BYTES);
        curl_easy_reset(curl);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
        curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, CONNECT_TIMEOUT_MS);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, TOTAL_TIMEOUT_MS);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_body_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_cb);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ctx);

        CURLcode res = curl_easy_perform(curl);
        long code = 0; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        char* ct = NULL; curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);

        /* Append to trace */
        const char* trace_prefix = "URL: ";
        size_t add_len = strlen(trace_prefix) + strlen(url) + 20;
        if (trace_len + add_len + 2 > trace_cap) {
            size_t newcap = (trace_cap * 2) + add_len + 64;
            char* nt = (char*)realloc(trace, newcap);
            if (nt) { trace = nt; trace_cap = newcap; }
        }
        if (trace_len + add_len + 2 <= trace_cap) {
            trace_len += snprintf(trace + trace_len, trace_cap - trace_len, "URL: %s -> Status: %ld\n", url, code);
        }

        if (code >= 300 && code < 400 && ctx.last_location && redirects < MAX_REDIRECTS) {
            char* next = resolve_redirect(url, ctx.last_location);
            if (!next) {
                ctx_free(&ctx);
                break;
            }
            /* Validate next before following */
            char *scheme2=NULL, *host2=NULL, *path2=NULL; int port2=-1;
            parse_url(next, &scheme2, &host2, &port2, &path2);
            if (!scheme2 || !host2) {
                if (scheme2) curl_free(scheme2);
                if (host2) curl_free(host2);
                if (path2) curl_free(path2);
                free(next);
                ctx_free(&ctx);
                break;
            }
            for (char* p = scheme2; *p; ++p) *p = (char)tolower((unsigned char)*p);
            if (strcmp(scheme2, "http") != 0 && strcmp(scheme2, "https") != 0 || port2 == 0) {
                curl_free(scheme2); curl_free(host2); if (path2) curl_free(path2);
                free(next);
                ctx_free(&ctx);
                break;
            }
            validate_public_resolution(scheme2, host2, port2);
            curl_free(scheme2); curl_free(host2); if (path2) curl_free(path2);

            free(url);
            url = next;
            redirects++;
            ctx_free(&ctx);
            continue;
        }

        /* Build result string */
        const char* content_type = ct ? ct : "unknown";
        const char* final_url = url;
        /* Preview first 200 bytes; ensure null-terminated copy */
        size_t preview_len = ctx.size < 200 ? ctx.size : 200;
        char* preview = (char*)malloc(preview_len + 4);
        if (!preview) preview = str_dup("");
        else {
            memcpy(preview, ctx.data, preview_len);
            if (ctx.size > 200) {
                memcpy(preview + preview_len, "...", 3);
                preview_len += 3;
            }
            preview[preview_len] = '\0';
        }

        /* Estimate result size and allocate */
        size_t res_sz = strlen(final_url) + strlen(content_type) + strlen(trace) + strlen(preview) + 256;
        result = (char*)malloc(res_sz);
        if (!result) result = str_dup("ERROR: OOM");
        else {
            snprintf(result, res_sz,
                     "Final URL: %s\nHTTP Status: %ld\nContent-Type: %s\nBytes Read (capped): %zu\nRedirects Followed: %ld\nTrace:\n%s\nPreview (first 200 bytes, may be truncated):\n%s",
                     final_url, code, content_type, ctx.size, redirects, trace, preview);
        }

        free(preview);
        ctx_free(&ctx);
        break;
    }

    free(url);
    free(trace);
    curl_easy_cleanup(curl);
    if (!result) return str_dup("ERROR: Unknown");
    return result;
}

int main(void) {
    const char* tests[5] = {
        "https://example.com",
        "http://httpbin.org/get",
        "https://httpbin.org/redirect/1",
        "ftp://example.com",
        "http://127.0.0.1"
    };
    for (int i = 0; i < 5; ++i) {
        printf("=== Test: %s ===\n", tests[i]);
        char* res = fetch_url(tests[i]);
        printf("%s\n\n", res);
        free(res);
    }
    return 0;
}