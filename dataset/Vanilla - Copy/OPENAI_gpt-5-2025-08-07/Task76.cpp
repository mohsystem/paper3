#include <iostream>
#include <string>
#include <regex>
#include <curl/curl.h>

static size_t writeToString(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    std::string* s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), total);
    return total;
}

static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string fetchTitle(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    std::string buffer;
    char errbuf[CURL_ERROR_SIZE] = {0};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task76-CPP/1.0 (+https://example.com)");
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); // enable automatic decompression
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) {
        return "";
    }

    try {
        std::regex re("<title[^>]*>([\\s\\S]*?)</title>", std::regex::icase);
        std::smatch m;
        if (std::regex_search(buffer, m, re)) {
            return trim(m[1].str());
        }
    } catch (...) {
        // std::regex may throw on some libstdc++ versions
    }
    return "";
}

int main(int argc, char** argv) {
    std::vector<std::string> urls;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) urls.emplace_back(argv[i]);
    } else {
        urls = {
            "https://www.example.com/",
            "https://www.wikipedia.org/",
            "https://www.iana.org/domains/reserved",
            "https://www.github.com/",
            "https://www.stackoverflow.com/"
        };
    }

    for (const auto& u : urls) {
        std::string title = fetchTitle(u);
        std::cout << u << " => " << title << std::endl;
    }
    return 0;
}