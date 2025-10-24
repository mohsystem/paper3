#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <curl/curl.h>

static const size_t MAX_BYTES = 1048576; // 1 MiB

static bool is_disallowed_ipv4(const uint8_t b[4]) {
    if (b[0] == 10) return true; // 10.0.0.0/8
    if (b[0] == 172 && (b[1] >= 16 && b[1] <= 31)) return true; // 172.16.0.0/12
    if (b[0] == 192 && b[1] == 168) return true; // 192.168.0.0/16
    if (b[0] == 127) return true; // loopback
    if (b[0] == 169 && b[1] == 254) return true; // link-local
    if (b[0] == 100 && (b[1] >= 64 && b[1] <= 127)) return true; // 100.64.0.0/10
    if (b[0] == 192 && b[1] == 0 && b[2] == 0) return true; // 192.0.0.0/24
    if (b[0] == 192 && b[1] == 0 && b[2] == 2) return true; // 192.0.2.0/24
    if (b[0] == 198 && (b[1] == 18 || b[1] == 19)) return true; // 198.18.0.0/15
    if (b[0] == 198 && b[1] == 51 && b[2] == 100) return true; // 198.51.100.0/24
    if (b[0] == 203 && b[1] == 0 && b[2] == 113) return true; // 203.0.113.0/24
    if (b[0] == 0) return true; // 0.0.0.0/8
    if (b[0] >= 224) return true; // multicast and above
    return false;
}

static bool is_disallowed_ipv6(const uint8_t b[16]) {
    // any local (::)
    bool allZero = true;
    for (int i = 0; i < 16; ++i) if (b[i] != 0) { allZero = false; break; }
    if (allZero) return true;
    // loopback ::1
    bool loop = true;
    for (int i = 0; i < 15; ++i) if (b[i] != 0) { loop = false; break; }
    if (loop && b[15] == 1) return true;
    // link-local fe80::/10
    if ((b[0] == 0xFE) && ((b[1] & 0xC0) == 0x80)) return true;
    // multicast ff00::/8
    if (b[0] == 0xFF) return true;
    // unique local fc00::/7
    if ((b[0] & 0xFE) == 0xFC) return true;
    // documentation 2001:db8::/32
    if (b[0] == 0x20 && b[1] == 0x01 && b[2] == 0x0D && b[3] == 0xB8) return true;
    return false;
}

static bool is_public_ip(const struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        const struct sockaddr_in* s4 = (const struct sockaddr_in*)sa;
        uint8_t b[4];
        std::memcpy(b, &s4->sin_addr, 4);
        return !is_disallowed_ipv4(b);
    } else if (sa->sa_family == AF_INET6) {
        const struct sockaddr_in6* s6 = (const struct sockaddr_in6*)sa;
        uint8_t b[16];
        std::memcpy(b, &s6->sin6_addr, 16);
        return !is_disallowed_ipv6(b);
    }
    return false;
}

struct CurlBuffer {
    std::string data;
    bool overflow = false;
};

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t realSize = size * nmemb;
    CurlBuffer* buf = static_cast<CurlBuffer*>(userdata);
    if (buf->data.size() + realSize > MAX_BYTES) {
        size_t remaining = MAX_BYTES - buf->data.size();
        buf->data.append(ptr, remaining);
        buf->overflow = true;
        return 0; // abort transfer
    }
    buf->data.append(ptr, realSize);
    return realSize;
}

static std::string validate_and_fetch_with_curl(const std::string& url) {
    std::string result;

    CURLU* h = curl_url();
    if (!h) return "ERROR: Failed to initialize URL parser.";
    CURLUcode uc;

    uc = curl_url_set(h, CURLUPART_URL, url.c_str(), 0);
    if (uc) { curl_url_cleanup(h); return "ERROR: Invalid URL format."; }

    char* scheme = nullptr;
    char* host = nullptr;
    char* port_str = nullptr;
    char* user = nullptr;
    char* password = nullptr;

    uc = curl_url_get(h, CURLUPART_SCHEME, &scheme, 0);
    if (uc || !scheme) { curl_url_cleanup(h); return "ERROR: URL must include a scheme."; }
    std::string scheme_s(scheme);

    if (!(scheme_s == "http" || scheme_s == "https")) {
        curl_free(scheme);
        curl_url_cleanup(h);
        return "ERROR: Only http and https schemes are allowed.";
    }

    uc = curl_url_get(h, CURLUPART_USER, &user, CURLU_URLDECODE);
    if (!uc && user) { curl_free(user); curl_free(scheme); curl_url_cleanup(h); return "ERROR: User info in URL is not allowed."; }
    uc = curl_url_get(h, CURLUPART_PASSWORD, &password, CURLU_URLDECODE);
    if (!uc && password) { curl_free(password); curl_free(scheme); curl_url_cleanup(h); return "ERROR: User info in URL is not allowed."; }

    uc = curl_url_get(h, CURLUPART_HOST, &host, 0);
    if (uc || !host || std::strlen(host) == 0) { curl_free(scheme); curl_url_cleanup(h); return "ERROR: URL must include a valid host."; }

    uc = curl_url_get(h, CURLUPART_PORT, &port_str, 0);
    int port = (scheme_s == "http") ? 80 : 443;
    if (!uc && port_str && std::strlen(port_str) > 0) {
        port = std::atoi(port_str);
    }
    if (!((scheme_s == "http" && (port == 80)) || (scheme_s == "https" && (port == 443)))) {
        if (port_str) curl_free(port_str);
        curl_free(host); curl_free(scheme); curl_url_cleanup(h);
        return "ERROR: Port not allowed. Only default ports 80 and 443 are permitted.";
    }

    // DNS resolution and IP validation
    struct addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    struct addrinfo* res = nullptr;
    int gai = getaddrinfo(host, (port == 80 ? "80" : "443"), &hints, &res);
    if (gai != 0) {
        if (port_str) curl_free(port_str);
        curl_free(host); curl_free(scheme); curl_url_cleanup(h);
        return "ERROR: Unable to resolve host.";
    }
    bool any = false;
    for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
        any = true;
        if (!is_public_ip(p->ai_addr)) {
            freeaddrinfo(res);
            if (port_str) curl_free(port_str);
            curl_free(host); curl_free(scheme); curl_url_cleanup(h);
            return "ERROR: Resolved to a private or disallowed IP address.";
        }
    }
    freeaddrinfo(res);
    if (!any) {
        if (port_str) curl_free(port_str);
        curl_free(host); curl_free(scheme); curl_url_cleanup(h);
        return "ERROR: No addresses resolved for host.";
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        if (port_str) curl_free(port_str);
        curl_free(host); curl_free(scheme); curl_url_cleanup(h);
        return "ERROR: Failed to initialize HTTP client.";
    }

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept-Encoding: identity");
    headers = curl_slist_append(headers, "Connection: close");
    headers = curl_slist_append(headers, "User-Agent: Task160/1.0 (+https://example.invalid)");

    CurlBuffer buf;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);

    CURLcode rc = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (rc != CURLE_OK) {
        result = std::string("ERROR: ") + curl_easy_strerror(rc);
    } else if (http_code >= 300 && http_code < 400) {
        result = "ERROR: HTTP " + std::to_string(http_code) + " (redirects are disallowed)";
    } else if (http_code >= 400) {
        result = "ERROR: HTTP " + std::to_string(http_code);
    } else {
        result = buf.data;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    if (port_str) curl_free(port_str);
    curl_free(host);
    curl_free(scheme);
    curl_url_cleanup(h);
    return result;
}

std::string safe_http_fetch(const std::string& url) {
    static bool curl_inited = false;
    if (!curl_inited) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_inited = true;
    }
    return validate_and_fetch_with_curl(url);
}

int main() {
    std::vector<std::string> tests = {
        "https://example.com",
        "http://example.com",
        "http://127.0.0.1",
        "http://169.254.169.254/latest/meta-data/",
        "https://example.com:443"
    };
    for (const auto& t : tests) {
        std::cout << "URL: " << t << "\n";
        std::string res = safe_http_fetch(t);
        std::cout << "Result (first 200 chars):\n";
        if (res.size() > 200) std::cout << res.substr(0, 200) << "\n";
        else std::cout << res << "\n";
        std::cout << "----\n";
    }
    return 0;
}