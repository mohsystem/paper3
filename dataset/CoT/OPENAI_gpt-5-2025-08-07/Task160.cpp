// Chain-of-Through Process Implementation in C++ (using libcurl)
// 1) Problem understanding: Function accepts a URL and performs a safe HTTP GET request, returning a summary.
// 2) Security requirements: Allow only http/https, validate DNS resolves to public IPs, enforce timeouts, limit response size, restrict and validate redirects, restrict protocols.
// 3) Secure coding generation: Use libcurl with protocol restrictions, manual redirect handling, size cap, and IP validation.
// 4) Code review: Blocks SSRF, sets safe UA, timeouts, limits data, validates each redirect target, no auto-follow to unknown schemes.
// 5) Secure code output: Finalized implementation.

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <curl/curl.h>

static const long MAX_REDIRECTS = 3;
static const long CONNECT_TIMEOUT_MS = 5000;
static const long TOTAL_TIMEOUT_MS = 15000;
static const size_t MAX_BYTES = 1024 * 1024; // 1MB
static const char* USER_AGENT = "Task160-HTTPClient/1.0 (+https://example.com/security)";

static inline std::string ltrim(const std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;
    return s.substr(i);
}
static inline std::string rtrim(const std::string& s) {
    if (s.empty()) return s;
    size_t i = s.size();
    while (i > 0 && std::isspace(static_cast<unsigned char>(s[i-1]))) i--;
    return s.substr(0, i);
}
static inline std::string trim(const std::string& s) { return rtrim(ltrim(s)); }

static bool is_public_ipv4(uint32_t ip_host_order) {
    // ip_host_order is in host byte order
    // Private: 10/8
    if ((ip_host_order & 0xFF000000u) == 0x0A000000u) return false;
    // 172.16/12
    if ((ip_host_order & 0xFFF00000u) == 0xAC100000u) return false;
    // 192.168/16
    if ((ip_host_order & 0xFFFF0000u) == 0xC0A80000u) return false;
    // 127/8 loopback
    if ((ip_host_order & 0xFF000000u) == 0x7F000000u) return false;
    // 169.254/16 link-local
    if ((ip_host_order & 0xFFFF0000u) == 0xA9FE0000u) return false;
    // 0.0.0.0/8 unspecified
    if ((ip_host_order & 0xFF000000u) == 0x00000000u) return false;
    // 224.0.0.0/4 multicast
    if ((ip_host_order & 0xF0000000u) == 0xE0000000u) return false;
    // 240.0.0.0/4 reserved
    if ((ip_host_order & 0xF0000000u) == 0xF0000000u) return false;
    // 100.64.0.0/10 CGNAT
    if ((ip_host_order & 0xFFC00000u) == 0x64400000u) return false;
    return true;
}

static bool is_public_ipv6(const struct in6_addr& addr6) {
    const unsigned char* b = addr6.s6_addr;
    // ::/128 unspecified
    bool all_zero = true;
    for (int i=0;i<16;i++) if (b[i]!=0) { all_zero=false; break; }
    if (all_zero) return false;
    // ::1/128 loopback
    bool loopback = true;
    for (int i=0;i<15;i++) if (b[i]!=0) { loopback=false; break; }
    if (loopback && b[15]==1) return false;
    // fe80::/10 link-local
    if ((b[0] == 0xFE) && ((b[1] & 0xC0) == 0x80)) return false;
    // fc00::/7 unique local
    if ((b[0] & 0xFE) == 0xFC) return false;
    // ff00::/8 multicast
    if (b[0] == 0xFF) return false;
    return true;
}

static void validate_public_resolution(const std::string& scheme, const std::string& host, int port) {
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    std::string portStr = std::to_string(port > 0 ? port : (scheme == "http" ? 80 : 443));
    struct addrinfo* res = nullptr;
    int rc = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res);
    if (rc != 0 || !res) {
        throw std::runtime_error("Unable to resolve host");
    }
    std::set<std::string> ips;
    for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            struct sockaddr_in* sa = (struct sockaddr_in*)p->ai_addr;
            uint32_t ip = ntohl(sa->sin_addr.s_addr);
            char buf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &sa->sin_addr, buf, sizeof(buf));
            ips.insert(buf);
            if (!is_public_ipv4(ip)) {
                freeaddrinfo(res);
                throw std::runtime_error("Blocked non-public address resolution for host");
            }
        } else if (p->ai_family == AF_INET6) {
            struct sockaddr_in6* sa6 = (struct sockaddr_in6*)p->ai_addr;
            char buf[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &sa6->sin6_addr, buf, sizeof(buf));
            ips.insert(buf);
            if (!is_public_ipv6(sa6->sin6_addr)) {
                freeaddrinfo(res);
                throw std::runtime_error("Blocked non-public address resolution for host");
            }
        }
    }
    if (ips.empty()) {
        freeaddrinfo(res);
        throw std::runtime_error("No resolved addresses");
    }
    freeaddrinfo(res);
}

struct FetchContext {
    std::string body;
    std::string last_location;
    size_t max_bytes;
    bool truncated{false};
};

static size_t write_body_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total = size * nmemb;
    FetchContext* ctx = static_cast<FetchContext*>(userdata);
    size_t allowed = 0;
    if (ctx->body.size() < ctx->max_bytes) {
        size_t remaining = ctx->max_bytes - ctx->body.size();
        allowed = std::min(remaining, total);
        ctx->body.append(ptr, ptr + allowed);
    }
    if (allowed < total) {
        ctx->truncated = true;
        return allowed; // cause CURLE_WRITE_ERROR if not consuming all?
    }
    return total;
}

static size_t header_cb(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t total = size * nitems;
    std::string line(buffer, buffer + total);
    if (line.size() >= 9) {
        std::string prefix = line.substr(0, 9);
        std::transform(prefix.begin(), prefix.end(), prefix.begin(), [](unsigned char c){ return std::tolower(c); });
        if (prefix.rfind("location:", 0) == 0) {
            FetchContext* ctx = static_cast<FetchContext*>(userdata);
            std::string v = trim(line.substr(9));
            // Remove trailing CRLF
            while (!v.empty() && (v.back() == '\r' || v.back() == '\n')) v.pop_back();
            ctx->last_location = trim(v);
        }
    }
    return total;
}

static void parse_url(const std::string& url, std::string& scheme, std::string& host, int& port, std::string& path) {
    CURLU* u = curl_url();
    if (!u) throw std::runtime_error("URL parser init failed");
    CURLUcode cu = curl_url_set(u, CURLUPART_URL, url.c_str(), 0);
    if (cu) { curl_url_cleanup(u); throw std::runtime_error("Invalid URL"); }
    char* s = nullptr;
    char* h = nullptr;
    char* p = nullptr;
    char* pa = nullptr;
    if (curl_url_get(u, CURLUPART_SCHEME, &s, 0)) { curl_url_cleanup(u); throw std::runtime_error("Missing scheme"); }
    if (curl_url_get(u, CURLUPART_HOST, &h, 0)) { curl_free(s); curl_url_cleanup(u); throw std::runtime_error("Missing host"); }
    long lp = 0;
    CURLUcode gp = curl_url_get(u, CURLUPART_PORT, &p, 0);
    if (!gp && p) {
        lp = std::strtol(p, nullptr, 10);
    }
    if (curl_url_get(u, CURLUPART_PATH, &pa, 0)) {
        // empty path is ok
    }
    scheme = s ? s : "";
    host = h ? h : "";
    port = (lp > 0 && lp <= 65535) ? (int)lp : -1;
    path = pa ? pa : "";
    if (s) curl_free(s);
    if (h) curl_free(h);
    if (p) curl_free(p);
    if (pa) curl_free(pa);
    curl_url_cleanup(u);
}

static std::string resolve_redirect(const std::string& base, const std::string& location) {
    // If absolute, return as is
    if (location.find("http://") == 0 || location.find("https://") == 0) return location;
    // Parse base
    std::string scheme, host, path;
    int port;
    parse_url(base, scheme, host, port, path);
    std::string origin = scheme + "://" + host;
    if (port > 0 && !((scheme=="http" && port==80) || (scheme=="https" && port==443))) {
        origin += ":" + std::to_string(port);
    }
    if (!location.empty() && location[0] == '/') {
        return origin + location;
    } else {
        // relative to current path
        std::string basePath = path;
        if (basePath.empty()) basePath = "/";
        // remove filename segment
        size_t pos = basePath.find_last_of('/');
        std::string dir = (pos == std::string::npos) ? "/" : basePath.substr(0, pos + 1);
        return origin + dir + location;
    }
}

std::string fetch_url(const std::string& input_url) {
    CURL* curl = curl_easy_init();
    if (!curl) return "ERROR: Failed to init curl";
    std::string url = input_url;
    std::string trace;
    long redirects = 0;
    std::string final_url;

    try {
        // Initial validation
        std::string scheme, host, path;
        int port;
        parse_url(url, scheme, host, port, path);
        std::string ls = scheme;
        std::transform(ls.begin(), ls.end(), ls.begin(), ::tolower);
        if (!(ls == "http" || ls == "https")) throw std::runtime_error("Only http/https allowed");
        if (host.empty()) throw std::runtime_error("Missing host");
        if (port == 0) throw std::runtime_error("Invalid port");
        validate_public_resolution(ls, host, port);

        while (true) {
            FetchContext ctx;
            ctx.max_bytes = MAX_BYTES;
            ctx.body.clear();
            ctx.last_location.clear();
            ctx.truncated = false;

            curl_easy_reset(curl);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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
            long code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
            char* ct = nullptr;
            curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);

            trace += "URL: " + url + " -> Status: " + std::to_string(code) + "\n";

            if (code >= 300 && code < 400 && !ctx.last_location.empty() && redirects < MAX_REDIRECTS) {
                std::string next = resolve_redirect(url, ctx.last_location);
                // Validate next
                std::string scheme2, host2, path2;
                int port2;
                parse_url(next, scheme2, host2, port2, path2);
                std::string ls2 = scheme2;
                std::transform(ls2.begin(), ls2.end(), ls2.begin(), ::tolower);
                if (!(ls2 == "http" || ls2 == "https")) throw std::runtime_error("Only http/https allowed on redirect");
                if (host2.empty()) throw std::runtime_error("Missing host on redirect");
                if (port2 == 0) throw std::runtime_error("Invalid port on redirect");
                validate_public_resolution(ls2, host2, port2);
                url = next;
                redirects++;
                continue;
            }

            final_url = url;
            std::string content_type = ct ? std::string(ct) : "unknown";
            std::string preview = ctx.body;
            if (preview.size() > 200) {
                preview = preview.substr(0, 200) + "...";
            }
            std::string result = "Final URL: " + final_url +
                                 "\nHTTP Status: " + std::to_string(code) +
                                 "\nContent-Type: " + content_type +
                                 "\nBytes Read (capped): " + std::to_string(ctx.body.size()) +
                                 "\nRedirects Followed: " + std::to_string(redirects) +
                                 "\nTrace:\n" + trace +
                                 "\nPreview (first 200 bytes, may be truncated):\n" + preview;
            curl_easy_cleanup(curl);
            return result;
        }
    } catch (const std::exception& ex) {
        std::string err = std::string("ERROR: ") + ex.what();
        curl_easy_cleanup(curl);
        return err;
    }
}

int main() {
    std::vector<std::string> tests = {
        "https://example.com",
        "http://httpbin.org/get",
        "https://httpbin.org/redirect/1",
        "ftp://example.com",
        "http://127.0.0.1"
    };
    for (const auto& t : tests) {
        std::cout << "=== Test: " << t << " ===\n";
        std::cout << fetch_url(t) << "\n\n";
    }
    return 0;
}