#include <iostream>
#include <string>
#include <cstdio>
#include <curl/curl.h>

// Note: This code requires libcurl.
// To compile: g++ -o ftp_downloader Task77.cpp -lcurl

// Callback function to write received data to a file
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

// Security: Check for path traversal characters in the local filename.
bool is_safe_filename(const std::string& filename) {
    return filename.find('/') == std::string::npos &&
           filename.find('\\') == std::string::npos &&
           filename.find("..") == std::string::npos;
}

/**
 * Connects to an FTP server using FTPS and downloads a file.
 *
 * @param host        The FTP server hostname or IP address.
 * @param username    The username for authentication.
 * @param password    The password for authentication.
 * @param remote_file The path to the file on the server.
 * @param local_file  The name of the file to save locally. Path traversal is prevented.
 * @return true on success, false on failure.
 */
bool downloadFileSecureFTP(const std::string& host, const std::string& username, const std::string& password,
                           const std::string& remote_file, const std::string& local_file) {
    
    if (!is_safe_filename(local_file)) {
        std::cerr << "Error: Local filename '" << local_file << "' contains invalid characters." << std::endl;
        return false;
    }

    CURL *curl;
    CURLcode res = CURLE_OK;
    FILE *outfile = nullptr;

    // Construct the FTPS URL
    std::string url = "ftps://" + host + "/" + remote_file;
    // Construct credentials
    std::string userpwd = username + ":" + password;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        outfile = fopen(local_file.c_str(), "wb");
        if (!outfile) {
            std::cerr << "Error opening local file for writing." << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        // Set FTPS URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // Set credentials
        curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());
        // Enforce FTPS (explicit TLS)
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        // For security, verify the peer's certificate.
        // For self-signed certs, you might need CURLOPT_SSL_VERIFYPEER, 0L but this is not recommended.
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        // Set the write callback function and data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);

        std::cout << "Starting download from " << url << " to " << local_file << std::endl;
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Download successful." << std::endl;
        }

        // Cleanup
        fclose(outfile);
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize curl." << std::endl;
        res = CURLE_FAILED_INIT;
    }
    
    curl_global_cleanup();
    return res == CURLE_OK;
}

int main(int argc, char *argv[]) {
    // SECURITY WARNING: Passing credentials as command-line arguments is insecure.
    // They can be viewed by other users on the system via the process list and
    // may be stored in shell history. In a production environment, use a more
    // secure method like environment variables, a secure vault, or interactive prompts.
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <host> <username> <password> <remote_file_path> <local_file_name>" << std::endl;
        std::cerr << "\n--- Test Case Examples (replace with actual server details) ---" << std::endl;
        std::cerr << "1. Download 'report.csv' from 'ftp.example.com':" << std::endl;
        std::cerr << "   " << argv[0] << " ftp.example.com user1 pass1 reports/report.csv report_local.csv" << std::endl;
        std::cerr << "2. Download 'archive.zip' from an IP address:" << std::endl;
        std::cerr << "   " << argv[0] << " 192.168.1.100 admin secret backups/archive.zip backup.zip" << std::endl;
        std::cerr << "3. Download 'image.jpg' from a public test server (credentials may vary):" << std::endl;
        std::cerr << "   " << argv[0] << " test.rebex.net demo password pub/example/image.jpg example.jpg" << std::endl;
        std::cerr << "4. Attempt to download a non-existent file:" << std::endl;
        std::cerr << "   " << argv[0] << " ftp.example.com user1 pass1 path/to/nonexistent.file missing.txt" << std::endl;
        std::cerr << "5. Use invalid credentials:" << std::endl;
        std::cerr << "   " << argv[0] << " ftp.example.com wronguser wrongpass reports/report.csv report.csv" << std::endl;
        return 1;
    }

    downloadFileSecureFTP(argv[1], argv[2], argv[3], argv[4], argv[5]);

    return 0;
}