// Chain-of-Through process:
// 1) Problem understanding: C++ function uses libcurl (FTPS) to securely download a file from an FTP server to current directory.
// 2) Security requirements: Enforce TLS (ftps://), verify certificates, sanitize filename, no overwrite, timeouts, atomic rename.
// 3) Secure coding generation: Use libcurl with strict SSL options, passive mode by default, safe temp files.
// 4) Code review: Ensured proper cleanup, error handling, and no sensitive logging.
// 5) Secure code output: Final code attempts FTPS with verification and secure local file handling.
#include <curl/curl.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

static std::string basename_safe(const std::string& path) {
    std::string p = path;
    for (auto& c : p) if (c == '\\') c = '/';
    size_t pos = p.find_last_of('/');
    std::string base = (pos == std::string::npos) ? p : p.substr(pos + 1);
    if (base.empty() || base == "." || base == "..") base = "downloaded_file";
    // sanitize
    for (auto& c : base) {
        if (c == '/' || c == '\\' || c == '\0' || c == '
' || c == '\r' || c == '\t') c = '_';
    }
    return base;
}

static std::string encode_path_segments(CURL* curl, const std::string& remote_path) {
    // Encode each segment but keep slashes
    std::string p = remote_path;
    for (auto& c : p) if (c == '\\') c = '/';
    std::stringstream ss;
    size_t start = 0;
    while (start <= p.size()) {
        size_t slash = p.find('/', start);
        std::string seg = (slash == std::string::npos) ? p.substr(start) : p.substr(start, slash - start);
        char* enc = curl_easy_escape(curl, seg.c_str(), (int)seg.size());
        if (enc) {
            ss << enc;
            curl_free(enc);
        }
        if (slash == std::string::npos) break;
        ss << "/";
        start = slash + 1;
    }
    std::string out = ss.str();
    if (out.empty()) out = "";
    if (!out.empty() && out[0] != '/') out = out; // leave relative as-is
    return out;
}

static size_t write_file(void* ptr, size_t size, size_t nmemb, void* stream) {
    FILE* f = static_cast<FILE*>(stream);
    return fwrite(ptr, size, nmemb, f);
}

bool download_sensitive_file(const std::string& host, long port, const std::string& username, const std::string& password, const std::string& remote_path) {
    if (host.empty() || remote_path.empty()) return false;

    std::string base = basename_safe(remote_path);
    std::string final_path = base;
    std::string tmp_path = base + ".tmp";

    // Do not overwrite existing file
    struct stat st;
    if (stat(final_path.c_str(), &st) == 0) {
        return false;
    }

    CURL* curl = curl_easy_init();
    if (!curl) return false;

    bool ok = false;
    FILE* f = nullptr;
    int fd = -1;

    char errbuf[CURL_ERROR_SIZE];
    errbuf[0] = '\0';

    do {
        std::string url = "ftps://" + host;
        if (port > 0) {
            url += ":" + std::to_string(port);
        }
        url += "/";
        std::string encoded = encode_path_segments(curl, remote_path[0] == '/' ? remote_path.substr(1) : remote_path);
        url += encoded;

        // Create temp file exclusively with 0600 perms
        std::string pattern = tmp_path + ".XXXXXX";
        std::vector<char> pat(pattern.begin(), pattern.end());
        pat.push_back('\0');
        fd = mkstemp(pat.data());
        if (fd == -1) break;
        fchmod(fd, S_IRUSR | S_IWUSR); // 0600
        f = fdopen(fd, "wb");
        if (!f) { close(fd); fd = -1; unlink(pat.data()); break; }
        tmp_path = pat.data(); // actual created temp name

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_FTPSSLAUTH, (long)CURLFTPAUTH_DEFAULT);
        curl_easy_setopt(curl, CURLOPT_FTP_SSL_CCC, (long)CURLFTPSSL_CCC_NONE);
        curl_easy_setopt(curl, CURLOPT_TRANSFERTEXT, 0L); // binary
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            break;
        }

        fflush(f);
        int ffd = fileno(f);
        fsync(ffd);
        fclose(f); f = nullptr;

        // Rename temp to final
        if (rename(tmp_path.c_str(), final_path.c_str()) != 0) {
            unlink(tmp_path.c_str());
            break;
        }

        ok = true;
    } while (false);

    if (f) {
        fclose(f);
        f = nullptr;
    }
    if (!ok && !tmp_path.empty()) {
        unlink(tmp_path.c_str());
    }
    curl_easy_cleanup(curl);
    return ok;
}

int main(int argc, char** argv) {
    // Usage: host username password remote_path [port]
    if (argc == 5 || argc == 6) {
        std::string host = argv[1];
        std::string user = argv[2];
        std::string pass = argv[3];
        std::string remote = argv[4];
        long port = (argc == 6) ? std::stol(argv[5]) : 21;
        bool ok = download_sensitive_file(host, port, user, pass, remote);
        std::cout << (ok ? "SUCCESS" : "FAILURE") << std::endl;
        return 0;
    }

    // 5 test cases (placeholders)
    struct Case { const char* h; long p; const char* u; const char* pw; const char* rp; };
    Case tests[5] = {
        {"localhost", 21, "user", "pass", "/sensitive.txt"},
        {"127.0.0.1", 21, "anonymous", "anonymous@", "/pub/example.bin"},
        {"ftp.example.com", 21, "user1", "pass1", "/data/file1.dat"},
        {"ftp.testserver.com", 21, "user2", "pass2", "docs/readme.txt"},
        {"192.0.2.1", 21, "user3", "pass3", "/secure/secret.cfg"},
    };
    for (int i = 0; i < 5; ++i) {
        bool ok = download_sensitive_file(tests[i].h, tests[i].p, tests[i].u, tests[i].pw, tests[i].rp);
        std::cout << "Test " << (i + 1) << ": " << (ok ? "SUCCESS" : "FAILURE") << std::endl;
    }
    return 0;
}