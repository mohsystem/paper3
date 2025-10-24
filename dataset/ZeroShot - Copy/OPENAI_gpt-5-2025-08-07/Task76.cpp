#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <curl/curl.h>
#include <arpa/inet.h>

static const size_t MAX_BYTES = 1048576; // 1 MiB

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t realsize = size * nmemb;
    std::string* buf = static_cast<std::string*>(userdata);
    size_t remaining = (MAX_BYTES > buf->size()) ? (MAX_BYTES - buf->size()) : 0;
    size_t to_copy = realsize < remaining ? realsize : remaining;
    if (to_copy > 0) {
        buf->append(ptr, to_copy);
    }
    // If we hit the cap, signal error to stop further writing
    if (realsize > to_copy) {
        return 0; // Causes CURLE_WRITE_ERROR
    }
    return realsize;
}

static bool is_literal_ip(const std::string& host) {
    // Handle IPv6 in brackets [::1]
    std::string h = host;
    if (!h.empty() && h.front() == '[' && h.back() == ']') {
        h = h.substr(1, h.size() - 2);
    }
    // Strip port if present (for IPv4 or hostname)
    auto pos = h.find('%'); // zone id for IPv6
    if (pos != std::string::npos) h = h.substr(0, pos);

    struct in_addr v4;
    struct in6_addr v6;
    if (inet_pton(AF_INET, h.c_str(), &v4) == 1) return true;
    if (inet_pton(AF_INET6, h.c_str(), &v6) == 1) return true;
    return false;
}

std::string fetch_title(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";

    std::string result;

    CURLU* cu = curl_url();
    if (!cu) {
        curl_easy_cleanup(curl);
        return "";
    }

    std::string title;

    do {
        if (curl_url_set(cu, CURLUPART_URL, url.c_str(), 0) != CURLUE_OK) {
            break;
        }

        char* scheme = nullptr;
        if (curl_url_get(cu, CURLUPART_SCHEME, &scheme, 0) != CURLUE_OK) {
            break;
        }
        std::string schemeStr = scheme ? scheme : "";
        curl_free(scheme);
        std::string schemeLower = schemeStr;
        std::transform(schemeLower.begin(), schemeLower.end(), schemeLower.begin(), ::tolower);
        if (schemeLower != "https") {
            break;
        }

        char* host = nullptr;
        if (curl_url_get(cu, CURLUPART_HOST, &host, 0) != CURLUE_OK) {
            break;
        }
        std::string hostStr = host ? host : "";
        curl_free(host);
        if (hostStr.empty()) break;
        std::string hostLower = hostStr;
        std::transform(hostLower.begin(), hostLower.end(), hostLower.begin(), ::tolower);
        if (hostLower == "localhost") {
            break;
        }
        if (is_literal_ip(hostStr)) {
            break;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
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
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            break;
        }
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
            break;
        }

        std::regex re("<title\\s*[^>]*>([\\s\\S]*?)</title>", std::regex::icase);
        std::smatch m;
        if (std::regex_search(result, m, re)) {
            title = m[1].str();
            // Collapse whitespace
            std::string out;
            out.reserve(title.size());
            bool ws = false;
            for (unsigned char c : title) {
                if (std::isspace(c)) {
                    if (!ws) {
                        out.push_back(' ');
                        ws = true;
                    }
                } else {
                    out.push_back(static_cast<char>(c));
                    ws = false;
                }
            }
            // trim
            size_t start = out.find_first_not_of(' ');
            size_t end = out.find_last_not_of(' ');
            if (start == std::string::npos) title.clear();
            else title = out.substr(start, end - start + 1);
        }

    } while (false);

    curl_url_cleanup(cu);
    curl_easy_cleanup(curl);
    return title;
}

int main(int argc, char** argv) {
    std::vector<std::string> tests;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) tests.emplace_back(argv[i]);
    } else {
        tests = {
            "https://example.com",
            "https://httpbin.org/html",
            "https://www.iana.org/domains/reserved",
            "https://www.wikipedia.org",
            "https://www.gnu.org"
        };
    }
    for (const auto& u : tests) {
        std::cout << u << " => " << fetch_title(u) << std::endl;
    }
    return 0;
}