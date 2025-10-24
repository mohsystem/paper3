
#include <iostream>
#include <string>
#include <regex>
#include <curl/curl.h>

using namespace std;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

string getPageTitle(const string& urlString) {
    CURL* curl;
    CURLcode res;
    string readBuffer;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) {
            return "Error: " + string(curl_easy_strerror(res));
        }
        
        regex pattern("<title>(.*?)</title>", regex_constants::icase);
        smatch match;
        
        if (regex_search(readBuffer, match, pattern)) {
            string title = match[1].str();
            title.erase(0, title.find_first_not_of(" \\t\\r\\n"));
            title.erase(title.find_last_not_of(" \\t\\r\\n") + 1);
            return title;
        }
        
        return "No title found";
    }
    
    return "Error: Could not initialize CURL";
}

int main(int argc, char* argv[]) {
    string testUrls[] = {
        "https://www.example.com",
        "https://www.google.com",
        "https://www.github.com",
        "https://www.stackoverflow.com",
        "https://www.wikipedia.org"
    };
    
    if (argc > 1) {
        cout << "URL: " << argv[1] << endl;
        cout << "Title: " << getPageTitle(argv[1]) << endl;
    } else {
        cout << "Running test cases:" << endl;
        for (const string& url : testUrls) {
            cout << "\\nURL: " << url << endl;
            cout << "Title: " << getPageTitle(url) << endl;
        }
    }
    
    return 0;
}
