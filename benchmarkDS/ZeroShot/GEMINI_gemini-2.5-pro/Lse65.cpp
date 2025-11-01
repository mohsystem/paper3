/*
 * IMPORTANT: This code requires the libcurl library.
 *
 * To compile on Linux/macOS:
 * g++ -o uploader Lse65.cpp -lcurl
 *
 * To compile on Windows (with MinGW and pre-compiled curl):
 * g++ -o uploader.exe Lse65.cpp -lcurl -lws2_32 -lwldap32
 *
 * You must have libcurl installed on your system.
 * The test cases will only work if a server (like the one in the Python or Java example)
 * is running and listening on the specified URL.
*/
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <fstream>
#include <vector>
#include <filesystem> // C++17

namespace fs = std::filesystem;

/**
 * @brief Uploads a PDF file to a server using a multipart/form-data POST request.
 *
 * @param serverUrl The URL of the server endpoint (e.g., "http://localhost:8080/upload").
 * @param pdfFilePath The path to the PDF file to upload.
 * @return true if the upload was successful (HTTP 2xx response), false otherwise.
 */
bool uploadPdfFile(const std::string& serverUrl, const std::string& pdfFilePath) {
    if (!fs::exists(pdfFilePath) || !fs::is_regular_file(pdfFilePath)) {
        std::cerr << "Error: File does not exist or is not a regular file: " << pdfFilePath << std::endl;
        return false;
    }

    CURL* curl;
    CURLcode res;
    bool success = false;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        curl_mime* mime = curl_mime_init(curl);
        curl_mimepart* part = curl_mime_addpart(mime);

        // Set the file part
        curl_mime_name(part, "file");
        curl_mime_filedata(part, pdfFilePath.c_str());
        curl_mime_type(part, "application/pdf");

        // Set curl options
        curl_easy_setopt(curl, CURLOPT_URL, serverUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);    // Fail on HTTP >= 400

        // For debugging:
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Perform the request
        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            // Consider 2xx as success
            if (http_code >= 200 && http_code < 300) {
                success = true;
            } else {
                 std::cerr << "Upload failed with HTTP status code: " << http_code << std::endl;
            }
        } else {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Cleanup
        curl_mime_free(mime);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return success;
}

void runTest(int testNum, const std::string& url, const fs::path& tempDir) {
    fs::path testFile = tempDir / ("test_cpp_" + std::to_string(testNum) + ".pdf");
    try {
        std::ofstream ofs(testFile);
        ofs << "Dummy C++ PDF content for test " << testNum;
        ofs.close();

        std::cout << "\n--- Test Case " << testNum << " ---" << std::endl;
        std::cout << "Uploading file: " << fs::absolute(testFile).string() << std::endl;
        
        if (uploadPdfFile(url, testFile.string())) {
            std::cout << "Result: Upload successful." << std::endl;
        } else {
            std::cout << "Result: Upload failed." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Test case " << testNum << " failed with exception: " << e.what() << std::endl;
    }
}

int main() {
    // NOTE: A server must be running at this URL for the tests to succeed.
    // You can run the Java or Python example file to start a compatible server.
    const std::string UPLOAD_URL = "http://localhost:8080/upload";
    
    std::cout << "C++ PDF Uploader" << std::endl;
    std::cout << "Make sure a server is running at " << UPLOAD_URL << std::endl;

    fs::path tempDir = fs::temp_directory_path() / "lse65_cpp_tests";
    try {
        fs::create_directories(tempDir);

        // --- Run 5 Test Cases ---
        for (int i = 1; i <= 5; ++i) {
            runTest(i, UPLOAD_URL, tempDir);
        }

        // --- Cleanup ---
        std::cout << "\nCleaning up temporary test files..." << std::endl;
        fs::remove_all(tempDir);
        std::cout << "Cleanup complete." << std::endl;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error during test setup/cleanup: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}