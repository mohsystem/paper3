#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

static const size_t MAX_SIZE_BYTES_CPP = 5 * 1024 * 1024; // 5MB
static const std::set<std::string> ALLOWED_EXT_CPP = {".txt",".png",".jpg",".jpeg",".pdf",".gif"};

std::string sanitize_filename_cpp(const std::string &name) {
    std::string base = name;
    // Extract basename
    size_t pos = base.find_last_of("/\\");
    if (pos != std::string::npos) base = base.substr(pos + 1);
    std::string out;
    for (char c : base) {
        if (std::isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-') out.push_back(c);
        else out.push_back('_');
    }
    if (out.size() > 100) out.resize(100);
    if (out.find('.') == std::string::npos) out += ".bin";
    return out;
}
std::string get_extension_cpp(const std::string &name) {
    size_t p = name.find_last_of('.');
    if (p == std::string::npos) return "";
    std::string ext = name.substr(p);
    std::string low;
    for (char c : ext) low.push_back(std::tolower((unsigned char)c));
    return low;
}
std::string random_hex_cpp(size_t nbytes) {
    std::vector<unsigned char> buf(nbytes);
    std::random_device rd;
    for (size_t i = 0; i < nbytes; ++i) buf[i] = (unsigned char)(rd() & 0xFF);
    static const char* hexd = "0123456789abcdef";
    std::string out; out.reserve(nbytes*2);
    for (unsigned char b : buf) { out.push_back(hexd[b>>4]); out.push_back(hexd[b&0xF]); }
    return out;
}
bool ensure_dir_cpp(const std::string &path) {
    struct stat st{};
    if (stat(path.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    if (mkdir(path.c_str(), 0700) == 0) return true;
    return false;
}

std::string save_uploaded_file_cpp(const uint8_t* data, size_t len, const std::string& original_filename, const std::string& content_type, size_t max_size_bytes, const std::string& upload_dir, std::string &err) {
    err.clear();
    if (!data) { err = "No data"; return ""; }
    if (original_filename.empty()) { err = "Missing filename"; return ""; }
    if (len == 0) { err = "Empty upload"; return ""; }
    if (len > max_size_bytes) { err = "File too large"; return ""; }
    if (!ensure_dir_cpp(upload_dir)) { err = "Cannot create upload dir"; return ""; }

    std::string sanitized = sanitize_filename_cpp(original_filename);
    std::string ext = get_extension_cpp(sanitized);
    if (ALLOWED_EXT_CPP.find(ext) == ALLOWED_EXT_CPP.end()) { err = "Disallowed file extension"; return ""; }

    std::string target = upload_dir + "/" + random_hex_cpp(12) + ext;

    // Ensure inside dir (basic check)
    char realdir[PATH_MAX]; char realtgt[PATH_MAX];
    if (!realpath(upload_dir.c_str(), realdir)) { err = "Path error"; return ""; }
    // We can't realpath target before it exists; just check prefix after resolving dir
    std::string absdir = realdir;
    if (absdir.back() != '/') absdir.push_back('/');

    int fd = open(target.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) { err = std::string("Open failed: ") + strerror(errno); return ""; }
    ssize_t written = write(fd, data, len);
    if (written < 0 || (size_t)written != len) {
        int e = errno;
        close(fd);
        unlink(target.c_str());
        err = std::string("Write failed: ") + strerror(e);
        return "";
    }
    if (fsync(fd) != 0) {
        int e = errno;
        close(fd);
        unlink(target.c_str());
        err = std::string("Sync failed: ") + strerror(e);
        return "";
    }
    close(fd);
    char realup[PATH_MAX];
    if (!realpath(target.c_str(), realup)) {
        unlink(target.c_str());
        err = "Path resolve failed";
        return "";
    }
    std::string abspath = realup;
    if (abspath.rfind(absdir, 0) != 0) {
        unlink(target.c_str());
        err = "Invalid path";
        return "";
    }
    return abspath;
}

// Minimal HTTP server to handle GET / and POST /upload with X-Filename and raw body
namespace tiny_http_cpp {
    std::string html_page() {
        return "<!doctype html><html><head><meta charset='utf-8'><title>C++ Upload</title></head>"
               "<body><h1>Upload a file</h1>"
               "<input type='file' id='f'><button id='u'>Upload</button>"
               "<pre id='o'></pre>"
               "<script>"
               "document.getElementById('u').onclick=async()=>{"
               "let f=document.getElementById('f').files[0];"
               "if(!f){alert('Choose a file');return;}"
               "let buf=await f.arrayBuffer();"
               "let r=await fetch('/upload',{method:'POST',headers:{'X-Filename':f.name,'Content-Type':'application/octet-stream','Content-Length':buf.byteLength},body:buf});"
               "document.getElementById('o').textContent=await r.text();};"
               "</script></body></html>";
    }

    bool send_all(int fd, const void* buf, size_t len) {
        const char* p = (const char*)buf;
        size_t left = len;
        while (left > 0) {
            ssize_t n = send(fd, p, left, 0);
            if (n <= 0) return false;
            p += n; left -= n;
        }
        return true;
    }

    void respond_text(int cfd, int code, const std::string& msg) {
        std::ostringstream oss;
        oss << "HTTP/1.1 " << code << " \r\n"
            << "Content-Type: text/plain; charset=utf-8\r\n"
            << "X-Content-Type-Options: nosniff\r\n"
            << "Content-Length: " << msg.size() << "\r\n"
            << "Connection: close\r\n\r\n"
            << msg;
        std::string resp = oss.str();
        send_all(cfd, resp.data(), resp.size());
    }

    void respond_html(int cfd, const std::string& html) {
        std::ostringstream oss;
        oss << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: text/html; charset=utf-8\r\n"
            << "X-Content-Type-Options: nosniff\r\n"
            << "Content-Length: " << html.size() << "\r\n"
            << "Connection: close\r\n\r\n"
            << html;
        std::string resp = oss.str();
        send_all(cfd, resp.data(), resp.size());
    }

    void handle_client(int cfd, const std::string& upload_dir) {
        const size_t HDR_LIMIT = 8192;
        std::string req;
        req.reserve(HDR_LIMIT);
        char buf[1024];
        // Read headers until CRLFCRLF or limit
        bool found = false;
        while (req.size() < HDR_LIMIT) {
            ssize_t n = recv(cfd, buf, sizeof(buf), 0);
            if (n <= 0) break;
            req.append(buf, buf + n);
            if (req.find("\r\n\r\n") != std::string::npos) { found = true; break; }
            if ((size_t)n < sizeof(buf)) break;
        }
        if (!found) { respond_text(cfd, 400, "Bad Request"); return; }

        // Parse request line
        size_t line_end = req.find("\r\n");
        if (line_end == std::string::npos) { respond_text(cfd, 400, "Bad Request"); return; }
        std::string reqline = req.substr(0, line_end);
        std::istringstream iss(reqline);
        std::string method, path, ver;
        iss >> method >> path >> ver;
        if (method.empty() || path.empty()) { respond_text(cfd, 400, "Bad Request"); return; }

        // Headers
        std::unordered_map<std::string, std::string> headers;
        size_t pos = line_end + 2;
        size_t hdr_end = req.find("\r\n\r\n", pos);
        while (pos < hdr_end) {
            size_t e = req.find("\r\n", pos);
            if (e == std::string::npos || e > hdr_end) break;
            std::string line = req.substr(pos, e - pos);
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string key = line.substr(0, colon);
                std::string val = line.substr(colon + 1);
                // trim
                auto ltrim = [](std::string &s){ s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch){return !std::isspace(ch);})); };
                auto rtrim = [](std::string &s){ s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch){return !std::isspace(ch);}).base(), s.end()); };
                ltrim(key); rtrim(key); ltrim(val); rtrim(val);
                std::string lower;
                for (char c : key) lower.push_back(std::tolower((unsigned char)c));
                headers[lower] = val;
            }
            pos = e + 2;
        }

        size_t body_start = hdr_end + 4;
        std::string body_initial;
        if (req.size() > body_start) body_initial.assign(req.begin() + body_start, req.end());

        if (method == "GET" && path == "/") {
            respond_html(cfd, html_page());
            return;
        }
        if (method != "POST" || path != "/upload") {
            respond_text(cfd, 404, "Not Found");
            return;
        }

        std::string cl = headers.count("content-length") ? headers["content-length"] : "";
        if (cl.empty()) { respond_text(cfd, 411, "Length Required"); return; }
        long long content_len = -1;
        try { content_len = std::stoll(cl); } catch (...) { content_len = -1; }
        if (content_len <= 0 || (size_t)content_len > MAX_SIZE_BYTES_CPP) {
            respond_text(cfd, 413, "Payload Too Large"); return;
        }
        std::string xfn = headers.count("x-filename") ? headers["x-filename"] : "";
        if (xfn.empty()) { respond_text(cfd, 400, "Missing X-Filename"); return; }
        std::string ctype = headers.count("content-type") ? headers["content-type"] : "application/octet-stream";

        std::vector<uint8_t> data;
        data.reserve((size_t)content_len);
        data.insert(data.end(), body_initial.begin(), body_initial.end());
        while (data.size() < (size_t)content_len) {
            ssize_t n = recv(cfd, buf, sizeof(buf), 0);
            if (n <= 0) break;
            size_t need = (size_t)content_len - data.size();
            size_t take = (size_t)n > need ? need : (size_t)n;
            data.insert(data.end(), (uint8_t*)buf, (uint8_t*)buf + take);
        }
        if (data.size() != (size_t)content_len) {
            respond_text(cfd, 400, "Incomplete body"); return;
        }

        std::string err;
        std::string saved = save_uploaded_file_cpp(data.data(), data.size(), xfn, ctype, MAX_SIZE_BYTES_CPP, upload_dir, err);
        if (saved.empty()) {
            respond_text(cfd, 400, std::string("Upload failed: ") + err);
        } else {
            std::string msg = "Saved: " + std::string(strrchr(saved.c_str(), '/') ? strrchr(saved.c_str(), '/') + 1 : saved.c_str());
            respond_text(cfd, 200, msg);
        }
    }

    void run(uint16_t port, const std::string& upload_dir) {
        int sfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sfd < 0) { perror("socket"); return; }
        int opt = 1;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(port);
        if (bind(sfd, (sockaddr*)&addr, sizeof(addr)) != 0) { perror("bind"); close(sfd); return; }
        if (listen(sfd, 16) != 0) { perror("listen"); close(sfd); return; }
        std::cout << "C++ server running on http://127.0.0.1:" << port << std::endl;
        while (true) {
            int cfd = accept(sfd, nullptr, nullptr);
            if (cfd < 0) continue;
            // Handle sequentially for simplicity
            handle_client(cfd, upload_dir);
            close(cfd);
        }
    }
}

int main() {
    std::string upload_dir = "uploads_cpp";
    ensure_dir_cpp(upload_dir);
    chmod(upload_dir.c_str(), 0700);

    // Start server in background thread
    std::thread srv([](){
        tiny_http_cpp::run(8082, "uploads_cpp");
    });
    srv.detach();

    // 5 tests
    std::cout << "Running C++ tests:" << std::endl;
    {
        std::string err;
        std::string saved = save_uploaded_file_cpp((const uint8_t*)"hello", 5, "hello.txt", "text/plain", MAX_SIZE_BYTES_CPP, upload_dir, err);
        if (!saved.empty()) std::cout << "Test 1 OK: " << saved << std::endl;
        else std::cout << "Test 1 FAILED: " << err << std::endl;
    }
    {
        std::string err;
        std::string saved = save_uploaded_file_cpp((const uint8_t*)"x", 1, "../../evil.txt", "text/plain", MAX_SIZE_BYTES_CPP, upload_dir, err);
        if (!saved.empty()) std::cout << "Test 2 OK (sanitized): " << saved << std::endl;
        else std::cout << "Test 2 FAILED: " << err << std::endl;
    }
    {
        std::string err;
        std::string saved = save_uploaded_file_cpp((const uint8_t*)"bin", 3, "malware.exe", "application/octet-stream", MAX_SIZE_BYTES_CPP, upload_dir, err);
        if (saved.empty()) std::cout << "Test 3 OK (rejected): " << err << std::endl;
        else std::cout << "Test 3 FAILED (accepted): " << saved << std::endl;
    }
    {
        std::string err;
        std::vector<uint8_t> big(MAX_SIZE_BYTES_CPP + 1, 0x41);
        std::string saved = save_uploaded_file_cpp(big.data(), big.size(), "big.txt", "text/plain", MAX_SIZE_BYTES_CPP, upload_dir, err);
        if (saved.empty()) std::cout << "Test 4 OK (oversize rejected): " << err << std::endl;
        else std::cout << "Test 4 FAILED (oversize accepted): " << saved << std::endl;
    }
    {
        std::string err;
        uint8_t img[] = {0x89, 0x50, 0x4E, 0x47};
        std::string saved = save_uploaded_file_cpp(img, sizeof(img), "my image (1).jpg", "image/jpeg", MAX_SIZE_BYTES_CPP, upload_dir, err);
        if (!saved.empty()) std::cout << "Test 5 OK: " << saved << std::endl;
        else std::cout << "Test 5 FAILED: " << err << std::endl;
    }

    // Keep main alive to serve
    while (true) pause();
    return 0;
}