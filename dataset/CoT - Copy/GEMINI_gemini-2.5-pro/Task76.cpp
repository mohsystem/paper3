/*
 * COMPILE & RUN INSTRUCTIONS for a Linux-based system:
 * 1. Install libcurl development package:
 *    - On Debian/Ubuntu: sudo apt-get install libcurl4-openssl-dev
 *    - On Fedora/CentOS: sudo dnf install libcurl-devel
 * 2. Compile the code:
 *    g++ -std=c++17 Task76.cpp -o Task76 -lcurl
 * 3. Run the program with test cases:
 *    ./Task76
 * 4. Run with a command-line argument:
 *    ./Task76 https://some.url/
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm> // for std::transform
#include <curl/curl.h>

// Callback function to write received data into a std::string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total_size = size * nmemb;
    if (userp == nullptr) {
        return 0; // Or handle error appropriately
    }
    // Append data, preventing DoS by limiting total size
    const size_t MAX_SIZE = 1024 * 512; // 512 KB limit
    if (userp->size() + total_size > MAX_SIZE) {
        return 0; // Stop the transfer by returning a size different from total_size
    }
    userp->append((char*)contents, total_size);
    return total_size;
}

// Function to get the page title
std::string getPageTitle(const std::string& url) {
    // 1. Input validation: check for https prefix
    if (url.rfind("https://", 0) != 0) {
        return "Error: URL must use HTTPS.";
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Error: Failed to initialize libcurl.";
    }

    std::string readBuffer;
    
    // 2. Set secure curl options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L); // 15 second timeout
    // libcurl uses system's CA bundle for certificate verification by default.
    // DO NOT disable CURLOPT_SSL_VERIFYPEER or CURLOPT_SSL_VERIFYHOST.

    CURLcode res = curl_easy_perform(curl);
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "Error: curl_easy_perform() failed: " + std::string(curl_easy_strerror(res));
    }
    
    if(http_code != 200) {
        return "Error: Received non-200 status code: " + std::to_string(http_code);
    }

    // 3. Simple and secure parsing
    std::string lower_buffer = readBuffer;
    std::transform(lower_buffer.begin(), lower_buffer.end(), lower_buffer.begin(), ::tolower);

    size_t start_pos = lower_buffer.find("<title>");
    if (start_pos == std::string::npos) {
        return "Error: Title tag not found.";
    }
    start_pos += std::string("<title>").length();

    size_t end_pos = lower_buffer.find("</title>", start_pos);
    if (end_pos == std::string::npos) {
        return "Error: Closing title tag not found.";
    }

    // Extract from the original buffer to preserve case
    std::string title = readBuffer.substr(start_pos, end_pos - start_pos);
    
    // Trim whitespace (simple version)
    size_t first = title.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return "";
    }
    size_t last = title.find_last_not_of(" \t\n\r");
    return title.substr(first, (last - first + 1));
}

int main(int argc, char* argv[]) {
    // libcurl global initialization
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Test cases
    std::vector<std::string> testUrls = {
        "https://www.google.com",
        "https://github.com",
        "https://www.microsoft.com/en-us/",
        "https://www.w3.org/",
        "https://example.com"
    };
    
    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    for (const auto& url : testUrls) {
        std::cout << "URL: " << url << std::endl;
        std::string title = getPageTitle(url);
        std::cout << "Title: " << title << std::endl;
        std::cout << "--------------------" << std::endl;
    }
    
    // Handling command line arguments
    if (argc > 1) {
        std::cout << "\n--- Running with Command Line Argument ---" << std::endl;
        std::string urlFromArg = argv[1];
        std::cout << "URL: " << urlFromArg << std::endl;
        std::string title = getPageTitle(urlFromArg);
        std::cout << "Title: " << title << std::endl;
        std::cout << "--------------------" << std::endl;
    } else {
        std::cout << "\nNote: You can also provide a URL as a command-line argument." << std::endl;
    }
    
    // libcurl global cleanup
    curl_global_cleanup();

    return 0;
}