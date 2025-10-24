#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <curl/curl.h>

#define MAX_BYTES 1048576

static bool is_disallowed_ipv4_c(const uint8_t b[4]) {
    if (b[0] == 10) return true;
    if (b[0] == 172 && (b[1] >= 16 && b[1] <= 31)) return true;
    if (b[0] == 192 && b[1] == 168) return true;
    if (b[0] == 127) return true;
    if (b[0] == 169 && b[1] == 254) return true;
    if (b[0] == 100 && (b[1] >= 64 && b[1] <= 127)) return true;
    if (b[0] == 192 && b[1] == 0 && b[2] == 0) return true;
    if (b[0] == 192 && b[1] == 0 && b[2] == 2) return true;
    if (b[0] == 198 && (b[1] == 18 || b[1] == 19)) return true;
    if (b[0] == 198 && b[1] == 51 && b[2] == 100) return true;
    if (b[0] == 203 && b[1] == 0 && b[2] == 113) return true;
    if (b[0] == 0) return true;
    if (b[0] >= 224) return true;
    return false;
}

static bool is_disallowed_ipv6_c(const uint8_t b[16]) {
    bool allZero = true;
    for (int i = 0; i < 16; ++i) if (b[i] != 0) { allZero = false; break; }
    if (allZero) return true;
    bool loop = true;
    for (int i = 0; i < 15; ++i) if (b[i] != 0) { loop = false; break; }
    if (loop && b[15] == 1) return true;
    if ((b[0] == 0xFE) && ((b[1] & 0xC0) == 0x80)) return true; // fe80::/10
    if (b[0] == 0xFF) return true; // multicast
    if ((b[0] & 0xFE) == 0xFC) return true; // fc00::/7
    if (b[0] == 0x20 && b[1] == 0x01 && b[2] == 0x0D && b[3] == 0xB8) return true; // 2001:db8::/32
    return false;
}

static bool is_public_ip_c(const struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        const struct sockaddr_in* s4 = (const struct sockaddr_in*)sa;
        uint8_t b[4];
        memcpy(b, &s4->sin_addr, 4);
        return !is_disallowed_ipv4_c(b);
    } else if (sa->sa_family == AF_INET6) {
        const struct sockaddr_in6* s6 = (const struct sockaddr_in6*)sa;
        uint8_t b[16];
        memcpy(b, &s6->sin6_addr, 16);
        return !is_disallowed_ipv6_c(b);
    }
    return false;
}

struct CBuf {
    char* data;
    size_t size;
    bool overflow;
};

static size_t write_cb_c(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t realSize = size * nmemb;
    struct CBuf* buf = (struct CBuf*)userdata;
    size_t newSize = buf->size + realSize;
    if (newSize > MAX_BYTES) {
        size_t remaining = MAX_BYTES - buf->size;
        if (remaining > 0) {
            char* tmp = (char*)realloc(buf->data, buf->size + remaining + 1);
            if (!tmp) return 0;
            buf->data = tmp;
            memcpy(buf->data + buf->size, ptr, remaining);
            buf->size += remaining;
            buf->data[buf->size] = '\0';
        }
        buf->overflow = true;
        return 0; // abort
    } else {
        char* tmp = (char*)realloc(buf->data, newSize + 1);
        if (!tmp) return 0;
        buf->data = tmp;
        memcpy(buf->data + buf->size, ptr, realSize);
        buf->size += realSize;
        buf->data[buf->size] = '\0';
        return realSize;
    }
}

char* safe_http_fetch(const char* url) {
    CURLU* h = curl_url();
    if (!h) {
        char* err = strdup("ERROR: Failed to initialize URL parser.");
        return err;
    }
    CURLUcode uc = curl_url_set(h, CURLUPART_URL, url, 0);
    if (uc) { curl_url_cleanup(h); return strdup("ERROR: Invalid URL format."); }

    char* scheme = NULL;
    char* host = NULL;
    char* port_str = NULL;
    char* user = NULL;
    char* password = NULL;

    uc = curl_url_get(h, CURLUPART_SCHEME, &scheme, 0);
    if (uc || !scheme) { curl_url_cleanup(h); return strdup("ERROR: URL must include a scheme."); }
    if (strcmp(scheme, "http") != 0 && strcmp(scheme, "https") != 0) {
        curl_free(scheme); curl_url_cleanup(h);
        return strdup("ERROR: Only http and https schemes are allowed.");
    }
    uc = curl_url_get(h, CURLUPART_USER, &user, CURLU_URLDECODE);
    if (!uc && user) { curl_free(user); curl_free(scheme); curl_url_cleanup(h); return strdup("ERROR: User info in URL is not allowed."); }
    uc = curl_url_get(h, CURLUPART_PASSWORD, &password, CURLU_URLDECODE);
    if (!uc && password) { curl_free(password); curl_free(scheme); curl_url_cleanup(h); return strdup("ERROR: User info in URL is not allowed."); }

    uc = curl_url_get(h, CURLUPART_HOST, &host, 0);
    if (uc || !host || strlen(host) == 0) { curl_free(scheme); curl_url_cleanup(h); return strdup("ERROR: URL must include a valid host."); }

    uc = curl_url_get(h, CURLUPART_PORT, &port_str, 0);
    int port = (strcmp(scheme, "http") == 0) ? 80 : 443;
    if (!uc && port_str && strlen(port_str) > 0) port = atoi(port_str);
    if (!((strcmp(scheme, "http") == 0 && port == 80) || (strcmp(scheme, "https") == 0 && port == 443))) {
        if (port_str) curl_free(port_str);
        curl_free(host); curl_free(scheme); curl_url_cleanup(h);
        return strdup("ERROR: Port not allowed. Only default ports 80 and 443 are permitted.");
    }

    // DNS validation
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    struct addrinfo* res = NULL;
    int gai = getaddrinfo(host, (port == 80 ? "80" : "443"), &hints, &res);
    if (gai != 0) {
        if (port_str) curl_free(port_str);
        curl_free(host); curl_free(scheme); curl_url_cleanup(h);
        return strdup("ERROR: Unable to resolve host.");
    }
    bool any = false;
    for (struct addrinfo* p = res; p != NULL; p = p->ai_next) {
        any = true;
        if (!is_public_ip_c(p->ai_addr)) {
            freeaddrinfo(res);
            if (port_str) curl_free(port_str);
            curl_free(host); curl_free(scheme); curl_url_cleanup(h);
            return strdup("ERROR: Resolved to a private or disallowed IP address.");
        }
    }
    freeaddrinfo(res);
    if (!any) {
        if (port_str) curl_free(port_str);
        curl_free(host); curl_free(scheme); curl_url_cleanup(h);
        return strdup("ERROR: No addresses resolved for host.");
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        if (port_str) curl_free(port_str);
        curl_free(host); curl_free(scheme); curl_url_cleanup(h);
        return strdup("ERROR: Failed to initialize HTTP client.");
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Accept-Encoding: identity");
    headers = curl_slist_append(headers, "Connection: close");
    headers = curl_slist_append(headers, "User-Agent: Task160/1.0 (+https://example.invalid)");

    struct CBuf buf;
    buf.data = NULL; buf.size = 0; buf.overflow = false;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_RANGE, "0-1048575");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_NOPROXY, "*");
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "identity");
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb_c);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    CURLcode rc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    char* out = NULL;
    if (rc != CURLE_OK) {
        const char* err = curl_easy_strerror(rc);
        size_t len = strlen(err) + 8;
        out = (char*)malloc(len);
        if (!out) out = strdup("ERROR: OOM");
        else snprintf(out, len, "ERROR: %s", err);
    } else if (http_code >= 300 && http_code < 400) {
        out = (char*)malloc(64);
        if (!out) out = strdup("ERROR: OOM");
        else snprintf(out, 64, "ERROR: HTTP %ld (redirects are disallowed)", http_code);
    } else if (http_code >= 400) {
        out = (char*)malloc(32);
        if (!out) out = strdup("ERROR: OOM");
        else snprintf(out, 32, "ERROR: HTTP %ld", http_code);
    } else {
        if (!buf.data) out = strdup("");
        else out = buf.data; // already null-terminated
        buf.data = NULL; // transfer ownership
    }

    if (buf.data) free(buf.data);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (port_str) curl_free(port_str);
    curl_free(host);
    curl_free(scheme);
    curl_url_cleanup(h);
    return out;
}

int main(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    const char* tests[5] = {
        "https://example.com",
        "http://example.com",
        "http://127.0.0.1",
        "http://169.254.169.254/latest/meta-data/",
        "https://example.com:443"
    };
    for (int i = 0; i < 5; ++i) {
        printf("URL: %s\n", tests[i]);
        char* res = safe_http_fetch(tests[i]);
        if (!res) {
            printf("Result: (null)\n");
        } else {
            size_t len = strlen(res);
            if (len > 200) {
                char tmp = res[200];
                res[200] = '\0';
                printf("Result (first 200 chars):\n%s\n", res);
                res[200] = tmp;
            } else {
                printf("Result (first 200 chars):\n%s\n", res);
            }
            free(res);
        }
        printf("----\n");
    }
    curl_global_cleanup();
    return 0;
}