// Steps applied (high-level):
// 1) Minimal web app with /api route, reads ?payload=..., parses JSON to map, returns "name".
// 2) Security: limit input size, strict flat JSON string parsing, validate key presence/type, robust URL decoding.
// 3) No unsafe deserialization; simple socket server handling GET only.
// 4) Reviewed for bounds and errors.
// 5) Final secure code.

#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <stdexcept>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
// Windows sockets (if needed). For simplicity, this sample assumes POSIX-like environment.
// If Windows support is required, Winsock initialization should be added.
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

static const size_t MAX_PAYLOAD_LENGTH_CPP = 2048;
static const size_t MAX_REQUEST_SIZE_CPP = 4096;

// Trim helpers
static inline size_t skipWs(const std::string& s, size_t i) {
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) i++;
    return i;
}

static std::string parseJsonString(const std::string& s, size_t startQuoteIdx, size_t& outNextIdx) {
    if (startQuoteIdx >= s.size() || s[startQuoteIdx] != '"') throw std::invalid_argument("Expected '\"'");
    std::string out;
    size_t i = startQuoteIdx + 1;
    while (i < s.size()) {
        char c = s[i];
        if (c == '\\') {
            if (i + 1 >= s.size()) throw std::invalid_argument("Invalid escape");
            char n = s[i + 1];
            if (n == '\\' || n == '"') { out.push_back(n); i += 2; }
            else if (n == 'n') { out.push_back('\n'); i += 2; }
            else if (n == 't') { out.push_back('\t'); i += 2; }
            else if (n == 'r') { out.push_back('\r'); i += 2; }
            else throw std::invalid_argument("Unsupported escape");
        } else if (c == '"') {
            outNextIdx = i + 1;
            return out;
        } else {
            out.push_back(c);
            i++;
        }
    }
    throw std::invalid_argument("Unterminated string");
}

static std::map<std::string, std::string> parseSimpleJsonToMap(const std::string& json) {
    std::map<std::string, std::string> res;
    std::string s;
    s.reserve(json.size());
    // Trim
    size_t a = 0, b = json.size();
    while (a < b && std::isspace(static_cast<unsigned char>(json[a]))) a++;
    while (b > a && std::isspace(static_cast<unsigned char>(json[b - 1]))) b--;
    if (b - a < 2) throw std::invalid_argument("Invalid JSON");
    if (json[a] != '{' || json[b - 1] != '}') throw std::invalid_argument("Invalid JSON object");
    size_t i = a + 1;
    while (i < b - 1) {
        i = skipWs(json, i);
        if (i >= b - 1) break;
        if (json[i] == ',') { i++; continue; }
        if (json[i] != '"') throw std::invalid_argument("Expected key string");
        size_t nextIdx = 0;
        std::string key = parseJsonString(json, i, nextIdx);
        i = skipWs(json, nextIdx);
        if (i >= b - 1 || json[i] != ':') throw std::invalid_argument("Expected ':'");
        i++;
        i = skipWs(json, i);
        if (i >= b - 1 || json[i] != '"') throw std::invalid_argument("Only string values allowed");
        std::string val = parseJsonString(json, i, nextIdx);
        i = nextIdx;
        res[key] = val;
        i = skipWs(json, i);
        if (i < b - 1) {
            if (json[i] == ',') i++;
            else if (json[i] == '}') break;
        }
    }
    return res;
}

static std::string urlDecode(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '+') {
            out.push_back(' ');
        } else if (c == '%' && i + 2 < in.size() &&
                   std::isxdigit(static_cast<unsigned char>(in[i + 1])) &&
                   std::isxdigit(static_cast<unsigned char>(in[i + 2]))) {
            auto hex = in.substr(i + 1, 2);
            char* end = nullptr;
            long val = strtol(hex.c_str(), &end, 16);
            out.push_back(static_cast<char>(val));
            i += 2;
        } else {
            out.push_back(c);
        }
    }
    return out;
}

static std::map<std::string, std::string> parseQuery(const std::string& q) {
    std::map<std::string, std::string> m;
    size_t start = 0;
    while (start <= q.size()) {
        size_t amp = q.find('&', start);
        std::string pair = q.substr(start, (amp == std::string::npos ? q.size() : amp) - start);
        size_t eq = pair.find('=');
        std::string key = (eq == std::string::npos) ? pair : pair.substr(0, eq);
        std::string val = (eq == std::string::npos) ? "" : pair.substr(eq + 1);
        m[urlDecode(key)] = urlDecode(val);
        if (amp == std::string::npos) break;
        start = amp + 1;
    }
    return m;
}

std::string getNameFromPayload(const std::string& payload) {
    if (payload.empty()) throw std::invalid_argument("Missing payload");
    if (payload.size() > MAX_PAYLOAD_LENGTH_CPP) throw std::invalid_argument("Payload too large");
    auto map = parseSimpleJsonToMap(payload);
    auto it = map.find("name");
    if (it == map.end()) throw std::invalid_argument("Missing 'name'");
    return it->second;
}

static void sendHttpResponse(int client_fd, int code, const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << code << " "
        << ((code == 200) ? "OK" :
            (code == 400) ? "Bad Request" :
            (code == 404) ? "Not Found" :
            (code == 405) ? "Method Not Allowed" :
            (code == 413) ? "Payload Too Large" : "Internal Server Error")
        << "\r\n";
    oss << "Content-Type: text/plain; charset=UTF-8\r\n";
    oss << "X-Content-Type-Options: nosniff\r\n";
    oss << "Cache-Control: no-store\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << body;
    std::string resp = oss.str();
#ifdef _WIN32
    send(client_fd, resp.c_str(), (int)resp.size(), 0);
    closesocket(client_fd);
#else
    send(client_fd, resp.c_str(), resp.size(), 0);
    close(client_fd);
#endif
}

static void run_server_cpp() {
#ifdef _WIN32
    // For brevity, Windows socket init omitted.
#endif
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8083);
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed\n";
#ifdef _WIN32
        closesocket(server_fd);
#else
        close(server_fd);
#endif
        return;
    }
    if (listen(server_fd, 16) < 0) {
        std::cerr << "Listen failed\n";
#ifdef _WIN32
        closesocket(server_fd);
#else
        close(server_fd);
#endif
        return;
    }
    std::cout << "C++ server listening on http://localhost:8083/api?payload={...}\n";
    for (;;) {
        sockaddr_in caddr{};
#ifdef _WIN32
        int clen = sizeof(caddr);
#else
        socklen_t clen = sizeof(caddr);
#endif
        int cfd = accept(server_fd, (sockaddr*)&caddr, &clen);
        if (cfd < 0) continue;
        char buf[MAX_REQUEST_SIZE_CPP + 1];
        ssize_t n = recv(cfd, buf, MAX_REQUEST_SIZE_CPP, 0);
        if (n <= 0) {
            sendHttpResponse(cfd, 400, "Bad Request");
            continue;
        }
        buf[n] = '\0';
        std::string req(buf, buf + n);
        // Parse first line
        size_t lineEnd = req.find("\r\n");
        if (lineEnd == std::string::npos) { sendHttpResponse(cfd, 400, "Bad Request"); continue; }
        std::string firstLine = req.substr(0, lineEnd);
        // Expect: GET /api?payload=... HTTP/1.1
        if (firstLine.rfind("GET ", 0) != 0) { sendHttpResponse(cfd, 405, "Method Not Allowed"); continue; }
        size_t sp1 = firstLine.find(' ');
        size_t sp2 = firstLine.find(' ', sp1 + 1);
        if (sp1 == std::string::npos || sp2 == std::string::npos) { sendHttpResponse(cfd, 400, "Bad Request"); continue; }
        std::string target = firstLine.substr(sp1 + 1, sp2 - sp1 - 1);
        // Split path and query
        std::string path, query;
        size_t qm = target.find('?');
        if (qm == std::string::npos) { path = target; }
        else { path = target.substr(0, qm); query = target.substr(qm + 1); }
        if (path != "/api") { sendHttpResponse(cfd, 404, "Not Found"); continue; }
        auto qs = parseQuery(query);
        auto it = qs.find("payload");
        if (it == qs.end()) { sendHttpResponse(cfd, 400, "Missing 'payload' parameter"); continue; }
        if (it->second.size() > MAX_PAYLOAD_LENGTH_CPP) { sendHttpResponse(cfd, 413, "Payload too large"); continue; }
        try {
            std::string name = getNameFromPayload(it->second);
            sendHttpResponse(cfd, 200, name);
        } catch (...) {
            sendHttpResponse(cfd, 400, "Invalid or missing 'name'");
        }
    }
}

int main() {
    // 5 test cases for core function
    std::vector<std::string> tests = {
        R"({"name":"Alice"})",
        R"({"name":"Bob","age":"30"})",
        R"({"Name":"CaseSensitive"})",
        R"({invalid json})",
        R"({"name":""})"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            std::string res = getNameFromPayload(tests[i]);
            std::cout << "Test " << (i + 1) << " OK: '" << res << "'\n";
        } catch (const std::exception& e) {
            std::cout << "Test " << (i + 1) << " Error: " << e.what() << "\n";
        }
    }
    run_server_cpp();
    return 0;
}