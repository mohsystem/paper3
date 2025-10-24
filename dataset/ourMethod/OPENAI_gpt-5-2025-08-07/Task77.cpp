#include <curl/curl.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <regex>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static bool validate_inputs(const std::string& host, const std::string& user, const std::string& pass, const std::string& remotePath) {
    static const std::regex hostRe("^[A-Za-z0-9._-]{1,255}$");
    if (!std::regex_match(host, hostRe)) return false;
    if (user.empty() || user.size() > 128) return false;
    if (pass.empty() || pass.size() > 256) return false;
    if (remotePath.empty() || remotePath.size() > 1024 || (!remotePath.empty() && remotePath.back() == '/')) return false;
    return true;
}

static std::string sanitize_filename(const std::string& remotePath) {
    std::string base = remotePath;
    for (char& c : base) if (c == '\\') c = '/';
    auto pos = base.find_last_of('/');
    if (pos != std::string::npos) base = base.substr(pos + 1);
    if (base.size() > 255) base = base.substr(0, 255);
    if (base.empty() || base == "." || base == "..") return "downloaded.bin";
    std::string out;
    out.reserve(base.size());
    for (char c : base) {
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_') {
            out.push_back(c);
        } else {
            out.push_back('_');
        }
    }
    if (out.empty()) out = "downloaded.bin";
    return out;
}

struct WriteCtx {
    int fd;
    size_t written;
};

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    WriteCtx* ctx = static_cast<WriteCtx*>(userdata);
    size_t total = size * nmemb;
    if (total == 0) return 0;
    ssize_t w = write(ctx->fd, ptr, total);
    if (w < 0) return 0;
    ctx->written += static_cast<size_t>(w);
    if (ctx->written > (size_t)2ULL * 1024ULL * 1024ULL * 1024ULL) { // 2GB safety
        return 0;
    }
    return (size_t)w;
}

bool downloadFtpFile(const std::string& host, const std::string& user, const std::string& pass, const std::string& remotePath, std::string& outLocalPath) {
    outLocalPath.clear();
    if (!validate_inputs(host, user, pass, remotePath)) {
        std::cerr << "Invalid inputs\n";
        return false;
    }

    std::string localName = sanitize_filename(remotePath);
    std::string url = "ftps://" + host + (remotePath.size() && remotePath[0] == '/' ? remotePath : ("/" + remotePath));

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL init failed\n";
        return false;
    }

    char tmpl[] = "./.dl_part_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd < 0) {
        curl_easy_cleanup(curl);
        std::cerr << "Temp file creation failed\n";
        return false;
    }
    fchmod(fd, 0600);

    WriteCtx ctx{fd, 0};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, user.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, pass.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_FTP_SSL_CCC, (long)CURLFTPSSL_CCC_NONE);
    curl_easy_setopt(curl, CURLOPT_FTP_FILEMETHOD, (long)CURLFTPMETHOD_SINGLECWD);
    curl_easy_setopt(curl, CURLOPT_TRANSFERTEXT, 0L); // binary
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

    CURLcode res = curl_easy_perform(curl);
    int saved_errno = 0;
    if (fsync(fd) != 0) saved_errno = errno;
    close(fd);

    bool ok = (res == CURLE_OK) && (saved_errno == 0);
    if (!ok) {
        unlink(tmpl);
        if (res != CURLE_OK) {
            std::cerr << "Transfer failed: " << curl_easy_strerror(res) << "\n";
        } else {
            std::cerr << "File sync failed\n";
        }
        curl_easy_cleanup(curl);
        return false;
    }

    std::string finalPath = "./" + localName;
    if (rename(tmpl, finalPath.c_str()) != 0) {
        unlink(tmpl);
        std::cerr << "Rename failed\n";
        curl_easy_cleanup(curl);
        return false;
    }

    chmod(finalPath.c_str(), 0600);
    outLocalPath = finalPath;
    curl_easy_cleanup(curl);
    return true;
}

int main(int argc, char** argv) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    if (argc == 5) {
        std::string out;
        bool ok = downloadFtpFile(argv[1], argv[2], argv[3], argv[4], out);
        if (ok) {
            std::cout << "Downloaded to: " << out << "\n";
        } else {
            std::cerr << "Operation failed\n";
            curl_global_cleanup();
            return 1;
        }
        curl_global_cleanup();
        return 0;
    }

    std::cout << "Usage: ./task77 <host> <username> <password> <remotePath>\n";
    struct Test { const char* h; const char* u; const char* p; const char* r; } tests[5] = {
        {"invalid.example", "user", "pass", "/readme.txt"},
        {"example.com", "anonymous", "guest@example.com", "/pub/example.dat"},
        {"localhost", "ftpuser", "p@ssw0rd!", "folder/file.txt"},
        {"ftp.example.org", "user_name", "secret", "/path/with space/file name.bin"},
        {"192.0.2.10", "u", "p", "/sensitive/secret.bin"}
    };
    for (int i = 0; i < 5; ++i) {
        std::string out;
        bool ok = downloadFtpFile(tests[i].h, tests[i].u, tests[i].p, tests[i].r, out);
        if (ok) {
            std::cout << "Test " << (i + 1) << " downloaded to: " << out << "\n";
        } else {
            std::cout << "Test " << (i + 1) << " failed\n";
        }
    }
    curl_global_cleanup();
    return 0;
}