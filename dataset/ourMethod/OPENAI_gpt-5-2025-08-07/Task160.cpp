#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cctype>
#include <sstream>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <curl/curl.h>

static const size_t MAX_URL_LENGTH = 2048;
static const long CONNECT_TIMEOUT = 10L;
static const size_t MAX_BODY_BYTES = 1024 * 1024; // 1 MiB

struct FetchResult {
    bool ok;
    long status;
    std::string headers;
    std::string body;
    std::string error;
};

static bool parse_url_basic(const std::string& url, std::string& scheme, std::string& host, int& port, bool& has_userinfo) {
    has_userinfo = false;
    scheme.clear(); host.clear(); port = -1;
    if (url.size() == 0 || url.size() > MAX_URL_LENGTH) return false;
    auto pos = url.find("://");
    if (pos == std::string::npos) return false;
    scheme = url.substr(0, pos);
    for (auto& c : scheme) c = (char)std::tolower((unsigned char)c);
    if (!(scheme == "http" || scheme == "https")) return false;
    size_t auth_start = pos + 3;
    size_t auth_end = url.find_first_of("/?#", auth_start);
    if (auth_end == std::string::npos) auth_end = url.size();
    std::string authority = url.substr(auth_start, auth_end - auth_start);
    if (authority.empty()) return false;
    if (authority.find('@') != std::string::npos) { has_userinfo = true; return false; }

    if (!authority.empty() && authority[0] == '[') {
        size_t rb = authority.find(']');
        if (rb == std::string::npos) return false;
        host = authority.substr(1, rb - 1);
        if (rb + 1 < authority.size()) {
            if (authority[rb + 1] != ':') return false;
            std::string port_str = authority.substr(rb + 2);
            if (port_str.empty()) return false;
            for (char ch : port_str) if (!std::isdigit((unsigned char)ch)) return false;
            long p = std::stol(port_str);
            if (p < 1 || p > 65535) return false;
            port = (int)p;
        }
    } else {
        size_t colon = authority.rfind(':');
        if (colon != std::string::npos && authority.find(':') == colon) {
            host = authority.substr(0, colon);
            std::string port_str = authority.substr(colon + 1);
            if (port_str.empty()) return false;
            for (char ch : port_str) if (!std::isdigit((unsigned char)ch)) return false;
            long p = std::stol(port_str);
            if (p < 1 || p > 65535) return false;
            port = (int)p;
        } else {
            host = authority;
        }
    }
    if (host.empty()) return false;
    if (port == -1) port = (scheme == "https") ? 443 : 80;
    return true;
}

static bool is_public_ipv4(uint32_t ip) {
    // ip in network byte order
    uint32_t h = ntohl(ip);

    // Unspecified
    if (h == 0x00000000) return false;
    // Broadcast
    if (h == 0xFFFFFFFF) return false;
    // Loopback 127.0.0.0/8
    if ((h & 0xFF000000) == 0x7F000000) return false;
    // Link-local 169.254.0.0/16
    if ((h & 0xFFFF0000) == 0xA9FE0000) return false;
    // Private 10.0.0.0/8
    if ((h & 0xFF000000) == 0x0A000000) return false;
    // Private 172.16.0.0/12
    if ((h & 0xFFF00000) == 0xAC100000) return false;
    // Private 192.168.0.0/16
    if ((h & 0xFFFF0000) == 0xC0A80000) return false;
    // Multicast 224.0.0.0/4
    if ((h & 0xF0000000) == 0xE0000000) return false;

    return true;
}

static bool is_public_ipv6(const unsigned char ip[16]) {
    // Unspecified ::/128
    bool all_zero = true;
    for (int i = 0; i < 16; ++i) if (ip[i] != 0) { all_zero = false; break; }
    if (all_zero) return false;
    // Loopback ::1/128
    bool loop = true;
    for (int i = 0; i < 15; ++i) if (ip[i] != 0) { loop = false; break; }
    if (loop && ip[15] == 1) return false;
    // Link-local fe80::/10
    if ((ip[0] == 0xFE) && ((ip[1] & 0xC0) == 0x80)) return false;
    // Multicast ff00::/8
    if (ip[0] == 0xFF) return false;
    // Unique local fc00::/7 (fc00..fdff)
    if ((ip[0] & 0xFE) == 0xFC) return false;

    return true;
}

static bool host_is_public_and_routable(const std::string& host) {
    addrinfo hints{};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_ADDRCONFIG;
    addrinfo* res = nullptr;
    int rc = getaddrinfo(host.c_str(), nullptr, &hints, &res);
    if (rc != 0 || !res) return false;
    bool ok = true;
    for (addrinfo* ai = res; ai; ai = ai->ai_next) {
        if (ai->ai_family == AF_INET) {
            sockaddr_in* in = (sockaddr_in*)ai->ai_addr;
            if (!is_public_ipv4(in->sin_addr.s_addr)) { ok = false; break; }
        } else if (ai->ai_family == AF_INET6) {
            sockaddr_in6* in6 = (sockaddr_in6*)ai->ai_addr;
            if (!is_public_ipv6(in6->sin6_addr.s6_addr)) { ok = false; break; }
        } else {
            ok = false; break;
        }
    }
    freeaddrinfo(res);
    return ok;
}

static size_t write_capped(void* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* buf = static_cast<std::string*>(userdata);
    size_t total = size * nmemb;
    if (buf->size() >= MAX_BODY_BYTES) return 0; // stop
    size_t remain = MAX_BODY_BYTES - buf->size();
    size_t to_copy = total < remain ? total : remain;
    buf->append(static_cast<char*>(ptr), static_cast<char*>(ptr) + to_copy);
    return to_copy;
}

static size_t header_accum(char* buffer, size_t size, size_t nitems, void* userdata) {
    std::string* hdrs = static_cast<std::string*>(userdata);
    size_t total = size * nitems;
    hdrs->append(buffer, buffer + total);
    return total;
}

static FetchResult fetch_via_curl(const std::string& url) {
    FetchResult result{};
    CURL* curl = curl_easy_init();
    if (!curl) {
        result.ok = false;
        result.error = "Failed to initialize curl";
        return result;
    }
    std::string body;
    std::string headers;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task160-Client/1.0");
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "identity");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_capped);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_accum);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers);

    CURLcode rc = curl_easy_perform(curl);
    if (rc != CURLE_OK) {
        result.ok = false;
        result.error = curl_easy_strerror(rc);
        curl_easy_cleanup(curl);
        return result;
    }
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    result.ok = true;
    result.status = status;
    result.headers = headers;
    if (body.size() >= MAX_BODY_BYTES) {
        body.append("\n[Truncated to " + std::to_string(MAX_BODY_BYTES) + " bytes]");
    }
    result.body = body;
    return result;
}

static std::string fetch_url_safely(const std::string& raw_url) {
    std::string scheme, host;
    int port = -1;
    bool has_userinfo = false;
    if (!parse_url_basic(raw_url, scheme, host, port, has_userinfo)) {
        return "ERROR: Invalid or disallowed URL";
    }
    if (!host_is_public_and_routable(host)) {
        return "ERROR: Host resolves to a private, loopback, link-local, multicast, or unspecified address";
    }
    FetchResult r = fetch_via_curl(raw_url);
    if (!r.ok) {
        return std::string("ERROR: ") + r.error;
    }
    std::ostringstream oss;
    oss << "STATUS: " << r.status << "\n";
    oss << "HEADERS:\n" << r.headers;
    oss << "BODY:\n" << r.body;
    return oss.str();
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::vector<std::string> tests = {
        "https://example.com",
        "http://example.com",
        "ftp://example.com",
        "http://127.0.0.1",
        "http://user:pass@example.com"
    };
    for (const auto& t : tests) {
        std::cout << "URL: " << t << "\n";
        std::cout << fetch_url_safely(t) << "\n----\n";
    }
    curl_global_cleanup();
    return 0;
}