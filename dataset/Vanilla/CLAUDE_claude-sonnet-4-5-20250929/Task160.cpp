
#include <iostream>
#include <string>
#include <curl/curl.h>

using namespace std;

// Callback function to write received data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

string makeHttpRequest(const string& urlString) {
    CURL* curl;
    CURLcode res;
    string response;
    long responseCode;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            response = "Error: " + string(curl_easy_strerror(res));
        } else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
            response = "Response Code: " + to_string(responseCode) + "\\n" + response;
        }
        
        curl_easy_cleanup(curl);
    } else {
        response = "Error: Failed to initialize CURL";
    }
    
    curl_global_cleanup();
    return response;
}

int main() {
    // Test cases
    string testUrls[] = {
        "https://jsonplaceholder.typicode.com/posts/1",
        "https://api.github.com/users/github",
        "https://httpbin.org/get",
        "https://www.google.com",
        "https://example.com"
    };
    
    for (int i = 0; i < 5; i++) {
        cout << "Test Case " << (i + 1) << ": " << testUrls[i] << endl;
        string result = makeHttpRequest(testUrls[i]);
        cout << result.substr(0, min((size_t)200, result.length())) << "...\\n\\n";
    }
    
    return 0;
}
