
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

// Callback function to write data received from curl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

json fetchAndParseJSON(const std::string& urlString) {
    if (urlString.empty()) {
        throw std::invalid_argument("URL cannot be empty");
    }
    
    // Validate protocol
    if (urlString.find("http://") != 0 && urlString.find("https://") != 0) {
        throw std::invalid_argument("Only HTTP and HTTPS protocols are allowed");
    }
    
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    try {
        curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::string error = "CURL error: " + std::string(curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            throw std::runtime_error(error);
        }
        
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        
        if (response_code != 200) {
            throw std::runtime_error("HTTP Error: " + std::to_string(response_code));
        }
        
        json jsonObject = json::parse(readBuffer);
        return jsonObject;
        
    } catch (const json::parse_error& e) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        throw std::runtime_error("JSON parse error: " + std::string(e.what()));
    } catch (...) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        throw;
    }
}

int main(int argc, char* argv[]) {
    // Test cases
    std::cout << "Test Case 1: Valid JSON URL" << std::endl;
    try {
        json result1 = fetchAndParseJSON("https://jsonplaceholder.typicode.com/todos/1");
        std::cout << "Result: " << result1.dump(2) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 2: Another valid JSON URL" << std::endl;
    try {
        json result2 = fetchAndParseJSON("https://jsonplaceholder.typicode.com/users/1");
        std::cout << "Result: " << result2.dump(2) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 3: Invalid URL format" << std::endl;
    try {
        json result3 = fetchAndParseJSON("not-a-valid-url");
        std::cout << "Result: " << result3.dump(2) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 4: Empty URL" << std::endl;
    try {
        json result4 = fetchAndParseJSON("");
        std::cout << "Result: " << result4.dump(2) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    std::cout << "\\nTest Case 5: Invalid protocol" << std::endl;
    try {
        json result5 = fetchAndParseJSON("ftp://example.com/data.json");
        std::cout << "Result: " << result5.dump(2) << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Command line argument support
    if (argc > 1) {
        std::cout << "\\nCommand line URL: " << argv[1] << std::endl;
        try {
            json result = fetchAndParseJSON(argv[1]);
            std::cout << "Result: " << result.dump(2) << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
}
