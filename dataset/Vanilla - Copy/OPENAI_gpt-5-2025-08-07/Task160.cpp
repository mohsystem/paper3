#include <iostream>
#include <string>
#include <cstring>
#include <curl/curl.h>

static size_t write_to_string(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total = size * nmemb;
    std::string* s = static_cast<std::string*>(userp);
    s->append(static_cast<char*>(contents), total);
    return total;
}

std::string fetch_url(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return "ERROR: Failed to initialize CURL";
    }

    std::string buffer;
    char errbuf[CURL_ERROR_SIZE];
    errbuf[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Task160/1.0");
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); // enable all supported encodings
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_cleanup(curl);
        return "HTTP " + std::to_string(http_code) + "\n" + buffer;
    } else {
        std::string err = "ERROR: ";
        if (errbuf[0] != '\0') {
            err += errbuf;
        } else {
            err += curl_easy_strerror(res);
        }
        curl_easy_cleanup(curl);
        return err;
    }
}

int main() {
    // Initialize global curl state (optional in modern libcurl, but good practice)
    curl_global_init(CURL_GLOBAL_DEFAULT);

    const char* urls[5] = {
        "https://www.example.com/",
        "https://httpbin.org/get",
        "https://jsonplaceholder.typicode.com/todos/1",
        "http://httpstat.us/200",
        "http://httpstat.us/404"
    };

    for (int i = 0; i < 5; ++i) {
        std::string url = urls[i];
        std::string res = fetch_url(url);
        std::cout << "=== Test " << (i + 1) << ": " << url << " ===\n";
        if (res.size() > 500) {
            std::cout << res.substr(0, 500) << "...\n\n";
        } else {
            std::cout << res << "\n\n";
        }
    }

    curl_global_cleanup();
    return 0;
}