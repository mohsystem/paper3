#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <curl/curl.h>

/*
Chain-of-Through process:
1) Problem understanding:
   - Fetch HTML from a HTTPS URL and return its <title>.
2) Security requirements:
   - Enforce HTTPS only and restrict redirects to HTTPS.
   - Apply timeouts and download size limits to prevent abuse.
3) Secure coding generation:
   - Use libcurl with CURLOPT_PROTOCOLS/CURLOPT_REDIR_PROTOCOLS to HTTPS.
   - Set timeouts and a write callback that limits size.
4) Code review:
   - Verified error checks, size limiting, input validation.
5) Secure code output:
   - Finalized with safe defaults and robust parsing.
*/

static const size_t MAX_BYTES = 1048576; // 1 MB

struct Buffer {
    std::string data;
    bool overflow = false;
};

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    size_t total = size * nmemb;
    Buffer* buf = static_cast<Buffer*>(userdata);
    size_t space = (MAX_BYTES > buf->data.size()) ? (MAX_BYTES - buf->data.size()) : 0;
    size_t to_copy = total < space ? total : space;
    if (to_copy > 0) {
        buf->data.append(ptr, to_copy);
    }
    if (total > to_copy) {
        buf->overflow = true;
        return 0; // abort transfer
    }
    return total;
}

static bool starts_with_https(const std::string& url) {
    if (url.size() < 8) return false;
    std::string prefix = url.substr(0, 8);
    for (auto& c : prefix) c = (char)tolower((unsigned char)c);
    return prefix == "https://";
}

std::string get_page_title(const std::string& url) {
    if (!starts_with_https(url)) return "";

    CURL* curl = curl_easy_init();
    if (!curl) return "";

    Buffer buf;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task76-Client/1.0 (+https)");
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); // enable compression
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK || buf.data.empty()) {
        return "";
    }

    std::regex title_re("<title[^>]*>([\\s\\S]*?)</title>", std::regex::icase);
    std::smatch m;
    if (std::regex_search(buf.data, m, title_re)) {
        std::string title = m[1].str();
        // collapse whitespace
        std::string out;
        bool in_space = false;
        for (char ch : title) {
            if (isspace(static_cast<unsigned char>(ch))) {
                if (!in_space) {
                    out.push_back(' ');
                    in_space = true;
                }
            } else {
                out.push_back(ch);
                in_space = false;
            }
        }
        // trim
        size_t start = out.find_first_not_of(' ');
        size_t end = out.find_last_not_of(' ');
        if (start == std::string::npos) return "";
        return out.substr(start, end - start + 1);
    }
    return "";
}

int main(int argc, char** argv) {
    std::vector<std::string> urls;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) urls.emplace_back(argv[i]);
    } else {
        urls = {
            "https://example.com",
            "https://www.wikipedia.org/",
            "https://www.mozilla.org/en-US/",
            "https://www.bbc.com/",
            "https://www.oracle.com/"
        };
    }
    for (const auto& u : urls) {
        std::cout << u << " -> " << get_page_title(u) << "\n";
    }
    return 0;
}