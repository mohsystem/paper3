#include <iostream>
#include <string>
#include <cstdio>
#include <curl/curl.h>

static std::string basename_from_path(const std::string& path) {
    if (path.empty()) return std::string();
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

std::string downloadFtpFile(const std::string& host, int port, const std::string& user, const std::string& pass, const std::string& remotePath) {
    CURL* curl = nullptr;
    CURLcode res;
    std::string localName = basename_from_path(remotePath);
    if (localName.empty()) return std::string();

    std::string path = remotePath.size() && remotePath[0] == '/' ? remotePath : ("/" + remotePath);
    std::string url = "ftp://" + host + ":" + std::to_string(port) + path;

    FILE* fp = std::fopen(localName.c_str(), "wb");
    if (!fp) return std::string();

    curl = curl_easy_init();
    if (!curl) {
        std::fclose(fp);
        return std::string();
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, user.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, pass.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
    curl_easy_setopt(curl, CURLOPT_TRANSFERTEXT, 0L);
    curl_easy_setopt(curl, CURLOPT_FTP_USE_EPSV, 1L);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    std::fclose(fp);

    if (res != CURLE_OK) {
        std::remove(localName.c_str());
        return std::string();
    }
    return localName;
}

int main(int argc, char** argv) {
    if (argc >= 5) {
        std::string host = argv[1];
        std::string user = argv[2];
        std::string pass = argv[3];
        std::string remote = argv[4];
        int port = 21;
        if (argc >= 6) {
            try { port = std::stoi(argv[5]); } catch (...) {}
        }
        std::string saved = downloadFtpFile(host, port, user, pass, remote);
        if (!saved.empty()) {
            std::cout << "Downloaded: " << saved << std::endl;
        } else {
            std::cout << "Download failed" << std::endl;
        }
    } else {
        // 5 test cases using public test FTP server
        std::string host = "test.rebex.net";
        std::string user = "demo";
        std::string pass = "password";
        int port = 21;
        const char* files[5] = {
            "pub/example/readme.txt",
            "pub/example/ConsoleClient.png",
            "pub/example/KeyGenerator.png",
            "pub/example/WinFormClient.png",
            "pub/example/WinSshShell.png"
        };
        for (int i = 0; i < 5; ++i) {
            std::string saved = downloadFtpFile(host, port, user, pass, files[i]);
            std::cout << ((saved.empty()) ? "Failed: " : "Downloaded: ") << files[i] << std::endl;
        }
    }
    return 0;
}