#include <curl/curl.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

struct Buffer {
    std::string data;
    bool stop = false;
    size_t maxBytes = 1024 * 1024; // 1 MB
};

static bool containsCaseInsensitive(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    auto it = std::search(
        haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](char ch1, char ch2) { return std::tolower(static_cast<unsigned char>(ch1)) == std::tolower(static_cast<unsigned char>(ch2)); }
    );
    return it != haystack.end();
}

static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t realSize = size * nmemb;
    Buffer* buf = static_cast<Buffer*>(userdata);
    if (!buf || !ptr) return 0;

    size_t remaining = (buf->maxBytes > buf->data.size()) ? (buf->maxBytes - buf->data.size()) : 0;
    size_t toCopy = (realSize < remaining) ? realSize : remaining;

    if (toCopy > 0) {
        buf->data.append(ptr, toCopy);
    }

    // Stop early if closing tag found or limit reached
    if (containsCaseInsensitive(buf->data, "</title>") || buf->data.size() >= buf->maxBytes) {
        buf->stop = true;
        return 0; // abort transfer intentionally
    }

    // If there was more data but we reached limit, abort
    if (toCopy < realSize) {
        buf->stop = true;
        return 0;
    }
    return realSize;
}

static std::string extractTitle(const std::string& html) {
    std::string lower = html;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });

    size_t tstart = lower.find("<title");
    if (tstart == std::string::npos) return "";
    size_t gt = lower.find('>', tstart);
    if (gt == std::string::npos) return "";
    size_t tend = lower.find("</title>", gt + 1);
    if (tend == std::string::npos) return "";

    std::string raw = html.substr(gt + 1, tend - (gt + 1));
    // Remove inner tags
    std::string out;
    out.reserve(raw.size());
    bool inTag = false;
    for (char c : raw) {
        if (c == '<') { inTag = true; continue; }
        if (c == '>') { inTag = false; continue; }
        if (!inTag) out.push_back(c);
    }
    // Normalize whitespace
    std::string title;
    title.reserve(out.size());
    bool prevSpace = false;
    for (char c : out) {
        if (c == '\r' || c == '\n' || c == '\t') c = ' ';
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!prevSpace) {
                title.push_back(' ');
                prevSpace = true;
            }
        } else {
            title.push_back(c);
            prevSpace = false;
        }
    }
    // Trim
    auto ltrim = [](std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){ return !std::isspace(ch); }));
    };
    auto rtrim = [](std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
    };
    ltrim(title);
    rtrim(title);
    return title;
}

std::string fetch_title(const std::string& url) {
    try {
        if (url.empty()) return "ERROR: URL is empty";
        if (url.size() > 2048) return "ERROR: URL too long";
        std::string lower = url;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });
        if (lower.rfind("https://", 0) != 0) {
            return "ERROR: Only HTTPS URLs are allowed";
        }

        CURLcode ginit = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (ginit != CURLE_OK) {
            return "ERROR: curl init failed";
        }

        CURL* curl = curl_easy_init();
        if (!curl) {
            curl_global_cleanup();
            return "ERROR: curl easy init failed";
        }

        Buffer buf;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureTitleFetcher/1.0 (+https://example.com)");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
        curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

        CURLcode res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        curl_easy_cleanup(curl);
        curl_global_cleanup();

        if (res != CURLE_OK && !buf.stop) {
            return std::string("ERROR: ") + curl_easy_strerror(res);
        }
        if (http_code != 0 && (http_code < 200 || http_code >= 300) && !buf.stop) {
            return "ERROR: HTTP " + std::to_string(http_code);
        }

        std::string title = extractTitle(buf.data);
        if (title.empty()) return "ERROR: Title not found";
        return title;
    } catch (const std::exception& ex) {
        std::string msg = ex.what() ? ex.what() : "Unexpected error";
        if (msg.size() > 200) msg = msg.substr(0, 200) + "...";
        return std::string("ERROR: ") + msg;
    }
}

int main(int argc, char** argv) {
    std::string tests[5] = {
        "https://example.com",
        "https://www.wikipedia.org",
        "https://www.iana.org/domains/reserved",
        "https://www.gnu.org",
        "https://httpbin.org/html"
    };

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            std::cout << argv[i] << " => " << fetch_title(argv[i]) << "\n";
        }
    } else {
        for (const auto& u : tests) {
            std::cout << u << " => " << fetch_title(u) << "\n";
        }
    }
    return 0;
}