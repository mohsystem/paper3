
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string readJsonFromUrl(const std::string& url) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if(res != CURLE_OK) {
            std::cerr << "Error: " << curl_easy_strerror(res) << std::endl;
            return "";
        }
    }
    return readBuffer;
}

json parseJson(const std::string& url) {
    try {
        std::string jsonContent = readJsonFromUrl(url);
        if(!jsonContent.empty()) {
            return json::parse(jsonContent);
        }
        return json();
    } catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return json();
    }
}

int main(int argc, char* argv[]) {
    std::string testUrls[] = {
        "https://jsonplaceholder.typicode.com/posts/1",
        "https://jsonplaceholder.typicode.com/users/1",
        "https://jsonplaceholder.typicode.com/comments/1",
        "https://jsonplaceholder.typicode.com/todos/1",
        "https://jsonplaceholder.typicode.com/albums/1"
    };
    
    for(int i = 0; i < 5; i++) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "URL: " << testUrls[i] << std::endl;
        json result = parseJson(testUrls[i]);
        if(!result.empty()) {
            std::cout << "JSON Object: " << result.dump(2) << std::endl;
        }
        std::cout << std::endl;
    }
    
    return 0;
}
